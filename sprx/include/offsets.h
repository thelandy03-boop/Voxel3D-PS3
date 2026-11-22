#ifndef OFFSETS_H
#define OFFSETS_H

#include <stdint.h>

/**
 * ==============================================================================
 * Offsets Oficiales de Minecraft: PlayStation 3 Edition (BLES01976)
 * Obtenidos mediante análisis estático y dinámico con Ghidra / REA / RPCS3.
 *
 * PPU Hash Big-Endian:    a51d222f3658b515f567d7d63d6b560c7f7e2972
 * PPU Hash Little-Endian: d29a1ced1051a3dd6ff04f6ce2b33a322ab8db94
 * ==============================================================================
 */

// Dirección de intercepción en caliente: ItemRenderer::renderItemInFirstPerson
#define VOX3_HOOK_RENDER_ITEM_FIRST_PERSON 0x005DA890

// Puntero global al CellGcmContextData activo del motor gráfico de 4J Studios.
// Extraído de: lwz r3, -12164(r2) en wrapper de cellGcmInit (0x005D27D0) con r2 = 0x00160000
// Cálculo: 0x00160000 - 0x2F7C = 0x0015D084
#define VOX3_GCM_CONTEXT_GLOBAL            0x0015D084

#endif // OFFSETS_H
