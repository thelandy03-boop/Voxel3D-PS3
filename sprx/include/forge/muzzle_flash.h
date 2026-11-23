#ifndef MUZZLE_FLASH_H
#define MUZZLE_FLASH_H

#include "gcm_types.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Estado dinámico del destello de disparo en la punta del cañón.
 */
struct MuzzleFlashState {
    bool     active;
    uint8_t  frames_remaining;
    float    position[3];      // Coordenadas relativas de la boca del cañón
    float    size;              // Radio del destello en unidades vóxel
    float    rotation_angle;    // Rotación aleatoria para dinamismo visual
};

/**
 * Inicializa el sistema de destello de cañón.
 */
void muzzle_flash_init(void);

/**
 * Dispara el efecto visual en las coordenadas especificadas por el ítem.
 */
void muzzle_flash_trigger(const float muzzle_pos[3], float size);

/**
 * Actualiza el temporizador de fotogramas del destello por cada frame.
 */
void muzzle_flash_update(void);

/**
 * Comprueba si el destello debe renderizarse en el frame actual.
 */
bool muzzle_flash_is_active(void);

/**
 * Renderiza el quad del destello de disparo con mezcla aditiva en la GPU RSX.
 */
void muzzle_flash_render(CellGcmContextData* ctx, const float* weapon_matrix);

#ifdef __cplusplus
}
#endif

#endif // MUZZLE_FLASH_H
