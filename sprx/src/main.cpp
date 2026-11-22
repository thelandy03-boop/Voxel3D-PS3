/**
 * Voxel3D-PS3: Plugin SPRX para Minecraft PlayStation 3 Edition
 * Módulo de intercepción PPU, telemetría y enrutamiento hacia el pipeline de renderizado RSX.
 */

#include "voxel3d.h"
#include "hook.h"
#include "netlogger.h"
#include "rsx_pipeline.h"
#include "crayfish_transforms.h"
#include <stdint.h>

// Contexto global del hook de ItemRenderer
static HookContext g_item_renderer_hook;

// Offset base tentativo para Minecraft PS3 Update 1.84 (BLES01976 / NPUB31419)
// En producción se obtiene por escaneo de patrones AOB en la sección .text
#define MINECRAFT_RENDER_ITEM_ADDR 0x0045A120

// Prototipo de la función original ItemRenderer::renderItemInFirstPerson
typedef void (*t_renderItemInFirstPerson)(void* itemRendererInstance, float partialTicks);

// Contador estático de llamadas para telemetría
static uint32_t s_render_call_count = 0;

/**
 * Función Detour interpuesta en caliente sobre ItemRenderer::renderItemInFirstPerson.
 * r3 = itemRendererInstance (puntero 'this' en C++)
 * f1 = partialTicks (tiempo parcial para interpolación de frames)
 */
void hook_renderItemInFirstPerson(void* itemRendererInstance, float partialTicks) {
    if ((++s_render_call_count % 300) == 1) {
        netlog_debug("renderItemInFirstPerson invocado (tick=%.3f, calls=%u)", 
                     (double)partialTicks, s_render_call_count);
    }

    // 1. Obtener la función original desde el trampolín generado
    t_renderItemInFirstPerson originalFunc = 
        (t_renderItemInFirstPerson)(void*)g_item_renderer_hook.trampoline_buffer;

    // 2. Comprobar si existe un modelo 3D registrado para el slot actual
    // Si g_gcm_context está disponible y el arma vóxel se dibuja con éxito:
    bool drawn = false;
    if (g_gcm_context) {
        // ID de prueba: 1 (ej. Pistol)
        // Si el modelo está cargado en VRAM, emitir Draw Call RSX
        drawn = rsx_draw_voxel_model(g_gcm_context, 1, nullptr);
    }

    // 3. Si no se renderizó modelo vóxel (ítem estándar o fallback seguro):
    if (!drawn && originalFunc) {
        originalFunc(itemRendererInstance, partialTicks);
    }
}

// Declaración de puntos de entrada del módulo PRX / SPRX de Cell OS
extern "C" {
    int voxel3d_prx_start(void) {
        // Inicializar telemetría UDP (IP por defecto para pruebas en RPCS3 o red local)
        netlog_init("127.0.0.1", 3000);
        netlog_info("Inicializando Voxel3D-PS3 SPRX plugin...");

        // Inicializar el pool estático de 4 MB en VRAM GDDR3 del RSX
        if (!rsx_init_vram_pool()) {
            netlog_error("Fallo al reservar pool de VRAM para RSX.");
            return -1;
        }

        // Inicializar el hook PPU en ItemRenderer
        bool success = install_ppu_hook(
            &g_item_renderer_hook,
            MINECRAFT_RENDER_ITEM_ADDR,
            (void*)hook_renderItemInFirstPerson
        );

        if (success) {
            netlog_info("PPU Hook instalado exitosamente en 0x%08X (trampoline listo)", MINECRAFT_RENDER_ITEM_ADDR);
        } else {
            netlog_error("Fallo crítico al instalar PPU Hook en 0x%08X", MINECRAFT_RENDER_ITEM_ADDR);
        }

        return success ? 0 : -1;
    }

    int voxel3d_prx_stop(void) {
        netlog_info("Deteniendo Voxel3D-PS3 SPRX plugin...");

        // Desinstalar hooks y restaurar bytes originales de forma segura
        if (g_item_renderer_hook.installed) {
            uninstall_ppu_hook(&g_item_renderer_hook);
            netlog_info("PPU Hook desinstalado. Memoria original restaurada.");
        }

        // Liberar el pool de memoria VRAM RSX
        rsx_free_vram_pool();

        netlog_info("Voxel3D-PS3 finalizado.");
        netlog_close();
        return 0;
    }
}
