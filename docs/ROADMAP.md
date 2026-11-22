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

## 1. Módulos del Sistema y Arquitectura

### 1.1. Reversing & Engine Internals
* [x] **Configuración de herramientas:** Configuración de `scetool` y entorno Ghidra/IDA para Cell PPU en [`REVERSE_ENGINEERING.md`](REVERSE_ENGINEERING.md).
* [x] **Identificación de clases objetivo:** Layouts C++ de `ItemStack`, `InventoryPlayer` y `ItemRenderer` en [`ENGINE_STRUCTURES.md`](ENGINE_STRUCTURES.md).
* [x] **Mapeo de matrices del brazo:** Análisis de `renderFirstPersonArm` y pila de matrices en [`ARCHITECTURE.md`](ARCHITECTURE.md).
* [~] **Descifrado de binario base:** Extracción y análisis del `EBOOT.ELF` desprotegido de Minecraft PS3 (Update 1.84).
* [ ] **Localización de vtable offsets:** Extracción de offsets específicos para BLES01976 y NPUB31419.
* [ ] **Aislamiento de la rama 2D:** Bypass de la extrusión de quads de sprites para ítems personalizados.

### 1.2. Formato de Malla VX3 & Tooling
* [x] **Estructura de Cabecera:** Cabecera binaria `VX3Header` (16 bytes) definida en [`VX3_FORMAT.md`](VX3_FORMAT.md) y [`voxel3d.h`](../sprx/include/voxel3d.h).
* [x] **Layout de Vértice Compacto:** Estructura `VoxelVertex` (12 bytes con `int16_t` y normales empaquetadas).
* [x] **Especificación de Optimización:** Algoritmos de *Greedy Meshing* y *Occlusion Face Culling* en [`VX3_FORMAT.md`](VX3_FORMAT.md).
* [~] **Estructura de herramienta CLI:** Esqueleto y configuración de entorno para conversor en [`tools/converter/`](../tools/converter/).
* [ ] **Parser de MagicaVoxel (.vox):** Módulo de importación de vóxeles y paletas de color `.vox`.
* [ ] **Parser de Blockbench (.bbmodel):** Importador de modelos JSON/Blockbench.
* [ ] **Generador Binario VX3:** Empaquetador offline con alineación de 128 bytes para RSX.

### 1.3. Plugin SPRX & Hooking PPU
* [x] **Build System PSL1GHT:** `Makefile` configurado con flags Cell PPU (`-mcpu=cell -fno-rtti -fno-exceptions`).
* [x] **Punto de Entrada PRX:** Funciones del ciclo de vida del módulo (`voxel3d_prx_start` / `voxel3d_prx_stop`) en [`main.cpp`](../sprx/src/main.cpp).
* [x] **Diseño de Trampolines PowerPC:** Secuencia de salto absoluto de 16 bytes (`lis/ori/mtctr/bctr`) en [`PPU_HOOKING.md`](PPU_HOOKING.md).
* [x] **Coherencia de Caché PPU:** Rutina de sincronización e invalidación estricta (`dcbst/sync/icbi/isync`).
* [~] **Inyector de Hooks C++:** Implementación de `install_ppu_hook` con llamadas `sys_dbg_write_process_memory`.
* [~] **Telemetría UDP NetLogger:** Transmisión de logs en tiempo real por red hacia PC de desarrollo.
* [ ] **Verificación en RPCS3:** Validación del hook en emulador sin congelamiento ni desalineación de pila.

### 1.4. Pipeline Gráfico RSX (libgcm)
* [x] **Mapeo de Direcciones a Offsets:** Conversión de punteros efectivos a offsets del RSX con `cellGcmAddressToOffset()`.
* [x] **Binding de Atributos de Vértices:** Configuración de 4 canales GCM (`ATTR 0` a `ATTR 3`) en [`RSX_PIPELINE.md`](RSX_PIPELINE.md).
* [x] **Preservación de Estado del Rasterizador:** Reglas de Cero Fuga de Estado (Depth, Culling, Alpha Test).
* [x] **Gestión de Memoria VRAM:** Pool estático cerrado de 4 MB en memoria local GDDR3 en [`MEMORY_MANAGEMENT.md`](MEMORY_MANAGEMENT.md).
* [x] **Matriz MVP de Constantes:** Paso de matriz $M_{final}$ al Vertex Program mediante `cellGcmSetVertexProgramConstants()`.
* [~] **Adquisición del Contexto GCM:** Captura del puntero global `CellGcmContextData` del motor de 4J.
* [ ] **Emisión de Draw Call Indexado:** Ejecución de `cellGcmSetDrawIndexArray()` para mallas vóxel.
* [ ] **Validación con RenderDoc:** Captura de frame bajo backend Vulkan en RPCS3 para verificar geometría.

### 1.5. Cinemática, ADS & Configuración Data-Driven
* [x] **Modelo Matemático de Retroceso:** Algoritmo de impulso instantáneo y decaimiento exponencial ($P_{target} + \Delta P \cdot e^{-\lambda t}$).
* [x] **Interpolación de Apuntado ADS:** Traslación hacia el centro de la cámara y ajuste multiplicativo del FOV.
* [x] **Esquema de Configuración JSON:** Especificación formal del archivo `config.json` en [`CONFIG_SPEC.md`](CONFIG_SPEC.md).
* [x] **Estructura C++ en Memoria:** Estructura compacta `VoxelItemConfig` para búsqueda $O(1)$ sin alocaciones dinámicas.
* [ ] **Lector Ligero de JSON en CellFS:** Parser embebido para leer configuraciones desde `/dev_hdd0/`.
* [ ] **Detección de Gatillo L2:** Lectura del estado de apuntado del mando DualShock 3.
* [ ] **Partículas de Muzzle Flash:** Posicionamiento del destello y salida de proyectiles en el cañón.

---

## 2. Hitos de Implementación (Milestones)

### Fase 0: Ingeniería Inversa y Mapeo de Memoria
* [x] **0.1. Metodología de análisis:** Guía y preparación de entorno para desensamblado PowerPC en [`REVERSE_ENGINEERING.md`](REVERSE_ENGINEERING.md).
* [x] **0.2. Layouts de clases del motor:** Reconstrucción de `ItemStack`, `InventoryPlayer` y `ItemRenderer`.
* [~] **0.3. Descifrado de EBOOT.BIN:** Extracción de ELF desprotegido de la versión 1.84 (BLES01976 / NPUB31419).
* [ ] **0.4. Localización de offsets PPU:** Offsets verificados de `renderItemInFirstPerson` y `renderFirstPersonArm`.
* [ ] **0.5. Validación dinámica en RPCS3:** Breakpoint verificado al cambiar de slot de ítem en la hotbar.

### Fase 1: Pipeline Offline de Conversión de Mallas
* [x] **1.1. Especificación del formato VX3:** Formato binario con cabecera de 16 bytes y empaquetado de vértices.
* [x] **1.2. Definición de algoritmos de optimización:** Reglas de *Greedy Meshing* y *Occlusion Culling*.
* [~] **1.3. Scaffolding de la herramienta CLI:** Estructura del proyecto en [`tools/converter/`](../tools/converter/).
* [ ] **1.4. Implementación del optimizador de mallas:** Algoritmo de reducción de triángulos para mallas vóxel.
* [ ] **1.5. Conversión de modelo de prueba:** Exportación y verificación de modelo complejo (AK-47) $< 30\text{ KB}$.

### Fase 2: Scaffolding del Plugin SPRX y Hook Seguro
* [x] **2.1. Toolchain y Build System:** `Makefile` PSL1GHT v2 y reglas de compilación para Cell PPU.
* [x] **2.2. Punto de entrada y salida del módulo:** Estructura PRX (`voxel3d_prx_start` / `voxel3d_prx_stop`).
* [x] **2.3. Arquitectura del trampolín PPU:** Salto absoluto de 16 bytes e invalidación de caché L1.
* [~] **2.4. Telemetría de depuración por red:** Integración del cliente NetLogger UDP hacia PC de desarrollo.
* [ ] **2.5. Inyección limpia en caliente:** Parcheo de memoria y ejecución del hook sin cuelgues de consola.

### Fase 3: Pipeline de Renderizado RSX de Bajo Nivel
* [x] **3.1. Especificación de Vertex Arrays:** Mapeo de atributos RSX y offsets de hardware.
* [x] **3.2. Presupuesto y alocación en VRAM:** Diseño del pool cerrado estático de 4 MB en GDDR3.
* [x] **3.3. Preservación del rasterizador:** Configuración de Depth, Alpha y Culling sin fugas de estado.
* [ ] **3.4. Reserva física de memoria local:** Alocación en `dev_gcm` y carga de modelos desde disco duro.
* [ ] **3.5. Emisión de Draw Call indexado:** Renderizado 3D de la malla vóxel reemplazando el sprite 2D.

### Fase 4: Cinemática, Apuntado (ADS) y Retroceso LERP
* [x] **4.1. Formulación matemática del retroceso:** Algoritmo de retroceso instantáneo y recuperación suave.
* [x] **4.2. Especificación de ADS y FOV:** Interpolación de posición central y multiplicador de campo de visión.
* [ ] **4.3. Multiplicación de matrices en tiempo real:** Transformación ligada a la mano del jugador.
* [ ] **4.4. Detección de controles (L2):** Activación fluida del modo apuntado con el mando.
* [ ] **4.5. Renderizado de Muzzle Flash:** Destello de disparo sincronizado con el arma.

### Fase 5: Sistema Data-Driven y Estabilización v1.0
* [x] **5.1. Esquema JSON de configuración:** Especificación del formato de datos para armas y parámetros.
* [x] **5.2. Estructura C++ de acceso rápido:** Caché $O(1)$ en memoria para parámetros sin búsquedas costosas.
* [ ] **5.3. Parser embebido de JSON:** Lectura de `config.json` en PS3 con CellFS sin fugas de memoria.
* [ ] **5.4. Pruebas de estabilidad prolongadas:** Sesiones continuas de 2 horas sin caídas de FPS ni memory leaks.
* [ ] **5.5. Empaquetado final v1.0:** Instalador para CFW/HEN y paquete inicial de 5 armas de prueba.

---

## Matriz de Riesgos y Mitigación

| Riesgo Técnico | Severidad | Probabilidad | Estrategia de Mitigación |
| :--- | :--- | :--- | :--- |
| **Fragmentación / OOM en VRAM del RSX** | Alta | Media | Reservar un pool estático cerrado de 4 MB al inicio; nunca alocar memoria dinámicamente en caliente por cada frame. |
| **Inestabilidad por Caché de Instrucciones PPU** | Crítica | Media | Implementar secuencia estricta de barrera de sincronización (`dcbst` + `icbi` + `isync`) en cada modificación de memoria. |
| **Corrupción de Estado Gráfico RSX** | Alta | Alta | Guardar y restaurar explícitamente los estados de Depth, Alpha y Culling al entrar y salir del hook. |
| **Divergencia de Offsets entre Actualizaciones** | Media | Alta | Utilizar escaneo de signaturas por patrones de bytes (AOB) en lugar de direcciones codificadas en duro. |
| **Congelamiento de la Consola al Desmontar Plugin** | Alta | Baja | Sincronizar el pipeline RSX con `cellGcmFinish()` antes de desvincular los trampolines y liberar el pool. |
