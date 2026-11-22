#ifndef VOXEL3D_OFFSETS_H
#define VOXEL3D_OFFSETS_H

// Offsets verificados para Minecraft PS3 EBOOT.elf (Europe) v1.0.8
// SHA256: 81bbf4d78af4aeb07394e978fd2fed5f89264f57fb7ba4ca4a9e64500a240d5c
// Mapping: VA = file offset + 0x00010000
#define VOX3_TOC_BASE               0x00160000u
#define VOX3_RENDER_ITEM_FIRST_PERSON 0x005DA890u
#define VOX3_RENDER_ITEM_RET        0x005DA8B0u

// El juego guarda el puntero CellGcmContextData* en una global accedida como
// `lwz r3,-12164(r2)` desde el wrapper de cellGcmInit (0x005D27D0).
// r2 = VOX3_TOC_BASE (0x00160000), por lo que la direccion es 0x0015D084.
// RPCS3 aplica las relocaciones del EBOOT antes de cargar el PRX, asi que el
// valor en esta direccion es el contexto real del juego en tiempo de ejecucion.
#define VOX3_GCM_CONTEXT_GLOBAL     0x0015D084u

// Constantes verificadas por scan_const_refs.py
#define VOX3_CONST_NEG_040          0x0015D1E8u  // -0.40f
#define VOX3_CONST_NEG_072          0x0015D1F0u  // -0.72f
#define VOX3_CONST_056              0x0015D24Cu  //  0.56f
#define VOX3_CONST_NEG_052          0x0015D250u  // -0.52f
#define VOX3_CONST_060              0x0015D254u  // 60.0f

// Estructura interna (Voxel3D-PS3 CONFIG_SPEC.md)
#define VOX3_ENTITY_OFFSET          0x0018u
#define VOX3_OFF_ANIMTOGO           0x0010u

#endif /* VOXEL3D_OFFSETS_H */
