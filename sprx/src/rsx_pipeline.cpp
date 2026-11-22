/**
 * Voxel3D-PS3: Pipeline de Renderizado RSX de Bajo Nivel (libgcm)
 * Gestión del pool de 4 MB en memoria GDDR3 local, configuración de atributos
 * de hardware y emisión de Draw Calls indexados para el Reality Synthesizer.
 */

#include "rsx_pipeline.h"
#include "netlogger.h"
#include <string.h>
#include <stdlib.h>

#if defined(__CELLOS_LV2__) && !defined(VOXEL3D_HOST_BUILD)
#include <cell/gcm.h>
#include <sys/memory.h>
#else
// Declaraciones de funciones mock para entornos sin toolchain Cell
static inline int32_t cellGcmAddressToOffset(const void* address, uint32_t* offset) {
    *offset = (uint32_t)(uintptr_t)address;
    return 0;
}
static inline void cellGcmSetVertexDataArray(CellGcmContextData*, uint8_t, uint16_t, uint8_t, uint8_t, uint8_t, uint8_t, uint32_t) {}
static inline void cellGcmSetDrawIndexArray(CellGcmContextData*, uint8_t, uint32_t, uint8_t, uint8_t, uint32_t) {}
static inline void cellGcmSetDepthTestEnable(CellGcmContextData*, uint32_t) {}
static inline void cellGcmSetDepthFunc(CellGcmContextData*, uint32_t) {}
static inline void cellGcmSetDepthMask(CellGcmContextData*, uint32_t) {}
static inline void cellGcmSetCullFaceEnable(CellGcmContextData*, uint32_t) {}
static inline void cellGcmSetCullFace(CellGcmContextData*, uint32_t) {}
static inline void cellGcmSetAlphaTestEnable(CellGcmContextData*, uint32_t) {}
static inline void cellGcmSetAlphaFunc(CellGcmContextData*, uint32_t, uint32_t) {}
static inline void cellGcmSetVertexProgramConstants(CellGcmContextData*, uint32_t, uint32_t, const float*) {}
static inline int32_t cellGcmFinish(CellGcmContextData*, uint32_t) { return 0; }
#endif

// Contexto GCM activo capturado del motor de 4J Studios
CellGcmContextData* g_gcm_context = nullptr;

// Puntero base y estado del pool estático de 4 MB
static uint8_t* s_pVramLocalBase = nullptr;
static uint32_t s_vramAllocatedOffset = 0;
static uint32_t s_memoryLocation = CELL_GCM_LOCATION_LOCAL;

// Tabla de slots para resolución O(1) de mallas vóxel residentes
static struct VoxelModelSlot s_modelSlots[VOXEL3D_MAX_MODELS];
static uint32_t s_registeredModelsCount = 0;

void rsx_set_active_context(CellGcmContextData* ctx) {
    g_gcm_context = ctx;
    netlog_debug("Contexto GCM activo asignado: %p", ctx);
}

bool rsx_init_vram_pool(void) {
    if (s_pVramLocalBase) {
        netlog_warn("El pool de VRAM ya se encuentra inicializado.");
        return true;
    }

    // Inicializar tabla de slots
    memset(s_modelSlots, 0, sizeof(s_modelSlots));
    s_registeredModelsCount = 0;
    s_vramAllocatedOffset = 0;

#if defined(__CELLOS_LV2__) && !defined(VOXEL3D_HOST_BUILD)
    CellGcmConfig gcmConfig;
    cellGcmGetConfiguration(&gcmConfig);

    // Intentar alocar bloque contiguo de 4 MB en memoria local GDDR3 (CELL_GCM_LOCATION_LOCAL)
    // En caso de saturación de GDDR3, fallback hacia memoria principal XDR mapeada
    sys_addr_t sys_addr = 0;
    int ret = sys_memory_allocate(VOXEL3D_VRAM_POOL_SIZE, SYS_MEMORY_PAGE_SIZE_64K, &sys_addr);
    if (ret == 0 && sys_addr != 0) {
        s_pVramLocalBase = (uint8_t*)sys_addr;
        s_memoryLocation = CELL_GCM_LOCATION_MAIN;
        cellGcmMapMainMemory(s_pVramLocalBase, VOXEL3D_VRAM_POOL_SIZE, &gcmConfig.ioAddress);
        netlog_info("Pool de 4 MB reservado en XDR Main Memory (IO Mapped: 0x%08X)", gcmConfig.ioAddress);
    } else {
        netlog_error("Fallo crítico al reservar 4 MB para el pool de VRAM (error: 0x%08X)", ret);
        return false;
    }
#else
    // Host / test allocation con alineación a 128 bytes
    void* ptr = nullptr;
    if (posix_memalign(&ptr, 128, VOXEL3D_VRAM_POOL_SIZE) == 0 && ptr) {
        s_pVramLocalBase = (uint8_t*)ptr;
        s_memoryLocation = CELL_GCM_LOCATION_LOCAL;
        netlog_info("Pool estático de 4 MB reservado en memoria local simulada (%p)", ptr);
    } else {
        netlog_error("Fallo de alocación de memoria alineada para el pool de VRAM.");
        return false;
    }
#endif

    return true;
}

void rsx_free_vram_pool(void) {
    if (!s_pVramLocalBase) {
        return;
    }

    netlog_info("Sincronizando pipeline RSX antes de liberar el pool...");
    if (g_gcm_context) {
        cellGcmFinish(g_gcm_context, 0);
    }

    // Desregistrar y limpiar memoria
    memset(s_modelSlots, 0, sizeof(s_modelSlots));
    s_registeredModelsCount = 0;
    s_vramAllocatedOffset = 0;

#if defined(__CELLOS_LV2__) && !defined(VOXEL3D_HOST_BUILD)
    if (s_memoryLocation == CELL_GCM_LOCATION_MAIN) {
        cellGcmUnmapMainMemory(s_pVramLocalBase);
        sys_memory_free((sys_addr_t)s_pVramLocalBase);
    }
#else
    free(s_pVramLocalBase);
#endif

    s_pVramLocalBase = nullptr;
    netlog_info("Pool de VRAM liberado correctamente.");
}

int rsx_load_model_to_vram(uint32_t model_id, const void* vx3_file_data, size_t file_size) {
    if (!s_pVramLocalBase || !vx3_file_data || file_size < sizeof(struct VX3Header)) {
        netlog_error("Parámetros inválidos para carga de modelo .vx3.");
        return -1;
    }

    if (s_registeredModelsCount >= VOXEL3D_MAX_MODELS) {
        netlog_error("Capacidad máxima de modelos residentes alcanzada (%u).", VOXEL3D_MAX_MODELS);
        return -1;
    }

    const struct VX3Header* header = (const struct VX3Header*)vx3_file_data;

    // Validación de Magic ("VX3D") y versión
    if (memcmp(header->magic, "VX3D", 4) != 0 || header->version != 1) {
        netlog_error("Cabecera .vx3 corrupta o incompatible.");
        return -1;
    }

    size_t vertex_bytes = header->vertex_count * sizeof(struct VoxelVertex);
    size_t index_bytes = header->index_count * sizeof(uint16_t);
    size_t required_payload = sizeof(struct VX3Header) + vertex_bytes + index_bytes;

    if (file_size < required_payload) {
        netlog_error("Tamaño de archivo insuficiente para modelo %u (%zu < %zu bytes).", 
                     model_id, file_size, required_payload);
        return -1;
    }

    // Requisito de hardware RSX: Alinear Vertex Buffer e Index Buffer a 128 bytes
    uint32_t aligned_vbo_offset = (s_vramAllocatedOffset + 127) & ~127;
    uint32_t aligned_ibo_offset = (aligned_vbo_offset + (uint32_t)vertex_bytes + 127) & ~127;
    uint32_t next_free_offset   = aligned_ibo_offset + (uint32_t)index_bytes;

    if (next_free_offset > VOXEL3D_VRAM_POOL_SIZE) {
        netlog_error("Desbordamiento de memoria en VRAM pool (%u bytes requeridos, %u disponibles).",
                     next_free_offset, VOXEL3D_VRAM_POOL_SIZE);
        return -1;
    }

    // Punteros de origen en el archivo .vx3
    const uint8_t* pRawVertices = (const uint8_t*)vx3_file_data + sizeof(struct VX3Header);
    const uint8_t* pRawIndices  = pRawVertices + vertex_bytes;

    // Copiar hacia el bloque contiguo de VRAM
    memcpy(s_pVramLocalBase + aligned_vbo_offset, pRawVertices, vertex_bytes);
    memcpy(s_pVramLocalBase + aligned_ibo_offset, pRawIndices, index_bytes);

    // Calcular offsets de bus para el RSX mediante cellGcmAddressToOffset
    uint32_t rsx_vbo_offset = 0;
    uint32_t rsx_ibo_offset = 0;
    cellGcmAddressToOffset(s_pVramLocalBase + aligned_vbo_offset, &rsx_vbo_offset);
    cellGcmAddressToOffset(s_pVramLocalBase + aligned_ibo_offset, &rsx_ibo_offset);

    // Registrar en slot libre
    uint32_t slot_idx = s_registeredModelsCount++;
    struct VoxelModelSlot* slot = &s_modelSlots[slot_idx];
    slot->model_id     = model_id;
    slot->vbo_offset   = rsx_vbo_offset;
    slot->ibo_offset   = rsx_ibo_offset;
    slot->vertex_count = header->vertex_count;
    slot->index_count  = header->index_count;
    slot->location     = s_memoryLocation;
    slot->is_loaded    = true;

    s_vramAllocatedOffset = next_free_offset;

    netlog_info("Modelo %u cargado en VRAM [Slot %u]: %u vértices (%zu B), %u índices (%zu B). VRAM libre: %u KB",
                model_id, slot_idx, slot->vertex_count, vertex_bytes, slot->index_count, index_bytes,
                (VOXEL3D_VRAM_POOL_SIZE - s_vramAllocatedOffset) / 1024);

    return (int)slot_idx;
}

const struct VoxelModelSlot* rsx_get_model_slot(uint32_t model_id) {
    for (uint32_t i = 0; i < s_registeredModelsCount; ++i) {
        if (s_modelSlots[i].model_id == model_id && s_modelSlots[i].is_loaded) {
            return &s_modelSlots[i];
        }
    }
    return nullptr;
}

void rsx_setup_voxel_rasterizer(CellGcmContextData* ctx) {
    if (!ctx) return;

    // Habilitar Depth Test y Depth Write
    cellGcmSetDepthTestEnable(ctx, CELL_GCM_TRUE);
    cellGcmSetDepthFunc(ctx, CELL_GCM_LEQUAL);
    cellGcmSetDepthMask(ctx, CELL_GCM_TRUE);

    // Activar Culling de caras traseras (CCW)
    cellGcmSetCullFaceEnable(ctx, CELL_GCM_TRUE);
    cellGcmSetCullFace(ctx, CELL_GCM_BACK);

    // Habilitar Alpha Test para descartar fragmentos transparentes
    cellGcmSetAlphaTestEnable(ctx, CELL_GCM_TRUE);
    cellGcmSetAlphaFunc(ctx, CELL_GCM_GREATER, 0x1A); // Alpha > 0.1
}

void rsx_restore_rasterizer(CellGcmContextData* ctx) {
    if (!ctx) return;

    // Restaurar depth write
    cellGcmSetDepthMask(ctx, CELL_GCM_TRUE);

    // Desvincular atributos 1, 2 y 3 para evitar fugas en el renderizado del brazo o HUD
    cellGcmSetVertexDataArray(ctx, 1, 0, 0, 0, CELL_GCM_VERTEX_F, CELL_GCM_LOCATION_LOCAL, 0);
    cellGcmSetVertexDataArray(ctx, 2, 0, 0, 0, CELL_GCM_VERTEX_F, CELL_GCM_LOCATION_LOCAL, 0);
    cellGcmSetVertexDataArray(ctx, 3, 0, 0, 0, CELL_GCM_VERTEX_F, CELL_GCM_LOCATION_LOCAL, 0);
}

bool rsx_draw_voxel_model(CellGcmContextData* ctx, uint32_t model_id, const float* mvp_matrix) {
    if (!ctx) return false;

    const struct VoxelModelSlot* slot = rsx_get_model_slot(model_id);
    if (!slot || !slot->is_loaded) {
        return false; // Fallback seguro a render 2D
    }

    // Configurar matriz MVP en registros uniformes C0-C3 del Vertex Program
    if (mvp_matrix) {
        cellGcmSetVertexProgramConstants(ctx, 0, 16, mvp_matrix);
    }

    // Configurar estado del rasterizador
    rsx_setup_voxel_rasterizer(ctx);

    // Enlace de los 4 canales de atributos de hardware de VoxelVertex (14 bytes stride)
    const uint8_t stride = (uint8_t)sizeof(struct VoxelVertex); // 14 bytes
    const uint32_t loc   = slot->location;

    // ATTR 0: Posición (x, y, z) -> S16 (Offset +0)
    cellGcmSetVertexDataArray(ctx, 0, 0, stride, 3, CELL_GCM_VERTEX_S16, loc, slot->vbo_offset + 0);

    // ATTR 1: Normal (nx, ny, nz) -> UB normalizado (Offset +6)
    cellGcmSetVertexDataArray(ctx, 1, 0, stride, 3, CELL_GCM_VERTEX_UB, loc, slot->vbo_offset + 6);

    // ATTR 2: Flags (ao / surface) -> UB (Offset +9)
    cellGcmSetVertexDataArray(ctx, 2, 0, stride, 1, CELL_GCM_VERTEX_UB, loc, slot->vbo_offset + 9);

    // ATTR 3: UV / Paleta (u, v) -> U16 (Offset +10)
    cellGcmSetVertexDataArray(ctx, 3, 0, stride, 2, CELL_GCM_VERTEX_U16, loc, slot->vbo_offset + 10);

    // Emisión de 1 Draw Call indexado dentro del anillo FIFO activo
    cellGcmSetDrawIndexArray(
        ctx,
        CELL_GCM_PRIMITIVE_TRIANGLES,
        slot->index_count,
        CELL_GCM_DRAW_INDEX_ARRAY_TYPE_16,
        loc,
        slot->ibo_offset
    );

    // Restauración de estado sin fuga
    rsx_restore_rasterizer(ctx);

    return true;
}
