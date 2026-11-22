/**
 * Voxel3D-PS3: Plugin SPRX para Minecraft PlayStation 3 Edition
 * Motor agnóstico de renderizado 3D data-driven con resolución O(1),
 * cinemática ADS, físicas de retroceso y emisión directa en la GPU RSX.
 */

#include "voxel3d.h"
#include "hook.h"
#include "netlogger.h"
#include "rsx_pipeline.h"
#include "engine_structures.h"
#include "item_registry.h"
#include "kinematics.h"
#include "crayfish_transforms.h"
#include <stdint.h>
#include <string.h>

// Contexto global del hook de ItemRenderer
static HookContext g_item_renderer_hook;

// Offset tentativo para Minecraft PS3 Update 1.84 (BLES01976 / NPUB31419)
#define MINECRAFT_RENDER_ITEM_ADDR 0x0045A120

// Prototipo de la función original ItemRenderer::renderItemInFirstPerson
typedef void (*t_renderItemInFirstPerson)(ItemRenderer* itemRendererInstance, float partialTicks);

// Estado dinámico de cinemática para el arma activa del jugador
static struct WeaponKinematicsState g_player_weapon_state;

// Contador de frames para telemetría
static uint32_t s_render_call_count = 0;

/**
 * Hook Detour interfezado sobre ItemRenderer::renderItemInFirstPerson.
 * r3 = itemRendererInstance (puntero a ItemRenderer en C++)
 * f1 = partialTicks (interpolación de tiempo entre ticks de física)
 */
void hook_renderItemInFirstPerson(ItemRenderer* itemRendererInstance, float partialTicks) {
    if ((++s_render_call_count % 300) == 1) {
        netlog_debug("renderItemInFirstPerson (tick=%.3f, calls=%u)", 
                     (double)partialTicks, s_render_call_count);
    }

    t_renderItemInFirstPerson originalFunc = 
        (t_renderItemInFirstPerson)(void*)g_item_renderer_hook.trampoline_buffer;

    bool voxel_model_drawn = false;

    // 1. Validar si el motor de Minecraft está sosteniendo un ItemStack activo
    if (itemRendererInstance && itemRendererInstance->m_pItemToRender) {
        ItemStack* stack = itemRendererInstance->m_pItemToRender;

        if (stack->isValid()) {
            uint32_t item_id = stack->getId();
            uint32_t metadata = stack->getDamage();

            // 2. Resolución O(1) inmediata en tabla XDR
            const struct RegisteredVoxelItem* voxel_item = item_registry_find(item_id, metadata);

            if (voxel_item && g_gcm_context) {
                // 3. Actualizar cinemática LERP (ADS y recuperación de retroceso)
                kinematics_update(&g_player_weapon_state, voxel_item, 1.0f / 60.0f);

                // 4. Construir matriz combinada para el RSX
                float mvp_matrix[16];
                kinematics_build_matrix(&g_player_weapon_state, voxel_item, mvp_matrix);

                // 5. Emitir Draw Call RSX de bajo nivel
                voxel_model_drawn = rsx_draw_voxel_model(g_gcm_context, voxel_item->model_id, mvp_matrix);
            }
        }
    }

    // 6. Si no es un modelo vóxel 3D o falló el render, fallback limpio al renderizador 2D original
    if (!voxel_model_drawn && originalFunc) {
        originalFunc(itemRendererInstance, partialTicks);
    }
}

// Declaración de puntos de entrada del módulo PRX / SPRX de Cell OS
extern "C" {
    int voxel3d_prx_start(void) {
        // Inicializar telemetría UDP
        netlog_init("127.0.0.1", 3000);
        netlog_info("Inicializando Voxel3D-PS3 Universal Engine...");

        // 1. Inicializar tabla de búsqueda O(1) en memoria XDR
        item_registry_init();
        kinematics_init(&g_player_weapon_state);

        // 2. Inicializar el pool estático de 4 MB en memoria GDDR3 del RSX
        if (!rsx_init_vram_pool()) {
            netlog_error("Fallo crítico al reservar pool de VRAM para RSX.");
            return -1;
        }

        // 3. Registrar preset de armas iniciales (Agnósticas a la fuente)
        struct RegisteredVoxelItem pistol_item;
        memset(&pistol_item, 0, sizeof(pistol_item));
        pistol_item.item_id = 290; // Wooden Hoe / Voxel Pistol
        pistol_item.metadata = 0;
        pistol_item.model_id = 1;
        strncpy(pistol_item.name, "Voxel 3D Pistol", sizeof(pistol_item.name) - 1);
        pistol_item.scale[0] = 0.0625f; pistol_item.scale[1] = 0.0625f; pistol_item.scale[2] = 0.0625f;
        pistol_item.rest_pos[0] = 0.25f; pistol_item.rest_pos[1] = -0.20f; pistol_item.rest_pos[2] = 0.45f;
        pistol_item.ads_pos[0] = 0.0f; pistol_item.ads_pos[1] = -0.14f; pistol_item.ads_pos[2] = 0.28f;
        pistol_item.ads_fov_mult = 0.80f;
        pistol_item.recoil_kick_pos[1] = 0.02f; pistol_item.recoil_kick_pos[2] = -0.06f;
        pistol_item.recoil_kick_rot[0] = 8.0f;
        pistol_item.recoil_recovery_speed = 18.0f;
        item_registry_register(&pistol_item);

        struct RegisteredVoxelItem rifle_item;
        memset(&rifle_item, 0, sizeof(rifle_item));
        rifle_item.item_id = 291; // Stone Hoe / Voxel Rifle
        rifle_item.metadata = 0;
        rifle_item.model_id = 2;
        strncpy(rifle_item.name, "Voxel 3D Rifle", sizeof(rifle_item.name) - 1);
        rifle_item.scale[0] = 0.0625f; rifle_item.scale[1] = 0.0625f; rifle_item.scale[2] = 0.0625f;
        rifle_item.rest_pos[0] = 0.20f; rifle_item.rest_pos[1] = -0.22f; rifle_item.rest_pos[2] = 0.40f;
        rifle_item.ads_pos[0] = 0.0f; rifle_item.ads_pos[1] = -0.12f; rifle_item.ads_pos[2] = 0.25f;
        rifle_item.ads_fov_mult = 0.70f;
        rifle_item.recoil_kick_pos[1] = 0.03f; rifle_item.recoil_kick_pos[2] = -0.09f;
        rifle_item.recoil_kick_rot[0] = 12.0f;
        rifle_item.recoil_recovery_speed = 14.0f;
        item_registry_register(&rifle_item);

        // 4. Instalar hook de intercepción PPU
        bool success = install_ppu_hook(
            &g_item_renderer_hook,
            MINECRAFT_RENDER_ITEM_ADDR,
            (void*)hook_renderItemInFirstPerson
        );

        if (success) {
            netlog_info("PPU Hook instalado en 0x%08X (trampoline listo)", MINECRAFT_RENDER_ITEM_ADDR);
        } else {
            netlog_error("Fallo al instalar PPU Hook en 0x%08X", MINECRAFT_RENDER_ITEM_ADDR);
        }

        return success ? 0 : -1;
    }

    int voxel3d_prx_stop(void) {
        netlog_info("Deteniendo Voxel3D-PS3 SPRX plugin...");

        if (g_item_renderer_hook.installed) {
            uninstall_ppu_hook(&g_item_renderer_hook);
            netlog_info("PPU Hook desinstalado. Memoria original restaurada.");
        }

        rsx_free_vram_pool();
        item_registry_clear();

        netlog_info("Voxel3D-PS3 finalizado.");
        netlog_close();
        return 0;
    }
}
