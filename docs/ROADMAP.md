# Hoja de Ruta (Roadmap) de Desarrollo: Voxel3D-PS3

Este documento define las etapas de desarrollo, hitos de ingeniería (*milestones*), criterios de aceptación y matriz de mitigación de riesgos para llevar el proyecto desde la fase de investigación hasta una versión estable en producción.

---

## Resumen de Hitos (Milestone Timeline)

```
[ Fase 0: Investigación & Reversing ] ────► [ Fase 1: Pipeline de Conversión (.vx3) ]
                   │                                          │
                   ▼                                          ▼
[ Fase 2: Hook PPU & Scaffolding ]    ────► [ Fase 3: Renderizado RSX (libgcm) ]
                   │                                          │
                   ▼                                          ▼
[ Fase 4: Físicas, ADS & Retroceso ]  ────► [ Fase 5: Estabilización & Release 1.0 ]
```

---

## Fase 0: Ingeniería Inversa y Mapeo de Memoria (Mes 1)
**Objetivo:** Obtener los puntos de intercepción exactos y validar las estructuras del motor en el ejecutable desprotegido.

* [ ] **0.1. Preparación de Binarios:**
  * Descifrado de `EBOOT.BIN` para versiones BLES01976 y NPUB31419 (actualización 1.84).
  * Extracción del `EBOOT.ELF` y descompresión de secciones de código.
* [ ] **0.2. Análisis Estático en Ghidra / IDA Pro:**
  * Localización de la función virtual `ItemRenderer::renderItemInFirstPerson`.
  * Identificación de la rutina que genera quads extruidos a partir de sprites de ítems.
  * Extracción de las referencias a matrices del brazo (`renderFirstPersonArm`).
* [ ] **0.3. Definición de Signaturas AOB:**
  * Creación de patrones de bytes (*Array of Bytes*) para que el hook localice las funciones dinámicamente sin depender únicamente de offsets fijos.

**Criterio de Aceptación:** Documentar en [`ENGINE_STRUCTURES.md`](file:///home/landy/Proyectos/Voxel3D-PS3/docs/ENGINE_STRUCTURES.md) las direcciones de memoria verificadas de `ItemRenderer` y validar que un breakpoint en RPCS3 se active exactamente al cambiar de ítem en la mano.

---

## Fase 1: Pipeline Offline de Conversión de Mallas (Mes 2)
**Objetivo:** Desarrollar la herramienta CLI para exportar modelos vóxel optimizados al formato binario `.vx3`.

* [ ] **1.1. Parser de Formatos Fuente:**
  * Lector de archivos MagicaVoxel (`.vox`).
  * Lector de modelos JSON / Blockbench (`.bbmodel`).
* [ ] **1.2. Algoritmos de Optimización de Malla:**
  * Implementación de *Occlusion Culling* (descartar caras internas de cubos coplanares y adyacentes).
  * Implementación de *Greedy Meshing* (fusión de rectángulos continuos de igual color/textura para reducir drásticamente el recuento de triángulos).
* [ ] **1.3. Compresor y Empaquetador Binario:**
  * Cuantización de coordenadas a formato `int16_t` y empaquetado de normales/flags en 12 bytes por vértice (`VoxelVertex`).
  * Generación de la cabecera estándar `VX3D` y cálculo de bounding box / offsets de alineación (128 bytes).

**Criterio de Aceptación:** Convertir un modelo complejo de rifle vóxel (ej. AK-47) desde `.vox` a `.vx3` con menos de 2,000 triángulos, un tamaño inferior a 30 KB y validar su integridad estructural con un script de desempaquetado de prueba.

---

## Fase 2: Scaffolding del Plugin SPRX y Hook Seguro (Mes 3)
**Objetivo:** Crear el esqueleto funcional del módulo `.sprx` capaz de inyectar trampolines en la PPU sin alterar la estabilidad del juego.

* [ ] **2.1. Toolchain y Build System:**
  * Configuración del `Makefile` bajo `PSL1GHT v2` y generación limpia del archivo `Voxel3D-PS3.sprx`.
  * Implementación de funciones de ciclo de vida del módulo (`sys_prx_start` y `sys_prx_stop`).
* [ ] **2.2. Motor de Trampolines PowerPC:**
  * Implementación de la secuencia de salto absoluto de 16 bytes (`lis/ori/mtctr/bctr`).
  * Inyección en memoria de código mediante syscalls de depuración (`sys_dbg_write_process_memory`).
  * Rutina obligatoria de invalidación y sincronización de cachés (`dcbst`, `sync`, `icbi`, `isync`).
* [ ] **2.3. Sistema de Telemetría UDP:**
  * Integración del cliente de logging por sockets UDP para transmitir trazas de ejecución en vivo hacia la PC de desarrollo.

**Criterio de Aceptación:** Cargar el `.sprx` en RPCS3 y consola física, interceptar `renderItemInFirstPerson`, registrar el ID del ítem sostenido en el log UDP y retornar limpiamente mediante el trampolín sin congelar el juego.

---

## Fase 3: Pipeline de Renderizado RSX de Bajo Nivel (Mes 4)
**Objetivo:** Reservar memoria VRAM y dibujar la primera malla vóxel 3D en pantalla sustituyendo el sprite original.

* [ ] **3.1. Alocador Estático de VRAM:**
  * Reserva del pool de 4 MB en memoria local GDDR3 (`CELL_GCM_LOCATION_LOCAL`).
  * Carga del archivo de prueba `.vx3` desde `/dev_hdd0/` directamente al pool.
  * Conversión de punteros a offsets del RSX con `cellGcmAddressToOffset()`.
* [ ] **3.2. Configuración del Rasterizador:**
  * Configuración de estados GCM: `cellGcmSetDepthTestEnable`, `cellGcmSetCullFaceEnable`, `cellGcmSetAlphaTestEnable`.
  * Vinculación de los 4 canales de atributos del vértice con `cellGcmSetVertexDataArray()`.
* [ ] **3.3. Emisión de Dibujo Indexado:**
  * Supresión de la llamada de renderizado 2D vanilla cuando el ítem sostenido coincide con el ID registrado.
  * Envío del comando de dibujo `cellGcmSetDrawIndexArray()`.
  * Preservación del estado gráfico del motor para no romper el renderizado del brazo ni del HUD.

**Criterio de Aceptación:** Renderizar en pantalla un ítem 3D vóxel en primera persona en lugar del sprite 2D de Minecraft, manteniendo 60 FPS estables y sin artefactos en la interfaz.

---

## Fase 4: Cinemática, Apuntado (ADS) y Retroceso LERP (Mes 5)
**Objetivo:** Integrar el comportamiento dinámico de armas estilo *MrCrayfish's Gun Mod*.

* [ ] **4.1. Transformaciones de Cámara y Brazo:**
  * Multiplicación y enlace de matrices: Matriz del modelo $\times$ Matriz de la mano del jugador $\times$ Proyección.
* [ ] **4.2. Sistema de Apuntado ADS (Aim Down Sights):**
  * Detección del botón L2 (gatillo izquierdo del DualShock 3 / mando).
  * Interpolación suave de traslación hacia el centro de la pantalla $(X=0)$ y ajuste dinámico del FOV de la cámara.
* [ ] **4.3. Algoritmo de Retroceso y Recuperación:**
  * Aplicación de impulso instantáneo (*kick impulse*) en posición y ángulo al disparar.
  * Recuperación suave mediante decaimiento exponencial ($P_{target} + (P - P_{target}) \cdot e^{-\lambda \Delta t}$).
* [ ] **4.4. Efectos Visuales (Muzzle Flash):**
  * Renderizado del destello de disparo en las coordenadas definidas del cañón.

**Criterio de Aceptación:** Transición fluida de reposo a apuntado ADS en menos de 200 ms y recuperación realista de retroceso tras disparar ráfagas continuas.

---

## Fase 5: Sistema Data-Driven y Estabilización v1.0 (Mes 6)
**Objetivo:** Eliminar cualquier parámetro fijo del código fuente y preparar el paquete final para usuarios finales.

* [ ] **5.1. Parser Ligero de Configuración JSON:**
  * Carga y parseo seguro de `/dev_hdd0/game/<TITLE_ID>/USRDIR/voxel3d/config.json`.
  * Indexación en memoria XDR para mapeo instantáneo $O(1)$ de ítems y modelos.
* [ ] **5.2. Pruebas de Estrés y Memory Leak Profiling:**
  * Validación de estabilidad en sesiones de juego continuas de más de 2 horas.
  * Cambio rápido de slots en la hotbar para verificar ausencia de fugas de memoria en VRAM.
* [ ] **5.3. Compatibilidad Multi-Región:**
  * Verificación y soporte de TitleIDs: `BLES01976` (Disco EU), `NPUB31419` (Digital US), `NPEB01899` (Digital EU), `BLUS31426` (Disco US).
* [ ] **5.4. Documentación y Release Público:**
  * Guía de instalación para el usuario final (archivos `.pkg` o copia manual por FTP/webMAN).
  * Pack inicial de 5 armas de prueba (Pistola, Escopeta, Subfusil, Fusil de asalto, Francotirador).

**Criterio de Aceptación:** Publicación de la versión **v1.0.0** con soporte completo para las versiones principales de Minecraft PS3, con instalador y modelos de muestra funcionales.

---

## Matriz de Riesgos y Mitigación

| Riesgo Técnico | Severidad | Probabilidad | Estrategia de Mitigación |
| :--- | :--- | :--- | :--- |
| **Fragmentación / OOM en VRAM del RSX** | Alta | Media | Reservar un pool estático cerrado de 4 MB al inicio; nunca alocar memoria dinámicamente en caliente por cada frame. |
| **Inestabilidad por Caché de Instrucciones PPU** | Crítica | Media | Implementar secuencia estricta de barrera de sincronización (`dcbst` + `icbi` + `isync`) en cada modificación de memoria. |
| **Corrupción de Estado Gráfico RSX** | Alta | Alta | Guardar y restaurar explícitamente los estados de Depth, Alpha y Culling al entrar y salir del hook. |
| **Divergencia de Offsets entre Actualizaciones** | Media | Alta | Utilizar escaneo de signaturas por patrones de bytes (AOB) en lugar de direcciones codificadas en duro. |
| **Congelamiento de la Consola al Desmontar Plugin** | Alta | Baja | Sincronizar el pipeline RSX con `cellGcmFinish()` antes de desvincular los trampolines y liberar el pool. |

---

## Exploraciones Futuras (Post-1.0)
* Renderizado de armas vóxel en tercera persona (vistas desde otros jugadores en pantalla dividida o multijugador).
* Descarga de compresión y descompresión de mallas al procesador vectorial SPU (Synergistic Processing Unit).
* Inyección de bancos de audio personalizados para sonidos de disparos y recarga.
