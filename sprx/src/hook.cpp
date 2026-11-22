/**
 * Voxel3D-PS3: Motor de Intercepción de Funciones (PPU Hooking)
 * Implementa trampolines de salto absoluto de 16 bytes y coherencia de caché para Cell PPU.
 */

#include "hook.h"
#include <string.h>

#if defined(__CELLOS_LV2__) || defined(__powerpc__) || defined(__PPC__)
#include <sys/process.h>

extern "C" {
    // Declaración de syscall de depuración LV2 (sys_dbg_write_process_memory)
    int sys_dbg_write_process_memory(sys_pid_t pid, void* dest, const void* src, size_t size);
}
#endif

void ppu_flush_instruction_cache(void* address, size_t size) {
#if defined(__CELLOS_LV2__) || defined(__powerpc__) || defined(__PPC__)
    uintptr_t start = (uintptr_t)address;
    uintptr_t end = start + size;

    // Alinear al bloque de línea de caché de la PPU (128 bytes)
    start &= ~127ULL;

    for (uintptr_t p = start; p < end; p += 128) {
        __asm__ volatile(
            "dcbst 0, %0 \n\t" // Data Cache Block Store (forzar volcado a RAM)
            "sync        \n\t" // Esperar a que la escritura en el bus concluya
            "icbi  0, %0 \n\t" // Instruction Cache Block Invalidate (invalidar I-Cache)
            "isync       \n\t" // Sincronizar pipeline de ejecución
            : : "r"(p) : "memory"
        );
    }
#else
    // Entorno de prueba / emulado (x86_64 host): no-op
    (void)address;
    (void)size;
#endif
}

static int write_memory_safe(void* dest, const void* src, size_t size) {
#if defined(__CELLOS_LV2__) || defined(__powerpc__) || defined(__PPC__)
    return sys_dbg_write_process_memory(sys_process_getpid(), dest, src, size);
#else
    memcpy(dest, src, size);
    return 0;
#endif
}

bool install_ppu_hook(HookContext* ctx, uint32_t target_addr, void* hook_func) {
    if (!ctx || !target_addr || !hook_func) {
        return false;
    }

    ctx->target_addr = target_addr;

    // 1. Respaldar los 16 bytes de instrucciones originales
    memcpy(ctx->original_bytes, (void*)(uintptr_t)target_addr, 16);

    // 2. Construir el buffer del trampolín:
    //    Primeros 16 bytes: instrucciones originales reubicadas
    //    Siguientes 16 bytes: salto absoluto de retorno a (target_addr + 16)
    memcpy(ctx->trampoline_buffer, ctx->original_bytes, 16);
    uint32_t return_addr = target_addr + 16;

    uint32_t* tramp_jump = (uint32_t*)(ctx->trampoline_buffer + 16);
    tramp_jump[0] = 0x3D600000 | ((return_addr >> 16) & 0xFFFF); // lis  %r11, ret@ha
    tramp_jump[1] = 0x616B0000 | (return_addr & 0xFFFF);         // ori  %r11, %r11, ret@l
    tramp_jump[2] = 0x7D6903A6;                                 // mtctr %r11
    tramp_jump[3] = 0x4E800420;                                 // bctr

    ppu_flush_instruction_cache(ctx->trampoline_buffer, sizeof(ctx->trampoline_buffer));

    // 3. Ensamblar e inyectar el parche de salto absoluto hacia el hook
    uint32_t patch[4];
    uint32_t hook_dest = (uint32_t)(uintptr_t)hook_func;
    patch[0] = 0x3D600000 | ((hook_dest >> 16) & 0xFFFF); // lis  %r11, hook@ha
    patch[1] = 0x616B0000 | (hook_dest & 0xFFFF);         // ori  %r11, %r11, hook@l
    patch[2] = 0x7D6903A6;                                 // mtctr %r11
    patch[3] = 0x4E800420;                                 // bctr

    write_memory_safe((void*)(uintptr_t)target_addr, patch, sizeof(patch));
    ppu_flush_instruction_cache((void*)(uintptr_t)target_addr, sizeof(patch));

    ctx->installed = true;
    return true;
}

bool uninstall_ppu_hook(HookContext* ctx) {
    if (!ctx || !ctx->installed || !ctx->target_addr) {
        return false;
    }

    write_memory_safe((void*)(uintptr_t)ctx->target_addr, ctx->original_bytes, 16);
    ppu_flush_instruction_cache((void*)(uintptr_t)ctx->target_addr, 16);

    ctx->installed = false;
    return true;
}
