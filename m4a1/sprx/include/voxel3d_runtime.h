#ifndef VOXEL3D_RUNTIME_H
#define VOXEL3D_RUNTIME_H

// ---------------------------------------------------------------------------
// API que el modulo M4A1 espera del runtime Voxel3D-PS3.
// Declarado aqui para que main.cpp sea autoconsistente; la implementacion
// concreta (cellGcm / cellFs / hooks PPU) vive en el framework del SPRX.
// ---------------------------------------------------------------------------

#include <stdint.h>
#include "voxel3d.h"

// Memoria / RSX
void*  voxel3d_vram_alloc(uint32_t bytes, uint32_t alignment);
void   voxel3d_vram_free_all(void);
void   voxel3d_set_model_matrix(const float pos[3], const float rot[3], const float scale[3]);
void   voxel3d_set_material_palette(void);
void   voxel3d_apply_fov(float multiplier);
void   draw_muzzle_flash(const float pos[3]);

// Input / tiempo
bool   voxel3d_input_down(uint32_t mask);
bool   voxel3d_swing_detected(void);   // m_animationsToGo del ItemStack (offset 0x10)
float  voxel3d_frame_delta(void);

// Sonido / proyectiles
void   voxel3d_play_sound(const char* event, float volume, float pitch);
void   voxel3d_spawn_projectile(const float origin[3], float speed, float damage);

// Hooks y config
void   voxel3d_install_hook(uint32_t target, void* hook);
void   voxel3d_remove_all_hooks(void);
void   voxel3d_call_original_render(uint32_t itemId, int32_t metadata, void* ctx);
bool   voxel3d_load_config(const char* path, VoxelItemConfig* item, VoxelWeaponConfig* weapon);
void   voxel3d_strcpy(char* dst, const char* src);

#endif // VOXEL3D_RUNTIME_H
