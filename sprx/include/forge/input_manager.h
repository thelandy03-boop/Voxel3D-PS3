#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Estado decodificado del mando DualShock 3.
 */
struct ControllerInputState {
    bool l2_aim_pressed;      // Gatillo L2 presionado (Apuntar con la mira ADS)
    bool r1_fire_pressed;     // Botón R1 presionado (Disparar)
    bool r1_fire_just_pressed;// Transición de flanco de subida (nuevo disparo en este frame)
    uint8_t l2_analog_value;  // Presión analógica de L2 (0 - 255)
};

/**
 * Inicializa el subsistema de lectura de mandos Cell OS (cellPad).
 */
void input_manager_init(void);

/**
 * Sondea el estado del mando del puerto 0 (jugador local principal)
 * y actualiza los flags de control.
 */
void input_manager_poll(struct ControllerInputState* out_state);

/**
 * Permite inyectar pulsaciones simuladas para pruebas o emuladores.
 */
void input_manager_simulate_press(bool l2_aim, bool r1_fire);

#ifdef __cplusplus
}
#endif

#endif // INPUT_MANAGER_H
