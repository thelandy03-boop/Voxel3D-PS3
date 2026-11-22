#ifndef RSX_PIPELINE_H
#define RSX_PIPELINE_H

#include "gcm_types.h"
#include "voxel3d.h"
#include <stdbool.h>

#define VOXEL3D_VRAM_POOL_SIZE (4 * 1024 * 1024) // 4 MB static VRAM GDDR3 pool
#define VOXEL3D_MAX_MODELS 64

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Slot de modelo vóxel residente en el pool de memoria de la GPU RSX.
 */
struct VoxelModelSlot {
    uint32_t model_id;
    uint32_t vbo_offset;       // Offset en bus RSX del Vertex Buffer
    uint32_t ibo_offset;       // Offset en bus RSX del Index Buffer
    uint32_t vertex_count;
    uint32_t index_count;
    uint32_t location;         // CELL_GCM_LOCATION_LOCAL o CELL_GCM_LOCATION_MAIN
    bool     is_loaded;
};

// Puntero global al contexto CellGcmContextData activo del motor de 4J Studios
extern CellGcmContextData* g_gcm_context;

/**
 * Asigna el contexto GCM interceptado del motor del juego.
 */
void rsx_set_active_context(CellGcmContextData* ctx);

/**
 * Inicializa el pool estático cerrado de 4 MB en memoria local GDDR3.
 * Si la GDDR3 está saturada, ejecuta fallback a memoria principal XDR con mapeo IO.
 */
bool rsx_init_vram_pool(void);

/**
 * Sincroniza la GPU con cellGcmFinish y libera los buffers asignados.
 */
void rsx_free_vram_pool(void);

/**
 * Carga un modelo binario .vx3 en el pool respetando la alineación obligatoria de 128 bytes.
 * Retorna el índice de slot asignado o -1 en caso de error.
 */
int rsx_load_model_to_vram(uint32_t model_id, const void* vx3_file_data, size_t file_size);

/**
 * Obtiene el descriptor en memoria del modelo solicitado.
 */
const struct VoxelModelSlot* rsx_get_model_slot(uint32_t model_id);

/**
 * Configura el estado del rasterizador RSX para geometría vóxel 3D sólida:
 * Depth LEQUAL, Depth Mask enabled, Cull Face BACK, Alpha Test > 0.1
 */
void rsx_setup_voxel_rasterizer(CellGcmContextData* ctx);

/**
 * Restaura el estado del rasterizador RSX y desvincula los atributos secundarios (1, 2, 3)
 * para garantizar "Cero Fuga de Estado" antes de devolver el control al motor de 4J Studios.
 */
void rsx_restore_rasterizer(CellGcmContextData* ctx);

/**
 * Enlaza los 4 canales de hardware RSX con sus offsets y emite la llamada de dibujo indexada
 * cellGcmSetDrawIndexArray() dentro del anillo de comandos FIFO activo.
 */
bool rsx_draw_voxel_model(CellGcmContextData* ctx, uint32_t model_id, const float* mvp_matrix);

#ifdef __cplusplus
}
#endif

#endif // RSX_PIPELINE_H
