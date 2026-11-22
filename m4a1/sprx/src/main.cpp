// Voxel3D-PS3 :: plugin SPRX del M4A1 para Minecraft PS3 (BLES01976)
// Fase 4: carga estatica del .vx3 en CELL_GCM_LOCATION_LOCAL, draw indexado
// con libgcm, ADS con L2 y retroceso con R2 usando interpolacion LERP.

#include <cell/gcm.h>
#include <cell/pad.h>
#include <sys/moduleexport.h>
#include <sys/prx.h>
#include <sys/sys_time.h>

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "hook.h"
#include "offsets.h"
#include "vx3_format.h"

// Diagnosticos de carga/hook. printf ya forma parte de las importaciones del
// PRX (libc), asi que activarlo no anade dependencias nuevas.
#define VOX3_DEBUG_LOG 1

// Fallbacks por si la SDK no expone estas constantes.
#ifndef CELL_PAD_BTN_OFFSET_DIGITAL2
#define CELL_PAD_BTN_OFFSET_DIGITAL2 3
#endif
#ifndef CELL_PAD_CTRL_L2
#define CELL_PAD_CTRL_L2 0x00000100u
#endif
#ifndef CELL_PAD_CTRL_R2
#define CELL_PAD_CTRL_R2 0x00000200u
#endif

// Generado por el Makefile con objcopy desde build_ps3/assets/m4a1.vx3.
extern "C" {
    extern const uint8_t _binary_assets_m4a1_vx3_start[];
    extern const uint8_t _binary_assets_m4a1_vx3_end[];
}

static const uint32_t VX3_VERT_COUNT = 1742u;
static const uint32_t VX3_INDEX_COUNT = 3492u;
static const uint32_t VX3_VERT_BYTES = VX3_VERT_COUNT * VX3_STRIDE;
static const uint32_t VX3_INDEX_BYTES = VX3_INDEX_COUNT * sizeof(uint16_t);

static void*    g_vram       = 0;
static uint32_t g_vtx_offset = 0;
static uint32_t g_idx_offset = 0;
static bool     g_uploaded   = false;
static uint64_t g_last_time  = 0;

static float g_ads          = 0.0f;   // 0 = cadera, 1 = ADS (L2)
static float g_recoil       = 0.0f;   // 0 = reposo, 1 = retroceso (R2)
static int   g_kick_ticks   = 0;

// ---------------------------------------------------------------------------
static inline float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

static inline float clamp01(float v) {
    return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
}

static float frame_delta(void) {
    const uint64_t now = (uint64_t)sys_time_get_system_time();
    if (g_last_time == 0) g_last_time = now;
    const uint64_t d = now - g_last_time;
    g_last_time = now;

    float dt = (float)d / 1000000.0f;
    if (dt <= 0.0f || dt > 0.1f) dt = 1.0f / 60.0f;
    return dt;
}

static bool pad_down(uint32_t mask) {
    CellPadData data;
    memset(&data, 0, sizeof(data));
    if (cellPadGetData(0, &data) < 0) return false;
    return (data.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & mask) != 0;
}

// ---------------------------------------------------------------------------
// Matriz 4x4 columna-mayor, compatible con CELL_GCM / Cg.
// ---------------------------------------------------------------------------
static void mat_identity(float m[16]) {
    memset(m, 0, 16 * sizeof(float));
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

static void mat_mul(float out[16], const float a[16], const float b[16]) {
    for (int c = 0; c < 4; ++c) {
        for (int r = 0; r < 4; ++r) {
            float s = 0.0f;
            for (int k = 0; k < 4; ++k) {
                s += a[k * 4 + r] * b[c * 4 + k];
            }
            out[c * 4 + r] = s;
        }
    }
}

static void mat_translate(float m[16], float x, float y, float z) {
    mat_identity(m);
    m[12] = x; m[13] = y; m[14] = z;
}

static void mat_scale(float m[16], float x, float y, float z) {
    mat_identity(m);
    m[0] = x; m[5] = y; m[10] = z;
}

static void mat_rot_x(float m[16], float a) {
    mat_identity(m);
    const float c = cosf(a), s = sinf(a);
    m[5] = c;  m[6] = s;
    m[9] = -s; m[10] = c;
}

static void mat_rot_y(float m[16], float a) {
    mat_identity(m);
    const float c = cosf(a), s = sinf(a);
    m[0] = c;  m[2] = -s;
    m[8] = s;  m[10] = c;
}

static void mat_rot_z(float m[16], float a) {
    mat_identity(m);
    const float c = cosf(a), s = sinf(a);
    m[0] = c;  m[1] = s;
    m[4] = -s; m[5] = c;
}

// ---------------------------------------------------------------------------
// Carga unica del asset en memoria local de la RSX.
// El header se valida y se descarta; vertices e indices se copian tal cual
// (ya estan en Big-Endian, formato nativo del PPU).
// ---------------------------------------------------------------------------
static bool upload_asset(void) {
    const uint8_t* p = _binary_assets_m4a1_vx3_start;
    const uint8_t* e = _binary_assets_m4a1_vx3_end;
    const size_t total = (size_t)(e - p);
    const size_t need = VX3_HEADER_SIZE + VX3_VERT_BYTES + VX3_INDEX_BYTES;

    if (total < need) return false;

    const VX3Header* hdr = reinterpret_cast<const VX3Header*>(p);
    if (memcmp(hdr->magic, "VX3D", 4) != 0 || hdr->version != 1) return false;
    if (hdr->vertex_count != VX3_VERT_COUNT ||
        hdr->index_count != VX3_INDEX_COUNT) return false;

    void*  local_base = 0;
    uint32_t local_size = 0;
    if (cellGcmMapLocalMemory(&local_base, &local_size) < 0 || !local_base ||
        local_size < VX3_VERT_BYTES + VX3_INDEX_BYTES) return false;
    g_vram = local_base;

    const uint8_t* verts = p + VX3_HEADER_SIZE;
    const uint8_t* inds  = verts + VX3_VERT_BYTES;

    memcpy(g_vram, verts, VX3_VERT_BYTES);
    memcpy(static_cast<uint8_t*>(g_vram) + VX3_VERT_BYTES, inds,
           VX3_INDEX_BYTES);

    if (cellGcmAddressToOffset(g_vram, &g_vtx_offset) < 0) return false;
    g_idx_offset = g_vtx_offset + VX3_VERT_BYTES;

    g_uploaded = true;
    return true;
}

// Layout de atributos segun RSX_PIPELINE.md / VX3_FORMAT.md.
static void bind_vertex_format(CellGcmContextData* ctx) {
    //ATTR0: posicion S16 x3 @ 0
    cellGcmSetVertexDataArray(ctx, 0, 0, VX3_STRIDE, 3,
                              CELL_GCM_VERTEX_S16_NR,
                              CELL_GCM_LOCATION_LOCAL, g_vtx_offset + 0);
    // ATTR1: normal S8 x3 @ 6 (se enlaza como UB; el shader interpreta la firma)
    cellGcmSetVertexDataArray(ctx, 1, 0, VX3_STRIDE, 3,
                              CELL_GCM_VERTEX_UB,
                              CELL_GCM_LOCATION_LOCAL, g_vtx_offset + 6);
    // ATTR2: material/flags U8 x1 @ 9
    cellGcmSetVertexDataArray(ctx, 2, 0, VX3_STRIDE, 1,
                              CELL_GCM_VERTEX_UB,
                              CELL_GCM_LOCATION_LOCAL, g_vtx_offset + 9);
    // ATTR3: UV U8 x2 @ 10 (3 uint8 finales: material,u,v)
    cellGcmSetVertexDataArray(ctx, 3, 0, VX3_STRIDE, 2,
                              CELL_GCM_VERTEX_UB,
                              CELL_GCM_LOCATION_LOCAL, g_vtx_offset + 10);
}

// ADS (L2) y retroceso (R2), ambos con LERP.
static void update_input(float dt) {
    // ADS: L2 mantenido -> objetivo 1; suelto -> objetivo 0.
    const float ads_target = pad_down(CELL_PAD_CTRL_L2) ? 1.0f : 0.0f;
    const float ads_step = VOX3_ADS_SPEED * dt;
    if (ads_target > g_ads)      g_ads += fminf(ads_step, ads_target - g_ads);
    else if (ads_target < g_ads) g_ads -= fminf(ads_step, g_ads - ads_target);

    // R2: cada pulsacion genera un kick de VOX3_RECOIL_FRAMES frames.
    if (pad_down(CELL_PAD_CTRL_R2) && g_kick_ticks <= 0) {
        g_kick_ticks = VOX3_RECOIL_FRAMES;
    }
    if (g_kick_ticks > 0) {
        g_recoil = lerp(g_recoil, 1.0f,
                        fminf(1.0f, VOX3_RECOIL_KICK * dt));
        --g_kick_ticks;
    } else {
        g_recoil = lerp(g_recoil, 0.0f,
                        fminf(1.0f, VOX3_RECOIL_RECOVER * dt));
    }

    g_ads = clamp01(g_ads);
    g_recoil = clamp01(g_recoil);
}

static void build_model_matrix(float out[16]) {
    const float px = lerp(VOX3_HIP_X, VOX3_ADS_X, g_ads)   +
                     g_recoil * VOX3_RECOIL_POS_X;
    const float py = lerp(VOX3_HIP_Y, VOX3_ADS_Y, g_ads)   +
                     g_recoil * VOX3_RECOIL_POS_Y;
    const float pz = lerp(VOX3_HIP_Z, VOX3_ADS_Z, g_ads)   +
                     g_recoil * VOX3_RECOIL_POS_Z;

    const float rx = VOX3_DEG2RAD(lerp(VOX3_HIP_ROT_X, VOX3_ADS_ROT_X, g_ads)) +
                     g_recoil * VOX3_DEG2RAD(VOX3_RECOIL_ROT_X);
    const float ry = VOX3_DEG2RAD(lerp(VOX3_HIP_ROT_Y, VOX3_ADS_ROT_Y, g_ads)) +
                     g_recoil * VOX3_DEG2RAD(VOX3_RECOIL_ROT_Y);
    const float rz = VOX3_DEG2RAD(lerp(VOX3_HIP_ROT_Z, VOX3_ADS_ROT_Z, g_ads)) +
                     g_recoil * VOX3_DEG2RAD(VOX3_RECOIL_ROT_Z);

    float T[16], S[16], RX[16], RY[16], RZ[16], R1[16], R[16], M[16];
    mat_translate(T, px, py, pz);
    mat_scale(S, VOX3_SCALE, VOX3_SCALE, VOX3_SCALE);
    mat_rot_x(RX, rx);
    mat_rot_y(RY, ry);
    mat_rot_z(RZ, rz);

    mat_mul(R1, RY, RX);
    mat_mul(R,  RZ, R1);
    mat_mul(M,  T,  R);
    mat_mul(out, M, S);
}

static void draw_weapon(CellGcmContextData* ctx) {
    float model[16];
    build_model_matrix(model);

    bind_vertex_format(ctx);
    cellGcmSetVertexProgramConstants(ctx, 0, 16, model);

    // Alpha-test, sin blending (requisito Voxel3D-PS3).
    cellGcmSetAlphaTestEnable(ctx, CELL_GCM_TRUE);
    cellGcmSetAlphaFunc(ctx, CELL_GCM_GREATER, 0x1A);
    cellGcmSetDepthTestEnable(ctx, CELL_GCM_TRUE);
    cellGcmSetDepthFunc(ctx, CELL_GCM_LEQUAL);
    cellGcmSetDepthMask(ctx, CELL_GCM_TRUE);
    cellGcmSetCullFaceEnable(ctx, CELL_GCM_TRUE);
    cellGcmSetCullFace(ctx, CELL_GCM_BACK);

    // Una sola pasada indexada desde CELL_GCM_LOCATION_LOCAL.
    cellGcmSetDrawIndexArray(ctx, CELL_GCM_PRIMITIVE_TRIANGLES, VX3_INDEX_COUNT,
                             CELL_GCM_DRAW_INDEX_ARRAY_TYPE_16, CELL_GCM_LOCATION_LOCAL,
                             g_idx_offset);

    cellGcmSetAlphaTestEnable(ctx, CELL_GCM_FALSE);
}

// Llamado por el trampoline despues del prologo original.
extern "C" void vox3_render_hook(void* arg) {
    const Vox3HookArgs* hook_args = static_cast<const Vox3HookArgs*>(arg);
    (void)hook_args;
#if VOX3_DEBUG_LOG
    static bool s_logged = false;
    if (!s_logged && hook_args) {
        s_logged = true;
        printf("[M4A1] render hook hit: r3=%08x r4=%08x r5=%08x r6=%08x "
               "r7=%08x r8=%08x r9=%08x r10=%08x\n",
               (unsigned int)hook_args->r3, (unsigned int)hook_args->r4,
               (unsigned int)hook_args->r5, (unsigned int)hook_args->r6,
               (unsigned int)hook_args->r7, (unsigned int)hook_args->r8,
               (unsigned int)hook_args->r9, (unsigned int)hook_args->r10);
        fflush(stdout);
    }
#endif

    // gCellGcmCurrentContext es la copia LOCAL de la libreria del PRX y el
    // juego nunca la inicializa: siempre seria NULL. Leemos la global real del
    // EBOOT (rellenada por las relocaciones/RPCS3 al iniciar cellGcm).
    const volatile uint32_t* ctx_slot =
        reinterpret_cast<const volatile uint32_t*>(VOX3_GCM_CONTEXT_GLOBAL);
    CellGcmContextData* ctx =
        reinterpret_cast<CellGcmContextData*>(static_cast<uintptr_t>(*ctx_slot));

#if VOX3_DEBUG_LOG
    static bool s_ctx_logged = false;
    if (!s_ctx_logged) {
        s_ctx_logged = true;
        printf("[M4A1] game gcm ctx global %08x -> %08x (local=%08x)\n",
               VOX3_GCM_CONTEXT_GLOBAL, (unsigned int)(uintptr_t)ctx,
               (unsigned int)(uintptr_t)gCellGcmCurrentContext);
        fflush(stdout);
    }
#endif

    if (!ctx) return;

    if (!g_uploaded && !upload_asset()) {
#if VOX3_DEBUG_LOG
        static bool s_upload_logged = false;
        if (!s_upload_logged) {
            s_upload_logged = true;
            printf("[M4A1] upload_asset FAILED\n");
            fflush(stdout);
        }
#endif
        return;
    }

#if VOX3_DEBUG_LOG
    static bool s_uploaded_logged = false;
    if (!s_uploaded_logged) {
        s_uploaded_logged = true;
        printf("[M4A1] upload_asset OK vtx=%08x idx=%08x\n",
               g_vtx_offset, g_idx_offset);
        fflush(stdout);
    }
#endif

    update_input(frame_delta());
    draw_weapon(ctx);
}

// ---------------------------------------------------------------------------
// Identidad y puntos de entrada del modulo SPRX (Lv-2 PRX).
// ---------------------------------------------------------------------------
SYS_MODULE_INFO(voxel3d, 0, 1, 0);
SYS_MODULE_START(vox3_prx_start);
SYS_MODULE_STOP(vox3_prx_stop);

extern "C" int vox3_prx_start(size_t args, void* argp) {
    (void)args;
    (void)argp;
#if VOX3_DEBUG_LOG
    printf("[M4A1] voxel3d.sprx loaded, installing hook at 0x%08x\n",
           VOX3_RENDER_ITEM_FIRST_PERSON);
    fflush(stdout);
#endif
    cellPadInit(1);
    if (vox3_hook_install(VOX3_RENDER_ITEM_FIRST_PERSON,
                          &vox3_render_hook) != 0) {
        return SYS_PRX_NO_RESIDENT;
    }
    return SYS_PRX_RESIDENT;
}

extern "C" int vox3_prx_stop(size_t args, void* argp) {
    (void)args;
    (void)argp;
    vox3_hook_remove();
    cellPadEnd();
    return SYS_PRX_STOP_OK;
}
