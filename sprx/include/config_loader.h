#ifndef CONFIG_LOADER_H
#define CONFIG_LOADER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Carga el archivo de configuración data-driven JSON desde el disco duro de la PS3,
 * carga todas las mallas .vx3 asociadas hacia el pool de VRAM GDDR3 del RSX,
 * y registra los metadatos y cinemáticas en la tabla O(1) de memoria XDR.
 *
 * @param config_path Ruta al archivo config.json (o nullptr para usar ruta por defecto)
 * @return Número de armas/modelos cargados exitosamente hacia VRAM.
 */
int config_loader_load_all(const char* config_path);

#ifdef __cplusplus
}
#endif

#endif // CONFIG_LOADER_H
