#ifndef VOXEL3D_H
#define VOXEL3D_H

#include <stdint.h>

// Definiciones de cabecera y estructuras para Voxel3D-PS3
#pragma pack(push, 1)

struct VX3Header {
    char magic[4];          // "VX3D"
    uint16_t version;       // 1
    uint16_t flags;         // 0
    uint32_t vertex_count;  // Número de vértices
    uint32_t index_count;   // Número de índices
};

struct VoxelVertex {
    int16_t x, y, z;
    uint8_t nx, ny, nz;
    uint8_t flags;
    uint16_t u, v;
};

#pragma pack(pop)

#endif // VOXEL3D_H
