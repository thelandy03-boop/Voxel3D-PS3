# Especificación del Formato Binario VX3 (.vx3)

El formato `.vx3` es una estructura binaria contigua y compacta diseñada específicamente para ser consumida directamente por la GPU RSX sin conversiones intermedias en tiempo de ejecución.

---

## 1. Cabecera (Header - 16 bytes)

| Offset | Tipo     | Nombre         | Descripción                                     |
|--------|----------|----------------|-------------------------------------------------|
| 0x00   | char[4]  | `magic`        | Identificador: `"VX3D"`                         |
| 0x04   | uint16_t | `version`      | Versión del formato (actualmente `1`)           |
| 0x06   | uint16_t | `flags`        | Flags de compresión / normales                  |
| 0x08   | uint32_t | `vertex_count` | Cantidad total de vértices                      |
| 0x0C   | uint32_t | `index_count`  | Cantidad total de índices (múltiplo de 3)       |

---

## 2. Layout de Vértice (Vertex Structure - 12 bytes por vértice)

Para minimizar el uso de ancho de banda y memoria GDDR3:

```cpp
struct VoxelVertex {
    int16_t x, y, z;      // Posición local normalizada (half-short / Q8.8 fixed point)
    uint8_t nx, ny, nz;   // Normal empaquetada (orientación de la cara del cubo: ±X, ±Y, ±Z)
    uint8_t flags;        // Flags de oclusión ambiental / sombreado vóxel
    uint16_t u, v;        // Coordenadas UV o índice de paleta de color (RGB565 / índice)
};
```

---

## 3. Optimización de Malla Offline

1. **Greedy Meshing & Face Culling:**
   * Las caras internas adyacentes entre vóxeles opacos se descartan por completo en la herramienta de PC.
   * Se fusionan caras coplanares contiguas con el mismo color o textura para reducir el recuento de triángulos.
2. **Presupuesto por Modelo:**
   * Un modelo de arma complejo (fusil/pistola vóxel) se mantiene entre 800 y 2500 triángulos.
   * Tamaño final en disco y VRAM: **15 KB - 30 KB** por arma.
