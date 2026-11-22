# Gestión de Memoria y Presupuesto VRAM/XDR

Este documento define la arquitectura de memoria, políticas de alocación y restricciones presupuestarias para `Voxel3D-PS3` sobre el hardware de PlayStation 3.

---

## 1. Topología de Memoria de PlayStation 3

La consola PlayStation 3 implementa una arquitectura de memoria dividida no unificada (NUMA asimétrico):

```
       ┌───────────────────────────┐         ┌───────────────────────────┐
       │   Cell Broadband Engine   │         │       GPU RSX (G70)       │
       │     (PPU + 6x SPUs)       │         │  (Command FIFO + Shaders) │
       └─────────────┬─────────────┘         └─────────────┬─────────────┘
                     │                                     │
         Ancho: 25.6 GB/s lectura/escritura    Ancho: 20.8 GB/s lectura/escritura
                     │                                     │
       ┌─────────────▼─────────────┐         ┌─────────────▼─────────────┐
       │    256 MB XDR DRAM        │◄────────┤   256 MB GDDR3 VRAM       │
       │    (Memoria Principal)    ├────────►│     (Memoria Local)       │
       └───────────────────────────┘ FlexIO  └───────────────────────────┘
```

### Características Críticas de Acceso:
1. **Memoria Local GDDR3 (`CELL_GCM_LOCATION_LOCAL`):**
   * Acceso de alta velocidad para la GPU RSX (20.8 GB/s).
   * **Advertencia PPU:** Las lecturas de la PPU hacia la GDDR3 son extraordinariamente lentas (~16 MB/s). Toda lectura PPU de la VRAM bloquea el bus. **Regla: La PPU solo escribe hacia VRAM durante la carga; nunca lee en caliente.**
2. **Memoria Principal XDR (`CELL_GCM_LOCATION_MAIN`):**
   * Acceso ultra-rápido para la PPU (25.6 GB/s).
   * El RSX puede acceder a ella mediante el bus FlexIO como memoria de E/S (*Mapped IO*), pero compite con el tráfico del Cell.

---

## 2. Presupuesto de Memoria (Memory Budget)

Minecraft: PS3 Edition opera en límites críticos de memoria XDR y GDDR3 debido a la densidad de chunks del mundo (864x864 bloques), iluminación dinámica y buffers de texturas de alta resolución.

| Dominio | Presupuesto Total Consola | Consumo Motor 4J | Margen Disponible | Asignación Máxima Voxel3D |
| :--- | :--- | :--- | :--- | :--- |
| **XDR RAM (PPU)** | 256 MB | ~238 MB | ~18 MB | **1.5 MB** (Metadatos, configs, tablas) |
| **GDDR3 (RSX)** | 256 MB | ~242 MB | ~14 MB | **4.0 MB** (Pool estático de mallas `.vx3`) |

### Presupuesto por Modelo `.vx3`:
* **Vértices máximos:** 2,500 vértices por modelo ($2,500 \times 12\text{ bytes} = 30,000\text{ bytes}$).
* **Índices máximos:** 3,750 índices `uint16_t` ($3,750 \times 2\text{ bytes} = 7,500\text{ bytes}$).
* **Tamaño total por arma:** $\approx 37.5\text{ KB}$.
* **Capacidad del Pool de 4 MB:** Alrededor de **100 a 110 modelos 3D residentes simultáneos en VRAM**.

---

## 3. Política de Alocación y Alineación

### Requisitos de Hardware del RSX:
* Los punteros base de Vertex Buffers (VBO) e Index Buffers (IBO) requieren alineación obligatoria a **128 bytes**.
* Los offsets calculados mediante `cellGcmAddressToOffset` deben ser múltiplos del tamaño de stride del atributo.

### Estructura del Voxel3D VRAM Pool:
En lugar de fragmentar la VRAM con alocaciones individuales dinámicas (`malloc`), Voxel3D reserva un único bloque contiguo estático durante la inicialización del plugin (`voxel3d_prx_start`):

```cpp
#define VOXEL3D_VRAM_POOL_SIZE (4 * 1024 * 1024) // 4 MB

struct VoxelModelSlot {
    uint32_t model_id;
    uint32_t vbo_offset;       // Offset RSX en GDDR3
    uint32_t ibo_offset;       // Offset RSX en GDDR3
    uint32_t vertex_count;
    uint32_t index_count;
    bool is_loaded;
};

// Puntero base en VRAM local
static void* s_pVramLocalBase = nullptr;
static uint32_t s_vramAllocatedOffset = 0;
```

---

## 4. Algoritmo de Inicialización y Liberación

### Secuencia de Arranque:
1. Consultar configuración de hardware mediante `cellGcmGetConfiguration()` para obtener la dirección base de la VRAM local disponible.
2. Comprobar si existe un heap secundario de GCM no reclamado por el ejecutable de 4J Studios.
3. Si la memoria local GDDR3 no tiene 4 MB libres contiguos:
   * **Mecanismo de Fallback:** Alocar 4 MB en memoria principal XDR usando `sys_memory_allocate(..., SYS_MEMORY_PAGE_SIZE_64K, ...)` y mapear la memoria como IO para el RSX mediante `cellGcmMapMainMemory()`.
4. Cargar los archivos `.vx3` del disco duro (`/dev_hdd0/`) directamente al pool mapeado.
5. Invalidar caches de la PPU y emitir barrera de sincronización (`cellSync()`).

### Secuencia de Parada (`voxel3d_prx_stop`):
1. Asegurar que el pipeline de renderizado RSX esté inactivo (`cellGcmFinish(gContext, 0)`).
2. Desvincular offsets y desmapear memoria IO si se utilizó memoria principal.
3. Liberar punteros y restaurar la tabla de hooks antes del desmontaje del módulo del kernel.

---

## 5. Prevención de Congelamiento por OOM (Out Of Memory)

Si una partida carga un modelo no registrado o si el pool de memoria se satura:
* **Condición de Falla Segura:** La función de detección de ítem (`ItemRenderer Hook`) valida el flag `is_loaded`.
* Si un modelo no está en VRAM, el hook ejecuta de inmediato el *trampoline* hacia la función original de 4J Studios.
* **Resultado:** El juego renderiza el sprite 2D por defecto de Minecraft en lugar de colapsar la consola con un *RSX FIFO crash* o `0x80010005`.
