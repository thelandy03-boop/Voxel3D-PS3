#ifndef ITEM_REGISTRY_H
#define ITEM_REGISTRY_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_REGISTERED_ITEMS 256
#define ITEM_ID_DIRECT_MAX 1024

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Descriptor data-driven de un ítem/arma vóxel registrado en memoria XDR.
 * Completamente agnóstico: sirve para armas, herramientas, bloques 3D o cualquier modelo .vx3.
 */
struct RegisteredVoxelItem {
    uint32_t item_id;              // ID de Minecraft (ej. 290 = Azada de madera / Fusil vóxel)
    uint32_t metadata;             // Metadatos / durabilidad (0 si no aplica)
    uint32_t model_id;             // ID del modelo cargado en el pool RSX
    char     name[32];             // Nombre legible del ítem
    
    // Transformación espacial
    float scale[3];                // Escala de vértices [sx, sy, sz]
    float rest_pos[3];             // Posición de reposo [x, y, z] en primera persona
    float rest_rot[3];             // Rotación de reposo en Euler [pitch, yaw, roll] en grados

    // Parámetros de Apuntado (ADS)
    float ads_pos[3];              // Posición alineada a la mira central
    float ads_rot[3];              // Rotación en modo apuntado
    float ads_fov_mult;            // Multiplicador de zoom FOV (ej. 0.70)

    // Parámetros de Retroceso Dinámico (Recoil)
    float recoil_kick_pos[3];      // Impulso de retroceso posicional [x, y, z]
    float recoil_kick_rot[3];      // Impulso angular de culatazo [pitch, yaw, roll]
    float recoil_recovery_speed;   // Velocidad de recuperación LERP (ej. 14.0)

    bool  active;
};

/**
 * Inicializa la tabla de búsqueda O(1) en memoria XDR.
 */
void item_registry_init(void);

/**
 * Registra o actualiza un ítem en la tabla O(1).
 */
bool item_registry_register(const struct RegisteredVoxelItem* item);

/**
 * Búsqueda instantánea O(1) por (item_id, metadata).
 * Retorna nullptr si el ítem sostenido no tiene modelo vóxel asociado (fallback automático a 2D).
 */
const struct RegisteredVoxelItem* item_registry_find(uint32_t item_id, uint32_t metadata);

/**
 * Obtiene la cantidad de ítems registrados en el sistema.
 */
uint32_t item_registry_count(void);

/**
 * Limpia y reinicia la tabla de búsqueda.
 */
void item_registry_clear(void);

#ifdef __cplusplus
}
#endif

#endif // ITEM_REGISTRY_H
