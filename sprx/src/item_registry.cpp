/**
 * Voxel3D-PS3: Tabla de Búsqueda O(1) Data-Driven en Memoria XDR
 * Permite asociar cualquier modelo vóxel 3D (.vx3) a cualquier ítem de Minecraft
 * con resolución inmediata en caliente por cada frame sin alocaciones dinámicas.
 */

#include "item_registry.h"
#include "netlogger.h"
#include <string.h>

// Tabla estática de almacenamiento
static struct RegisteredVoxelItem s_itemsPool[MAX_REGISTERED_ITEMS];
static uint32_t s_itemCount = 0;

// Tabla de resolución directa O(1) para IDs 0..1023
static int16_t s_directMap[ITEM_ID_DIRECT_MAX];

void item_registry_init(void) {
    memset(s_itemsPool, 0, sizeof(s_itemsPool));
    for (int i = 0; i < ITEM_ID_DIRECT_MAX; ++i) {
        s_directMap[i] = -1;
    }
    s_itemCount = 0;
    netlog_info("Tabla de búsqueda O(1) de ítems inicializada en XDR.");
}

bool item_registry_register(const struct RegisteredVoxelItem* item) {
    if (!item || item->item_id == 0 || s_itemCount >= MAX_REGISTERED_ITEMS) {
        netlog_error("No se pudo registrar ítem (límite alcanzado o ID inválido: %u).", 
                     item ? item->item_id : 0);
        return false;
    }

    // Comprobar si ya existe una entrada para actualizarla
    for (uint32_t i = 0; i < s_itemCount; ++i) {
        if (s_itemsPool[i].active && 
            s_itemsPool[i].item_id == item->item_id && 
            s_itemsPool[i].metadata == item->metadata) {
            s_itemsPool[i] = *item;
            s_itemsPool[i].active = true;
            netlog_info("Ítem actualizado en registro: '%s' (ID %u:%u ➔ Modelo %u)",
                        item->name, item->item_id, item->metadata, item->model_id);
            return true;
        }
    }

    uint32_t slot = s_itemCount++;
    s_itemsPool[slot] = *item;
    s_itemsPool[slot].active = true;

    // Actualizar tabla directa O(1) si está dentro del rango estándar de Minecraft
    if (item->item_id < ITEM_ID_DIRECT_MAX) {
        s_directMap[item->item_id] = (int16_t)slot;
    }

    netlog_info("Nuevo ítem registrado: '%s' (ID %u:%u ➔ Modelo %u) [Total: %u]",
                item->name, item->item_id, item->metadata, item->model_id, s_itemCount);
    return true;
}

const struct RegisteredVoxelItem* item_registry_find(uint32_t item_id, uint32_t metadata) {
    // 1. Acceso directo O(1) por array estático indexado
    if (item_id < ITEM_ID_DIRECT_MAX) {
        int16_t slot = s_directMap[item_id];
        if (slot >= 0 && slot < (int16_t)MAX_REGISTERED_ITEMS) {
            const struct RegisteredVoxelItem* candidate = &s_itemsPool[slot];
            if (candidate->active && candidate->metadata == metadata) {
                return candidate;
            }
        }
    }

    // 2. Búsqueda para metadatos secundarios o IDs mayores a 1024
    for (uint32_t i = 0; i < s_itemCount; ++i) {
        if (s_itemsPool[i].active && 
            s_itemsPool[i].item_id == item_id && 
            s_itemsPool[i].metadata == metadata) {
            return &s_itemsPool[i];
        }
    }

    return nullptr; // No registrado -> renderizador original de Minecraft
}

uint32_t item_registry_count(void) {
    return s_itemCount;
}

void item_registry_clear(void) {
    item_registry_init();
}
