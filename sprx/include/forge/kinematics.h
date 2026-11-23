#ifndef KINEMATICS_H
#define KINEMATICS_H

#include "item_registry.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Estado dinámico de cinemática en runtime para el arma activa del jugador.
 */
struct WeaponKinematicsState {
    float ads_progress;            // [0.0 = Reposo en cadera, 1.0 = Apuntando con la mira]
    float current_pos[3];          // Posición interpolada actual
    float current_rot[3];          // Rotación interpolada actual

    // Físicas de retroceso (Recoil)
    float recoil_offset[3];        // Desplazamiento lineal del retroceso
    float recoil_angle[3];         // Rotación angular del culatazo
    bool  is_firing;

    // Estado del mando DualShock 3
    bool  l2_aim_pressed;          // Gatillo L2 presionado
};

/**
 * Inicializa el estado de cinemáticas.
 */
void kinematics_init(struct WeaponKinematicsState* state);

/**
 * Aplica un impulso instantáneo de disparo (recoil kick).
 */
void kinematics_apply_recoil(struct WeaponKinematicsState* state, const struct RegisteredVoxelItem* item);

/**
 * Actualiza la interpolación ADS y la recuperación amortiguada del retroceso por cada frame.
 * delta_time en segundos (ej. 1.0f / 60.0f).
 */
void kinematics_update(struct WeaponKinematicsState* state, const struct RegisteredVoxelItem* item, float delta_time);

/**
 * Construye la matriz de transformación 4x4 combinada para la GPU RSX:
 * M_final = M_ads_lerp * M_recoil * M_rest * M_scale
 */
void kinematics_build_matrix(const struct WeaponKinematicsState* state, 
                             const struct RegisteredVoxelItem* item, 
                             float out_matrix4x4[16]);

// Utilidades matriciales para RSX
void matrix4x4_identity(float out[16]);
void matrix4x4_multiply(float out[16], const float a[16], const float b[16]);
void matrix4x4_translate(float out[16], float tx, float ty, float tz);
void matrix4x4_scale(float out[16], float sx, float sy, float sz);
void matrix4x4_rotate_euler(float out[16], float pitch_deg, float yaw_deg, float roll_deg);

#ifdef __cplusplus
}
#endif

#endif // KINEMATICS_H
