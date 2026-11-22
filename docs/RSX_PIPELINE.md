# Pipeline y Gestión de Estado del RSX (Reality Synthesizer)

Este documento especifica la integración de bajo nivel entre el plugin nativo `Voxel3D-PS3` y la GPU RSX (NVIDIA G70 / NV47 personalizada) utilizando la API `libgcm`.

---

## 1. Arquitectura de Hardware y Restricciones del RSX

La GPU RSX opera como un procesador de comandos asíncrono alimentado por la PPU a través de un anillo FIFO (*Command Buffer*).

* **Unidades de Procesamiento:** 8 Vertex Pipelines independientes y 24 Pixel/Fragment Pipelines vectoriales.
* **Frecuencias de Operación:** Núcleo a 500 MHz, VRAM a 650 MHz (bus de 128-bit GDDR3 con ancho de banda teórico de 20.8 GB/s).
* **Interconexión FlexIO:** Conexión de 12 canales hacia el Cell (XDR RAM) a 20 GB/s de lectura PPU y 15 GB/s de escritura PPU, pero con latencia de acceso significativamente superior a la memoria GDDR3 local.
* **Restricción Crítica:** Todo comando gráfico debe emitirse dentro del buffer de comandos activo sin fragmentar el pipeline ni desincronizar los semáforos de flip del motor de 4J Studios.

---

## 2. Inyección en el Command Buffer de libgcm

El motor de Minecraft PS3 mantiene una instancia global del contexto GCM (`CellGcmContextData`). El plugin debe adquirir el contexto de ejecución actual sin recrear un contexto nuevo, para no desalinear el puntero de escritura (`current`) del buffer FIFO.

```
       [ PPU: ItemRenderer Hook ]
                   │
                   ▼ (Obtención del CellGcmContextData activo)
   ┌───────────────────────────────┐
   │ cellGcmSetVertexProgram(...)  │ ──► Configura Vertex Shader
   │ cellGcmSetFragmentProgram(...)│ ──► Configura Fragment Shader
   │ cellGcmSetVertexDataArray(...)│ ──► Enlaza VBO en VRAM local
   │ cellGcmSetDrawIndexArray(...) │ ──► Emite 1 Draw Call indexado
   └───────────────────────────────┘
                   │ (Escritura en Command Buffer)
                   ▼
       [ RSX FIFO Command Ring ] ────► [ RSX Rasterizer & Output ]
```

### Cálculo de Offsets para el RSX
El RSX no procesa direcciones virtuales efectivas (EA) de 32 o 64 bits de la PPU. Toda dirección en memoria local o mapeada debe convertirse a un offset relativo del bus RSX:

```cpp
uint32_t rsx_vram_offset = 0;
int32_t ret = cellGcmAddressToOffset(pLocalVboAddress, &rsx_vram_offset);
// rsx_vram_offset se utiliza en cellGcmSetVertexDataArray
```

---

## 3. Layout de Atributos de Vértices (Vertex Array Binding)

Para la estructura `VoxelVertex` (12 bytes) definida en la especificación `.vx3`, los atributos de hardware se configuran de la siguiente manera:

| Atributo Hardware | Semántica RSX | Tipo GCM | Componentes | Stride | Offset Relativo |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **ATTR 0** | Posición ($x, y, z$) | `CELL_GCM_VERTEX_S16` | 3 | 12 bytes | `0` |
| **ATTR 1** | Normal empaquetada | `CELL_GCM_VERTEX_UB` (Norm) | 3 | 12 bytes | `6` |
| **ATTR 2** | Flags / AO | `CELL_GCM_VERTEX_UB` | 1 | 12 bytes | `9` |
| **ATTR 3** | UV / Paleta ($u, v$) | `CELL_GCM_VERTEX_U16` | 2 | 12 bytes | `10` |

### Enlace de Atributos en GCM:
```cpp
// Atributo 0: Posición (S16 normalizada o escalada en el shader)
cellGcmSetVertexDataArray(gContext, 0, 0, 12, 3, CELL_GCM_VERTEX_S16,
                          CELL_GCM_LOCATION_LOCAL, rsx_vram_offset + 0);

// Atributo 1: Normal (3 componentes uint8 normalizados en [-1.0, 1.0])
cellGcmSetVertexDataArray(gContext, 1, 0, 12, 3, CELL_GCM_VERTEX_UB,
                          CELL_GCM_LOCATION_LOCAL, rsx_vram_offset + 6);

// Atributo 2: Flags (AO y datos de cara)
cellGcmSetVertexDataArray(gContext, 2, 0, 12, 1, CELL_GCM_VERTEX_UB,
                          CELL_GCM_LOCATION_LOCAL, rsx_vram_offset + 9);

// Atributo 3: UVs o índice de paleta
cellGcmSetVertexDataArray(gContext, 3, 0, 12, 2, CELL_GCM_VERTEX_U16,
                          CELL_GCM_LOCATION_LOCAL, rsx_vram_offset + 10);
```

---

## 4. Estrategia de Shaders (Vertex & Fragment Programs)

Existen dos vías de ejecución técnica:

### Vía A: Reutilización de Shaders Nativos de 4J Studios (Recomendada inicialmente)
4J Studios compila internamente micro-shaders en lenguaje Cg (`sce_vp_rsx` y `sce_fp_rsx`). Al interceptar `renderItemInFirstPerson`, el motor ya tiene enlazado el programa de shader responsable de pintar quads y texturas del atlas.
* **Ventajas:** No requiere cargar programas de sombreado adicionales en VRAM ni alterar los samplers de textura enlazados (`terrain.png` / `items.png`).
* **Requisito:** Ajustar la escala de posición local en el cálculo de la matriz de transformación para acomodar el formato `int16_t`.

### Vía B: Shaders Dedicados Voxel3D
Cargar micro-programas Cg precompilados optimizados para vóxeles con sombreado de oclusión ambiental por vértice (*baked AO*):
* **Vertex Program Uniforms:**
  * Constante `C0 - C3`: Matriz MVP combinada (`float4x4`).
  * Constante `C4`: Vector de escala de modelo y desplazamiento ($sx, sy, sz, w$).
  * Constante `C5`: Intensidad lumínica del ambiente del bloque actual ($lightmap_{sky}, lightmap_{block}$).
* **Fragment Program:**
  * Muestreo de textura o paleta RGB565 mediante `cellGcmSetTexture()`.
  * Cálculo de sombreado Lambertiano simple en hardware.

---

## 5. Preservación y Restauración del Estado del Rasterizador

Para evitar artefactos visuales en el renderizado posterior (mano del jugador, HUD, chunks adyacentes), el plugin debe adherirse al principio de **Cero Fuga de Estado**:

1. **Estado que se debe configurar explícitamente para el Vóxel 3D:**
   ```cpp
   // Habilitar Depth Test y Depth Write
   cellGcmSetDepthTestEnable(gContext, CELL_GCM_TRUE);
   cellGcmSetDepthFunc(gContext, CELL_GCM_LEQUAL);
   cellGcmSetDepthMask(gContext, CELL_GCM_TRUE);

   // Activar Culling de caras traseras (CCW estándar)
   cellGcmSetCullFaceEnable(gContext, CELL_GCM_TRUE);
   cellGcmSetCullFace(gContext, CELL_GCM_BACK);

   // Configurar Alpha Test (descartar píxeles transparentes de textura)
   cellGcmSetAlphaTestEnable(gContext, CELL_GCM_TRUE);
   cellGcmSetAlphaFunc(gContext, CELL_GCM_GREATER, 0x1A); // Alpha > 0.1
   ```

2. **Restauración al Salir del Hook:**
   * Desactivar o restaurar el estado de los canales de atributos vinculados (`cellGcmSetVertexDataArray` en atributos 1, 2 y 3) para no colapsar el renderizado del brazo (`RenderPlayer::renderFirstPersonArm`).
   * No invalidar buffers de textura en uso por la GUI.

---

## 6. Pipeline de Transformación de Matriz (MVP & Recoil)

La posición del arma en pantalla se rige por la matriz combinada:

$$M_{final} = M_{proj} \times M_{view} \times M_{arm} \times M_{recoil} \times M_{ads}$$

Donde:
* $M_{arm}$: Matriz del hueso de la mano derecha del modelo del jugador.
* $M_{recoil}$: Matriz de rotación/traslación generada por el algoritmo de retroceso.
* $M_{ads}$: Matriz de interpolación hacia el eje central de visión del jugador.

La matriz resultante de 64 bytes (`4x4 float`) se transfiere al RSX mediante:
```cpp
cellGcmSetVertexProgramConstants(gContext, 0, 16, (float*)&mFinalMatrix);
```
Esto asegura que la transformación completa sea calculada en 1 ciclo de vértice por la unidad vectorial del RSX sin sobrecargar la PPU.
