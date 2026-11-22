/**
 * Voxel3D-PS3: Plugin SPRX para Minecraft PlayStation 3 Edition
 * Motor agnóstico de renderizado 3D data-driven con resolución O(1),
 * sondeo de DualShock 3 (L2 ADS / R1 Disparo), cinemática LERP,
 * Muzzle Flash y emisión de bajo nivel en la GPU RSX.
 */

#include "voxel3d.h"
#include "hook.h"
#include "netlogger.h"
#include "rsx_pipeline.h"
#include "engine_structures.h"
#include "item_registry.h"
#include "kinematics.h"
#include "muzzle_flash.h"
#include "input_manager.h"
#include "config_loader.h"
#include "crayfish_transforms.h"
#include "offsets.h"
#include <stdint.h>
#include <string.h>

// Contexto global del hook de ItemRenderer
static HookContext g_item_renderer_hook;

// Offset verificado para Minecraft PS3 Update 1.84 (BLES01976)
#define MINECRAFT_RENDER_ITEM_ADDR VOX3_HOOK_RENDER_ITEM_FIRST_PERSON

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

    // Adquirir el puntero global CellGcmContextData de 4J Studios si aún no está asignado
    if (!g_gcm_context) {
#if defined(__CELLOS_LV2__) && !defined(VOXEL3D_HOST_BUILD)
        CellGcmContextData** ppGcmGlobal = (CellGcmContextData**)VOX3_GCM_CONTEXT_GLOBAL;
        if (ppGcmGlobal && *ppGcmGlobal) {
            g_gcm_context = *ppGcmGlobal;
            netlog_info("Contexto GCM capturado de global @0x%08X -> %p", 
                        VOX3_GCM_CONTEXT_GLOBAL, g_gcm_context);
        }
#endif
    }

    // 1. Validar si el motor de Minecraft está sosteniendo un ItemStack activo
    if (itemRendererInstance && itemRendererInstance->m_pItemToRender) {
        ItemStack* stack = itemRendererInstance->m_pItemToRender;

        if (stack->isValid()) {
            uint32_t item_id = stack->getId();
            uint32_t metadata = stack->getDamage();

            // 2. Resolución O(1) inmediata en tabla XDR
            const struct RegisteredVoxelItem* voxel_item = item_registry_find(item_id, metadata);

            if (voxel_item && g_gcm_context) {
                // 3. Sondeo de controles DualShock 3
                struct ControllerInputState pad;
                input_manager_poll(&pad);
                g_player_weapon_state.l2_aim_pressed = pad.l2_aim_pressed;

                // Detección de disparo y retroceso
                if (pad.r1_fire_just_pressed) {
                    kinematics_apply_recoil(&g_player_weapon_state, voxel_item);
                    muzzle_flash_trigger(nullptr, 0.15f);
                }

                // 4. Actualizar cinemáticas (SmoothStep ADS y amortiguación de retroceso)
                kinematics_update(&g_player_weapon_state, voxel_item, 1.0f / 60.0f);
                muzzle_flash_update();

                // 5. Construir matriz combinada para el RSX
                float mvp_matrix[16];
                kinematics_build_matrix(&g_player_weapon_state, voxel_item, mvp_matrix);

                // 6. Emitir Draw Call RSX de bajo nivel para la malla vóxel
                voxel_model_drawn = rsx_draw_voxel_model(g_gcm_context, voxel_item->model_id, mvp_matrix);

                // 7. Renderizar destello de disparo en la boca del cañón si está activo
                if (voxel_model_drawn && muzzle_flash_is_active()) {
                    muzzle_flash_render(g_gcm_context, mvp_matrix);
                }
            }
        }
    }

    // 8. Fallback limpio al renderizador 2D original si no es un modelo vóxel 3D
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

        // 1. Inicializar subsistemas
        item_registry_init();
        kinematics_init(&g_player_weapon_state);
        input_manager_init();
        muzzle_flash_init();

        // 2. Inicializar el pool estático de 4 MB en memoria GDDR3 del RSX
        if (!rsx_init_vram_pool()) {
            netlog_error("Fallo crítico al reservar pool de VRAM para RSX.");
            return -1;
        }

        // 3. Cargar configuración data-driven y mallas .vx3
        config_loader_load_all(nullptr);

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
