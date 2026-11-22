# Ingeniería de Hooking en PowerPC (Cell PPU)

Este documento detalla la metodología de intercepción de funciones en caliente en la PPU (PowerPC Processor Unit) para PlayStation 3, abordando las protecciones de memoria de CellOS Lv2, la convención de llamadas PowerPC y la coherencia de caché de instrucciones.

---

## 1. Contexto de Ejecución de la PPU y Convención de Llamadas (PPC ABI)

El ejecutable de Minecraft PS3 (`EBOOT.BIN` / `EBOOT.ELF`) corre en la PPU del Cell. Aunque el hardware es PowerPC de 64 bits, la mayoría de los motores de videojuegos en PS3 compilan utilizando la ABI de punteros de 32 bits (`-m32`).

### Registros Clave de PowerPC:
* **`r1` (Stack Pointer):** Puntero de pila (alineación estricta de 16 bytes).
* **`r2` (Table of Contents - TOC):** Registro base para direccionamiento global y tablas de símbolos. **Crítico:** Si una llamada salta entre módulos (ej. de `EBOOT` a `SPRX`), el TOC debe ser preservado o actualizado adecuadamente si se usan punteros de función externos (*function descriptors*).
* **`r3` - `r10`:** Paso de parámetros enteros y valores de retorno (`r3` almacena el primer argumento o puntero `this` en llamadas C++).
* **`r11` - `r12`:** Registros temporales y volátiles (seguros para usar en secuencias de salto o *trampolines* sin alterar el estado de la función).
* **`lr` (Link Register):** Dirección de retorno de la función.
* **`ctr` (Count Register):** Registro de salto indirecto (`mtctr` / `bctr`).

---

## 2. Bypass de Protecciones de Memoria de CellOS

Por defecto, la sección de código ejecutable (`.text`) de un proceso en PS3 se encuentra protegida contra escritura (`PAGE_READ | PAGE_EXEC`).

### Mecanismos de Modificación de Memoria:
En consolas con Custom Firmware (CFW CEX/DEX o HEN):

1. **Syscall de Depuración (`sys_dbg_write_process_memory`):**
   ```cpp
   // Syscall LV2 905 (DEX/CFW)
   int sys_dbg_write_process_memory(sys_pid_t pid, void* dest, const void* src, size_t size);
   ```
2. **Poke de Kernel LV2 (Syscall 7 / 9):**
   Permite escribir directamente en la memoria física o sobreescribir las tablas de páginas MMU para conceder permisos de escritura temporales a la PPU.

---

## 3. Arquitectura del Trampoline de Salto Absoluto

En PowerPC, la instrucción de salto directo relativo `b <offset>` tiene un límite de direccionamiento de $\pm 32\text{ MB}$ (24 bits con desplazamiento de 2 bits). Dado que los módulos dinámicos `.sprx` son cargados por el sistema operativo en rangos de memoria altos (típicamente `0xD0000000` o superior), un salto relativo simple provocará un desbordamiento (*relocation truncation*).

### Secuencia de Salto Absoluto de 16 Bytes:
Para saltar de forma determinista a cualquier dirección de 32 bits sin alterar los registros de argumentos (`r3-r10`):

```asm
# Cargar dirección de 32 bits en el registro volátil r11
lis   %r11, target_addr@ha    # Carga los 16 bits superiores con ajuste algebraico
ori   %r11, %r11, target_addr@l # Carga los 16 bits inferiores
mtctr %r11                    # Mueve el destino al Count Register (CTR)
bctr                          # Salta incondicionalmente a la dirección en CTR
```

### Estructura del Trampolín:
```
[ Función Original: renderItemInFirstPerson ]
   0x0045A120: lis   %r11, VoxelHook@ha   ◄── Parche aplicado en caliente
   0x0045A124: ori   %r11, %r11, VoxelHook@l
   0x0045A128: mtctr %r11
   0x0045A12C: bctr
   0x0045A130: [Continuación función original...]

[ Trampoline Stub en Memoria SPRX ]
   0xD0010000: [Instrucción original 1 reubicada]
   0xD0010004: [Instrucción original 2 reubicada]
   0xD0010008: [Instrucción original 3 reubicada]
   0xD001000C: [Instrucción original 4 reubicada]
   0xD0010010: lis   %r11, 0x0045A130@ha  ◄── Salto de retorno hacia la original
   0xD0010014: ori   %r11, %r11, 0x0045A130@l
   0xD0010018: mtctr %r11
   0xD001001C: bctr
```

---

## 4. Coherencia de Caché Obligatoria (D-Cache / I-Cache)

La arquitectura Cell cuenta con cachés L1 de datos e instrucciones separadas para la PPU que **no son coherentes por hardware para código automodificable**.

Escribir bytes de salto altera la caché de datos (D-Cache). Si no se vacía explícitamente hacia la memoria principal y se invalida la caché de instrucciones (I-Cache), la PPU continuará ejecutando las instrucciones cacheadas anteriores, provocando fallos erráticos o cuelgues (*kernel panic / freeze*).

### Rutina de Sincronización en C++ / Assembly:
```cpp
void ppu_flush_instruction_cache(void* address, size_t size) {
    uintptr_t start = (uintptr_t)address;
    uintptr_t end = start + size;

    // Alinear al tamaño de línea de caché de la PPU (128 bytes)
    start &= ~127ULL;

    for (uintptr_t p = start; p < end; p += 128) {
        __asm__ volatile(
            "dcbst 0, %0 \n\t" // Data Cache Block Store (forzar a RAM)
            "sync        \n\t" // Esperar a que la escritura en bus termine
            "icbi  0, %0 \n\t" // Instruction Cache Block Invalidate (invalidar I-Cache)
            "isync       \n\t" // Sincronizar pipeline de instrucciones
            : : "r"(p) : "memory"
        );
    }
}
```

---

## 5. Implementación de la Intercepción

```cpp
#include <stdint.h>
#include <string.h>

struct HookContext {
    uint32_t target_addr;
    uint8_t  original_bytes[16];
    uint8_t  trampoline_buffer[32];
    bool     installed;
};

bool install_ppu_hook(HookContext* ctx, uint32_t target_addr, void* hook_func) {
    ctx->target_addr = target_addr;
    
    // 1. Respaldar los primeros 16 bytes originales
    memcpy(ctx->original_bytes, (void*)target_addr, 16);
    
    // 2. Construir el trampolín (4 instrucciones originales + salto de vuelta)
    memcpy(ctx->trampoline_buffer, ctx->original_bytes, 16);
    uint32_t return_addr = target_addr + 16;
    
    uint32_t* tramp_jump = (uint32_t*)(ctx->trampoline_buffer + 16);
    tramp_jump[0] = 0x3D600000 | ((return_addr >> 16) & 0xFFFF); // lis %r11, ret@ha
    tramp_jump[1] = 0x616B0000 | (return_addr & 0xFFFF);         // ori %r11, %r11, ret@l
    tramp_jump[2] = 0x7D6903A6;                                 // mtctr %r11
    tramp_jump[3] = 0x4E800420;                                 // bctr

    ppu_flush_instruction_cache(ctx->trampoline_buffer, sizeof(ctx->trampoline_buffer));

    // 3. Escribir el salto hacia la función hook en el destino
    uint32_t patch[4];
    uint32_t hook_dest = (uint32_t)hook_func;
    patch[0] = 0x3D600000 | ((hook_dest >> 16) & 0xFFFF); // lis %r11, hook@ha
    patch[1] = 0x616B0000 | (hook_dest & 0xFFFF);         // ori %r11, %r11, hook@l
    patch[2] = 0x7D6903A6;                                 // mtctr %r11
    patch[3] = 0x4E800420;                                 // bctr

    // Utilizar syscall segura de escritura en memoria
    sys_dbg_write_process_memory(sys_process_getpid(), (void*)target_addr, patch, sizeof(patch));
    ppu_flush_instruction_cache((void*)target_addr, sizeof(patch));

    ctx->installed = true;
    return true;
}
```
Con este mecanismo, la ejecución del hook es atómica, inmune a problemas de latencia de caché y con capacidad de invocación de la función original en caso de que el ítem sostenido no sea un arma o modelo vóxel 3D.
