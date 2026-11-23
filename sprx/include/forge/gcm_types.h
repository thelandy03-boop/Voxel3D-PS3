#ifndef GCM_TYPES_H
#define GCM_TYPES_H

#include <stdint.h>
#include <stddef.h>

#if defined(__CELLOS_LV2__) && !defined(VOXEL3D_HOST_BUILD)
#include <cell/gcm.h>
#else

// Ubicaciones de memoria para GPU RSX
#define CELL_GCM_LOCATION_LOCAL 0
#define CELL_GCM_LOCATION_MAIN  1

// Constantes booleanas GCM
#define CELL_GCM_FALSE 0
#define CELL_GCM_TRUE  1

// Formatos de atributo de vértice RSX
#define CELL_GCM_VERTEX_S16    1
#define CELL_GCM_VERTEX_F      2
#define CELL_GCM_VERTEX_UB     4
#define CELL_GCM_VERTEX_U16    5

// Primitivas de dibujo
#define CELL_GCM_PRIMITIVE_POINTS         1
#define CELL_GCM_PRIMITIVE_LINES          2
#define CELL_GCM_PRIMITIVE_LINE_STRIP     3
#define CELL_GCM_PRIMITIVE_TRIANGLES      4
#define CELL_GCM_PRIMITIVE_TRIANGLE_STRIP 5
#define CELL_GCM_PRIMITIVE_TRIANGLE_FAN   6

// Tipos de formato de índice
#define CELL_GCM_DRAW_INDEX_ARRAY_TYPE_32 0
#define CELL_GCM_DRAW_INDEX_ARRAY_TYPE_16 1

// Modos de comparación para Depth y Alpha Test
#define CELL_GCM_NEVER    0x0200
#define CELL_GCM_LESS     0x0201
#define CELL_GCM_EQUAL    0x0202
#define CELL_GCM_LEQUAL   0x0203
#define CELL_GCM_GREATER  0x0204
#define CELL_GCM_NOTEQUAL 0x0205
#define CELL_GCM_GEQUAL   0x0206
#define CELL_GCM_ALWAYS   0x0207

// Modos de Culling
#define CELL_GCM_FRONT 0x0404
#define CELL_GCM_BACK  0x0405

// Estructura del anillo de comandos FIFO (CellGcmContextData)
typedef struct CellGcmContextData {
    uint32_t* current;
    uint32_t* end;
    uint32_t* begin;
    uint32_t  callback;
} CellGcmContextData;

// Configuración de memoria y frecuencias del RSX
typedef struct CellGcmConfig {
    void*    localAddress;
    uint32_t ioAddress;
    uint32_t localSize;
    uint32_t ioSize;
    uint32_t memoryFrequency;
    uint32_t coreFrequency;
} CellGcmConfig;

#endif // __CELLOS_LV2__

#endif // GCM_TYPES_H
