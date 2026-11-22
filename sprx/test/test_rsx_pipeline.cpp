/**
 * Test unitario para verificar la lógica de RSX Pipeline,
 * carga de archivos .vx3 reales y alineación a 128 bytes.
 */

#include "rsx_pipeline.h"
#include "netlogger.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static void* read_file_bytes(const char* path, size_t* out_size) {
    FILE* f = fopen(path, "rb");
    if (!f) return nullptr;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    void* buf = malloc(sz);
    fread(buf, 1, sz, f);
    fclose(f);
    *out_size = (size_t)sz;
    return buf;
}

int main(void) {
    printf("[+] Iniciando verificación del Pipeline RSX y Pool GDDR3...\n");

    // 1. Inicializar pool de 4 MB
    bool pool_ok = rsx_init_vram_pool();
    assert(pool_ok);
    printf("  [✓] Pool estático de 4 MB reservado con éxito.\n");

    // 2. Cargar pistol.vx3 compilado
    size_t pistol_size = 0;
    void* pistol_data = read_file_bytes("../models/pistol.vx3", &pistol_size);
    assert(pistol_data != nullptr);
    int slot_pistol = rsx_load_model_to_vram(1, pistol_data, pistol_size);
    assert(slot_pistol >= 0);
    printf("  [✓] Modelo 1 (Pistol) cargado en VRAM (Slot %d, %zu bytes).\n", slot_pistol, pistol_size);

    // 3. Cargar rifle.vx3 compilado
    size_t rifle_size = 0;
    void* rifle_data = read_file_bytes("../models/rifle.vx3", &rifle_size);
    assert(rifle_data != nullptr);
    int slot_rifle = rsx_load_model_to_vram(2, rifle_data, rifle_size);
    assert(slot_rifle >= 0);
    printf("  [✓] Modelo 2 (Rifle) cargado en VRAM (Slot %d, %zu bytes).\n", slot_rifle, rifle_size);

    // 4. Verificar alineación obligatoria de hardware RSX a 128 bytes
    const struct VoxelModelSlot* pSlot1 = rsx_get_model_slot(1);
    const struct VoxelModelSlot* pSlot2 = rsx_get_model_slot(2);
    assert(pSlot1 != nullptr);
    assert(pSlot2 != nullptr);

    assert((pSlot1->vbo_offset % 128) == 0);
    assert((pSlot1->ibo_offset % 128) == 0);
    assert((pSlot2->vbo_offset % 128) == 0);
    assert((pSlot2->ibo_offset % 128) == 0);
    printf("  [✓] Verificación de alineación RSX (128 bytes) superada para VBO e IBO.\n");

    // 5. Simular Draw Call RSX
    CellGcmContextData dummy_ctx;
    rsx_set_active_context(&dummy_ctx);
    float dummy_matrix[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
    bool draw_ok = rsx_draw_voxel_model(&dummy_ctx, 1, dummy_matrix);
    assert(draw_ok);
    printf("  [✓] Draw Call RSX simulado emitido con éxito sin fuga de estado.\n");

    // 6. Liberar pool
    rsx_free_vram_pool();
    free(pistol_data);
    free(rifle_data);
    printf("  [✓] Pool de VRAM liberado correctamente.\n");

    printf("\n[SUCCESS] Todos los tests del pipeline RSX y gestión de VRAM pasaron exitosamente.\n");
    return 0;
}
