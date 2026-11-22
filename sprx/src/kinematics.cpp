/**
 * Voxel3D-PS3: Cinemática Espacial, Apuntado ADS y Físicas de Retroceso
 * Motor de interpolación matricial en tiempo real para el Reality Synthesizer (RSX).
 */

#include "kinematics.h"
#include <math.h>
#include <string.h>

#define DEG_TO_RAD 0.017453292519943295f

void matrix4x4_identity(float out[16]) {
    memset(out, 0, 16 * sizeof(float));
    out[0] = 1.0f;
    out[5] = 1.0f;
    out[10] = 1.0f;
    out[15] = 1.0f;
}

void matrix4x4_multiply(float out[16], const float a[16], const float b[16]) {
    float temp[16];
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            temp[r * 4 + c] = a[r * 4 + 0] * b[0 * 4 + c] +
                              a[r * 4 + 1] * b[1 * 4 + c] +
                              a[r * 4 + 2] * b[2 * 4 + c] +
                              a[r * 4 + 3] * b[3 * 4 + c];
        }
    }
    memcpy(out, temp, 16 * sizeof(float));
}

void matrix4x4_translate(float out[16], float tx, float ty, float tz) {
    matrix4x4_identity(out);
    out[3]  = tx;
    out[7]  = ty;
    out[11] = tz;
}

void matrix4x4_scale(float out[16], float sx, float sy, float sz) {
    matrix4x4_identity(out);
    out[0]  = sx;
    out[5]  = sy;
    out[10] = sz;
}

void matrix4x4_rotate_euler(float out[16], float pitch_deg, float yaw_deg, float roll_deg) {
    float rad_x = pitch_deg * DEG_TO_RAD;
    float rad_y = yaw_deg   * DEG_TO_RAD;
    float rad_z = roll_deg  * DEG_TO_RAD;

    float cx = cosf(rad_x), sx = sinf(rad_x);
    float cy = cosf(rad_y), sy = sinf(rad_y);
    float cz = cosf(rad_z), sz = sinf(rad_z);

    // Rotación combinada Rz * Ry * Rx (orden estándar Euler para armamento)
    matrix4x4_identity(out);
    out[0] = cy * cz;
    out[1] = cz * sx * sy - cx * sz;
    out[2] = cx * cz * sy + sx * sz;

    out[4] = cy * sz;
    out[5] = cx * cz + sx * sy * sz;
    out[6] = -cz * sx + cx * sy * sz;

    out[8] = -sy;
    out[9] = cy * sx;
    out[10] = cx * cy;
}

void kinematics_init(struct WeaponKinematicsState* state) {
    if (!state) return;
    memset(state, 0, sizeof(struct WeaponKinematicsState));
}

void kinematics_apply_recoil(struct WeaponKinematicsState* state, const struct RegisteredVoxelItem* item) {
    if (!state || !item) return;

    state->recoil_offset[0] += item->recoil_kick_pos[0];
    state->recoil_offset[1] += item->recoil_kick_pos[1];
    state->recoil_offset[2] += item->recoil_kick_pos[2];

    state->recoil_angle[0]  += item->recoil_kick_rot[0];
    state->recoil_angle[1]  += item->recoil_kick_rot[1];
    state->recoil_angle[2]  += item->recoil_kick_rot[2];

    state->is_firing = true;
}

void kinematics_update(struct WeaponKinematicsState* state, const struct RegisteredVoxelItem* item, float delta_time) {
    if (!state || !item) return;

    // 1. Interpolación de Apuntado (ADS) con suavizado cúbico (SmoothStep)
    float ads_speed = 8.0f; // Velocidad de transición ~125 ms
    if (state->l2_aim_pressed) {
        state->ads_progress += delta_time * ads_speed;
        if (state->ads_progress > 1.0f) state->ads_progress = 1.0f;
    } else {
        state->ads_progress -= delta_time * ads_speed;
        if (state->ads_progress < 0.0f) state->ads_progress = 0.0f;
    }

    float t = state->ads_progress;
    float smooth_t = t * t * (3.0f - 2.0f * t);

    for (int i = 0; i < 3; ++i) {
        state->current_pos[i] = item->rest_pos[i] + smooth_t * (item->ads_pos[i] - item->rest_pos[i]);
        state->current_rot[i] = item->rest_rot[i] + smooth_t * (item->ads_rot[i] - item->rest_rot[i]);
    }

    // 2. Decaimiento Exponencial del Retroceso (Recoil Damping)
    float recovery_rate = item->recoil_recovery_speed > 0.0f ? item->recoil_recovery_speed : 14.0f;
    float decay = expf(-recovery_rate * delta_time);

    for (int i = 0; i < 3; ++i) {
        state->recoil_offset[i] *= decay;
        state->recoil_angle[i]  *= decay;
    }
}

void kinematics_build_matrix(const struct WeaponKinematicsState* state, 
                             const struct RegisteredVoxelItem* item, 
                             float out_matrix4x4[16]) {
    if (!state || !item || !out_matrix4x4) return;

    // 1. Matriz de Escala del Vóxel
    float m_scale[16];
    matrix4x4_scale(m_scale, item->scale[0], item->scale[1], item->scale[2]);

    // 2. Matriz de Rotación combinada (Euler actual + Culatazo de retroceso)
    float m_rot[16];
    float final_pitch = state->current_rot[0] + state->recoil_angle[0];
    float final_yaw   = state->current_rot[1] + state->recoil_angle[1];
    float final_roll  = state->current_rot[2] + state->recoil_angle[2];
    matrix4x4_rotate_euler(m_rot, final_pitch, final_yaw, final_roll);

    // 3. Matriz de Traslación combinada (Posición actual + Desplazamiento de retroceso)
    float m_trans[16];
    float final_tx = state->current_pos[0] + state->recoil_offset[0];
    float final_ty = state->current_pos[1] + state->recoil_offset[1];
    float final_tz = state->current_pos[2] + state->recoil_offset[2];
    matrix4x4_translate(m_trans, final_tx, final_ty, final_tz);

    // Combinación: M_final = M_trans * M_rot * M_scale
    float m_rot_scale[16];
    matrix4x4_multiply(m_rot_scale, m_rot, m_scale);
    matrix4x4_multiply(out_matrix4x4, m_trans, m_rot_scale);
}
