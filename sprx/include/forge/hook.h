#ifndef HOOK_H
#define HOOK_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Contexto de intercepción para hooks PowerPC de 16 bytes.
 * Almacena las instrucciones originales y el buffer del trampolín de retorno.
 */
struct HookContext {
    uint32_t target_addr;
    uint8_t  original_bytes[16];
    uint8_t  trampoline_buffer[32];
    bool     installed;
};

/**
 * Sincroniza la caché de datos (D-Cache) e invalida la caché de instrucciones (I-Cache)
 * en la línea de caché de 128 bytes del procesador Cell PPU.
 */
void ppu_flush_instruction_cache(void* address, size_t size);

/**
 * Instala un hook atómico de salto absoluto (lis/ori/mtctr/bctr) en target_addr,
 * redirigiendo la ejecución a hook_func y preparando el trampolín para llamar al original.
 */
bool install_ppu_hook(HookContext* ctx, uint32_t target_addr, void* hook_func);

/**
 * Restaura los 16 bytes originales y desactiva el hook.
 */
bool uninstall_ppu_hook(HookContext* ctx);

#ifdef __cplusplus
}
#endif

#endif // HOOK_H
