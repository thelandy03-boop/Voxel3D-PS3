#ifndef VOXEL3D_VX3_FORMAT_H
#define VOXEL3D_VX3_FORMAT_H

#include <stddef.h>
#include <stdint.h>

// ---------------------------------------------------------------------------
// VX3 v1 - Big-Endian nativo en PPU (PS3). Sin byteswap en tiempo de carga.
// Coincide exactamente con el struct '<' de Python en tools/obj_to_vx3.py:
//   struct.pack(">3h3b3B", ...)
// ---------------------------------------------------------------------------
#pragma pack(push, 1)

typedef struct {
    char     magic[4];       // "VX3D"
    uint16_t version;        // 1
    uint16_t flags;          // 0
    uint32_t vertex_count;   // 1742 para M4A1
    uint32_t index_count;    // 3492 para M4A1
} VX3Header;

typedef struct {
    int16_t  x, y, z;        // posicion Q8.8 (coord * 256)
    int8_t   nx, ny, nz;     // normal empaquetada (-128..127)
    uint8_t  material;       // indice de material (0..2)
    uint8_t  u, v;           // UV cuantizado 0..255
} VX3Vertex;

#pragma pack(pop)

#ifdef __cplusplus
static_assert(sizeof(VX3Header)  == 16, "VX3Header debe ser 16 bytes");
static_assert(sizeof(VX3Vertex)  == 12, "VX3Vertex debe ser 12 bytes");
#endif

#define VX3_HEADER_SIZE  16u
#define VX3_STRIDE       12u

#endif /* VOXEL3D_VX3_FORMAT_H */
