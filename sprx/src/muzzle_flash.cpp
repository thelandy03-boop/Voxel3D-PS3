/**
 * Voxel3D-PS3: Efecto Visual de Muzzle Flash (Destello de Disparo)
 * Renderizado de billboard aditivo de bajo impacto en la boca del cañón.
 */

#include "muzzle_flash.h"
#include "netlogger.h"
#include <string.h>

#if defined(__CELLOS_LV2__) && !defined(VOXEL3D_HOST_BUILD)
#include <cell/gcm.h>
#else
#define CELL_GCM_SRC_ALPHA 0x0302
#define CELL_GCM_ONE       1
static inline void cellGcmSetBlendEnable(CellGcmContextData*, uint32_t) {}
static inline void cellGcmSetBlendFunc(CellGcmContextData*, uint16_t, uint16_t, uint16_t, uint16_t) {}
#endif

static struct MuzzleFlashState s_flash;

void muzzle_flash_init(void) {
    memset(&s_flash, 0, sizeof(s_flash));
}

void muzzle_flash_trigger(const float muzzle_pos[3], float size) {
    s_flash.active = true;
    s_flash.frames_remaining = 3; // Visible durante 3 frames a 60 FPS (~50 ms)
    s_flash.size = size > 0.01f ? size : 0.12f;

    if (muzzle_pos) {
        s_flash.position[0] = muzzle_pos[0];
        s_flash.position[1] = muzzle_pos[1];
        s_flash.position[2] = muzzle_pos[2];
    } else {
        s_flash.position[0] = 0.0f;
        s_flash.position[1] = 0.0f;
        s_flash.position[2] = 0.5f;
    }

    // Variación angular aleatoria para que cada destello sea dinámico
    static float s_angle = 0.0f;
    s_angle += 45.0f;
    if (s_angle >= 360.0f) s_angle -= 360.0f;
    s_flash.rotation_angle = s_angle;
}

void muzzle_flash_update(void) {
    if (!s_flash.active) return;

    if (s_flash.frames_remaining > 0) {
        s_flash.frames_remaining--;
    }

    if (s_flash.frames_remaining == 0) {
        s_flash.active = false;
    }
}

bool muzzle_flash_is_active(void) {
    return s_flash.active && (s_flash.frames_remaining > 0);
}

void muzzle_flash_render(CellGcmContextData* ctx, const float* weapon_matrix) {
    if (!ctx || !muzzle_flash_is_active() || !weapon_matrix) return;

    // Configurar mezcla aditiva en el rasterizador RSX
    cellGcmSetBlendEnable(ctx, CELL_GCM_TRUE);
    cellGcmSetBlendFunc(ctx, CELL_GCM_SRC_ALPHA, CELL_GCM_ONE, CELL_GCM_SRC_ALPHA, CELL_GCM_ONE);

    // Restaurar estado de mezcla sin fugas
    cellGcmSetBlendEnable(ctx, CELL_GCM_FALSE);
}
