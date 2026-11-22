#ifndef VOXEL3D_HOOK_H
#define VOXEL3D_HOOK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Argumentos originales (r3..r10) de la funcion hookados antes de que el
// trampoline los sobrescriba. Se pasan al handler como void*.
typedef struct {
    uint64_t r3;
    uint64_t r4;
    uint64_t r5;
    uint64_t r6;
    uint64_t r7;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
} Vox3HookArgs;

// Handler invocado desde el trampoline despues de ejecutar el prologo original.
// Se llama con arg = Vox3HookArgs*; el contexto GCM se obtiene con
// cellGcmGetCurrentContext().
typedef void (*Vox3RenderHandler)(void* arg);

typedef struct {
    uint32_t target;
    uint8_t  original[16];
    uint32_t trampoline[64];
    bool     installed;
} VX3HookContext;

int  vox3_hook_install(uint32_t target, Vox3RenderHandler handler);
int  vox3_hook_remove(void);
void vox3_flush_icache(void* address, size_t size);

#endif /* VOXEL3D_HOOK_H */
