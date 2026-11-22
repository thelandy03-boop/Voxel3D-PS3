/**
 * Voxel3D-PS3: Gestor de Entrada de Mandos (DualShock 3)
 * Mapeo en caliente de gatillos L2 (Apuntado ADS) y R1 (Disparo/Retroceso).
 */

#include "input_manager.h"
#include <string.h>

#if defined(__CELLOS_LV2__) && !defined(VOXEL3D_HOST_BUILD)
#include <cell/pad.h>
#else
// Mocks para compilación en host
#define CELL_PAD_MAX_CODES 64
typedef struct CellPadData {
    int32_t len;
    uint16_t button[CELL_PAD_MAX_CODES];
} CellPadData;
#define CELL_PAD_CTRL_L2 (1 << 8)
#define CELL_PAD_CTRL_R1 (1 << 11)
static inline int cellPadGetData(uint32_t, CellPadData*) { return -1; }
#endif

static bool s_prev_r1_state = false;
static bool s_simulated_l2 = false;
static bool s_simulated_r1 = false;
static bool s_has_simulation = false;

void input_manager_init(void) {
    s_prev_r1_state = false;
    s_simulated_l2 = false;
    s_simulated_r1 = false;
    s_has_simulation = false;
}

void input_manager_simulate_press(bool l2_aim, bool r1_fire) {
    s_simulated_l2 = l2_aim;
    s_simulated_r1 = r1_fire;
    s_has_simulation = true;
}

void input_manager_poll(struct ControllerInputState* out_state) {
    if (!out_state) return;

    memset(out_state, 0, sizeof(struct ControllerInputState));

#if defined(__CELLOS_LV2__) && !defined(VOXEL3D_HOST_BUILD)
    CellPadData data;
    int ret = cellPadGetData(0, &data); // Puerto 0 (Jugador 1)
    if (ret == 0 && data.len > 0) {
        uint16_t digital = data.button[2];
        out_state->l2_aim_pressed = (digital & CELL_PAD_CTRL_L2) != 0;

        bool r1_pressed = (digital & CELL_PAD_CTRL_R1) != 0;
        out_state->r1_fire_pressed = r1_pressed;
        out_state->r1_fire_just_pressed = r1_pressed && !s_prev_r1_state;
        s_prev_r1_state = r1_pressed;
        return;
    }
#endif

    // Si se inyectó simulación para pruebas o fallback
    if (s_has_simulation) {
        out_state->l2_aim_pressed = s_simulated_l2;
        out_state->r1_fire_pressed = s_simulated_r1;
        out_state->r1_fire_just_pressed = s_simulated_r1 && !s_prev_r1_state;
        s_prev_r1_state = s_simulated_r1;
        return;
    }

    out_state->r1_fire_just_pressed = false;
}
