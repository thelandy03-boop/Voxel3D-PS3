# Hoja de Ruta (Roadmap) de Desarrollo: Voxel3D-PS3

Este documento define las etapas de desarrollo, hitos de ingeniería (*milestones*), criterios de aceptación y matriz de mitigación de riesgos para llevar el proyecto desde la fase de investigación hasta una versión estable en producción.

> [!NOTE]
> **Instrucciones para Desarrolladores y Agentes de IA:**
> El estado y los gráficos de progreso de este proyecto son **100% dinámicos y reales**. Se extraen automáticamente de este archivo.
> Para modificar un porcentaje o estado de un componente:
> 1. Cambia la casilla correspondiente en este archivo:
>    * `* [x]` : Tarea completada y verificada (**verde** / `done`).
>    * `* [~]` : Tarea iniciada / en progreso activo (**amarillo** / `started`).
>    * `* [ ]` : Tarea pendiente (**gris** / `to do`).
> 2. Ejecuta el comando: `python3 tools/generate_status_svg.py`
> 3. El script regenera `docs/status.svg` y recalcula los porcentajes exactos de manera automática.

---

## 1. Especificaciones Técnicas y Arquitectura

### 1.1. Core Engine & Arquitectura
* [x] **Arquitectura General:** Ciclo de renderizado, intercepción PPU y pipeline RSX en [`ARCHITECTURE.md`](ARCHITECTURE.md).
* [x] **Estructuras C++ 4J Studios:** Mapeo de `ItemStack`, `InventoryPlayer` y matrices en [`ENGINE_STRUCTURES.md`](ENGINE_STRUCTURES.md).
* [x] **Metodología de Reversing:** Guía para desensamblado con Ghidra/IDA en [`REVERSE_ENGINEERING.md`](REVERSE_ENGINEERING.md).
* [x] **Preguntas Frecuentes:** Respuestas técnicas sobre diseño y compatibilidad en [`FAQ.md`](FAQ.md).

### 1.2. GPU RSX, Memoria & Assets
* [x] **Pipeline Gráfico RSX:** Gestión de contexto FIFO, vertex arrays y shaders en [`RSX_PIPELINE.md`](RSX_PIPELINE.md).
* [x] **Presupuesto VRAM/XDR:** Alocación estática en GDDR3 local y memoria mapeada en [`MEMORY_MANAGEMENT.md`](MEMORY_MANAGEMENT.md).
* [x] **Formato Binario VX3:** Especificación de cabecera y vértices compactos en [`VX3_FORMAT.md`](VX3_FORMAT.md).
* [x] **Pipeline de Assets 3D:** Guía de modelado para Blockbench y MagicaVoxel en [`ASSET_PIPELINE.md`](ASSET_PIPELINE.md).

### 1.3. Hooking, Toolchains & Entornos
* [x] **Ingeniería de Hooking PPU:** Trampolines absolutos de 16 bytes y coherencia de caché en [`PPU_HOOKING.md`](PPU_HOOKING.md).
* [x] **Esquema Data-Driven:** Formato JSON para armas, retroceso LERP y ADS en [`CONFIG_SPEC.md`](CONFIG_SPEC.md).
* [x] **Toolchain & Telemetría:** Entorno PSL1GHT, depuración en RPCS3 y NetLogger en [`DEBUG_AND_TOOLCHAIN.md`](DEBUG_AND_TOOLCHAIN.md).
* [x] **Compilación en Linux:** Guía de compilación con ps3dev en [`COMPILATION_GUIDE.md`](COMPILATION_GUIDE.md).
* [x] **Compilación en Windows:** Configuración con WSL2, MSYS2 y RPCS3 en [`WINDOWS_SETUP.md`](WINDOWS_SETUP.md).

---

## 2. Hitos de Implementación (Código / Binarios / Reversing)

### Fase 0: Ingeniería Inversa y Mapeo de Memoria
* [x] **0.1. Preparación de Herramientas:** Configuración de `scetool` y entorno Ghidra PPC para análisis del ejecutable.
* [x] **0.2. Descifrado de EBOOT.BIN:** Extracción de `EBOOT.ELF` desprotegido de Minecraft PS3 (Update 1.84).
* [x] **0.3. Localización de ItemRenderer:** Búsqueda y aislamiento de `ItemRenderer::renderItemInFirstPerson` (`0x005DA890`).
* [x] **0.4. Extracción de Offsets:** Documentación de direcciones y vtables definitivas para BLES01976 (`0x005DA890` / `0x0015D084`).
* [x] **0.5. Validación Dinámica en RPCS3:** Breakpoint verificado al cambiar de slot de ítem en la hotbar.

### Fase 1: Pipeline Offline de Conversión de Mallas
* [x] **1.1. Estructura de Cabecera y Vértice:** Definición C++ de `VX3Header` y `VoxelVertex` en [`voxel3d.h`](../sprx/include/voxel3d.h).
* [x] **1.2. Parser de MagicaVoxel (.vox):** Script para leer archivo binario `.vox` y extraer paletas y vóxeles.
* [x] **1.3. Scaffolding y Extracción CLI:** Herramienta de transpilación en [`tools/converter/extract_crayfish_data.py`](../tools/converter/extract_crayfish_data.py).
* [x] **1.4. Algoritmo Greedy Meshing:** Eliminación de caras internas ocultas y fusión de polígonos.
* [x] **1.5. Empaquetador Binario (.vx3):** Generación de archivo final con alineación de 128 bytes para RSX.

### Fase 2: Scaffolding del Plugin SPRX y Hook Seguro
* [x] **2.1. Build System PSL1GHT:** `Makefile` inicial configurado con flags de compilación para Cell PPU.
* [x] **2.2. Esqueleto del Módulo PRX:** Entry points `voxel3d_prx_start` y `voxel3d_prx_stop` en [`main.cpp`](../sprx/src/main.cpp).
* [x] **2.3. Inyector de Trampolín PPU:** Implementación en C++ de parcheo de memoria con `sys_dbg_write_process_memory`.
* [x] **2.4. Telemetría UDP en Tiempo Real:** Integración del cliente NetLogger en el plugin para envío de logs.
* [x] **2.5. Inyección Limpia en Runtime:** Prueba de intercepción en RPCS3 sin congelar el hilo principal (`imported_patch.yml`).

### Fase 3: Pipeline de Renderizado RSX de Bajo Nivel
* [x] **3.1. Adquisición del Contexto GCM:** Captura del puntero global `CellGcmContextData` del motor de 4J.
* [x] **3.2. Reserva del Pool en VRAM:** Alocación del bloque de 4 MB en memoria local GDDR3 (`CELL_GCM_LOCATION_LOCAL`).
* [x] **3.3. Configuración de Vertex Data Arrays:** Enlace de los 4 canales de atributos RSX con `cellGcmAddressToOffset()`.
* [x] **3.4. Emisión de Draw Call Indexado:** Llamada a `cellGcmSetDrawIndexArray()` para dibujar la malla vóxel.
* [x] **3.5. Preservación del Rasterizador:** Guardado y restauración estricta de Depth, Alpha y Culling sin fugas de estado.

### Fase 4: Cinemática, Apuntado (ADS) y Retroceso LERP
* [x] **4.1. Transformaciones Espaciales:** Multiplicación de matriz del modelo vóxel con la matriz de la mano del jugador.
* [x] **4.2. Detección de Controles (L2):** Detección del estado de apuntado en el mando DualShock 3.
* [x] **4.3. Interpolación de Apuntado ADS:** Traslación hacia el centro de la pantalla y multiplicador dinámico de FOV.
* [x] **4.4. Impulso y Recuperación de Retroceso:** Física de retroceso instantáneo y decaimiento exponencial en runtime.
* [x] **4.5. Efecto Visual de Muzzle Flash:** Renderizado del destello de disparo en las coordenadas del cañón.

### Fase 5: Sistema Data-Driven y Release v1.0
* [x] **5.1. Generación de Presets y Configuración:** Transpilación de matrices de MrCrayfish hacia [`crayfish_config.json`](../models/crayfish_config.json) y [`crayfish_transforms.h`](../sprx/include/crayfish_transforms.h).
* [x] **5.2. Tabla de Búsqueda O(1):** Indexación en memoria XDR para resolución inmediata de armas sin alocaciones dinámicas.
* [ ] **5.3. Pruebas de Estabilidad Prolongadas:** Sesiones continuas de 2 horas sin caídas de framerate ni fugas de memoria.
* [ ] **5.4. Soporte Multi-Región:** Validación de compatibilidad con versiones BLES01976 y NPUB31419.
* [ ] **5.5. Empaquetado Final v1.0:** Instalador para CFW/HEN y pack inicial de 5 armas vóxel de prueba.

---

## Matriz de Riesgos y Mitigación

| Riesgo Técnico | Severidad | Probabilidad | Estrategia de Mitigación |
| :--- | :--- | :--- | :--- |
| **Fragmentación / OOM en VRAM del RSX** | Alta | Media | Reservar un pool estático cerrado de 4 MB al inicio; nunca alocar memoria dinámicamente en caliente por cada frame. |
| **Inestabilidad por Caché de Instrucciones PPU** | Crítica | Media | Implementar secuencia estricta de barrera de sincronización (`dcbst` + `icbi` + `isync`) en cada modificación de memoria. |
| **Corrupción de Estado Gráfico RSX** | Alta | Alta | Guardar y restaurar explícitamente los estados de Depth, Alpha y Culling al entrar y salir del hook. |
| **Divergencia de Offsets entre Actualizaciones** | Media | Alta | Utilizar escaneo de signaturas por patrones de bytes (AOB) en lugar de direcciones codificadas en duro. |
| **Congelamiento de la Consola al Desmontar Plugin** | Alta | Baja | Sincronizar el pipeline RSX con `cellGcmFinish()` antes de desvincular los trampolines y liberar el pool. |
