# Preguntas Frecuentes Técnicas (FAQ)

Respuestas técnicas a las dudas más comunes sobre la arquitectura, diseño y compatibilidad de **Voxel3D-PS3**.

---

### 1. ¿Por qué no cargar archivos `.obj`, `.fbx` o `.json` directamente en la consola?
La unidad PPU del Cell Broadband Engine tiene una capacidad limitada para operaciones intensivas de análisis de texto ASCII en tiempo real. Parsear cadenas de caracteres, dividir tokens y generar buffers de geometría dinámica en caliente provocaría caídas críticas de framerate (*stuttering*) y fragmentaría la memoria XDR. 

Por esta razón, el pipeline utiliza un conversor offline en PC que precalcula toda la geometría, elimina caras ocultas y empaqueta los datos en una estructura binaria plana (`.vx3`) que la GPU RSX consume directamente sin procesamiento intermedio.

---

### 2. ¿Por qué utilizar la API `libgcm` en lugar de OpenGL / PSGL?
Sony proporcionó la biblioteca **PSGL** (envolvente de OpenGL ES para PlayStation 3), pero esta capa introduce una sobrecarga considerable de memoria (reserva entre 30 y 50 MB adicionales solo para inicializar su propio contexto) y no permite un control granular del command buffer.

El motor de Minecraft: PS3 Edition (desarrollado por 4J Studios) fue construido directamente sobre la API nativa de bajo nivel **`libgcm`**. Para interceptar el ciclo de renderizado sin corromper el estado del motor ni duplicar buffers gráficos, el plugin debe emitir comandos FIFO nativos utilizando el mismo contexto GCM del juego.

---

### 3. ¿El renderizado vóxel 3D reducirá los FPS del juego?
El objetivo de diseño es mantener **60 FPS estables** sin impacto medible. De hecho, en el juego vanilla, cada ítem 2D en primera persona se dibuja mediante un algoritmo de extrusión que recalcula decenas de micro-quads en la CPU cuadro por cuadro. 

Al reemplazar este cálculo por un único comando de dibujo indexado (`cellGcmSetDrawIndexArray`) leyendo una malla estática alojada en la memoria GDDR3 del RSX, la carga sobre la PPU disminuye.

---

### 4. ¿Es compatible tanto con consolas reales como con el emulador RPCS3?
Sí. El plugin está diseñado para ser agnóstico del entorno siempre que la arquitectura sea CellOS:
* **RPCS3:** Entorno ideal para desarrollo rápido y depuración visual mediante RenderDoc (backend Vulkan) o GDB stub.
* **Consola física (PS3):** Compatible con Custom Firmware (CFW Evilnat 4.90/4.91 CEX/PEX/DEX) o PS3HEN mediante cargadores de plugins como webMAN MOD o Cobra.

---

### 5. ¿Puedo jugar en línea en PlayStation Network (PSN) con este plugin cargado?
**No.** La inyección de código dinámico en la sección `.text` de la PPU altera la memoria de un proceso comercial en ejecución. Conectarse a PSN con plugins de juego activos activa los filtros de telemetría antitrampas de Sony, derivando en el baneo permanente de la cuenta de usuario y de la consola (`IDPS`). El proyecto es para uso **estrictamente local y offline**.

---

### 6. ¿Cómo se configuran los efectos de retroceso y apuntado?
Todos los parámetros dinámicos (retroceso angular, traslación de retroceso, velocidad de recuperación LERP, posición de la mira ADS y coordenadas del cañón) están desacoplados del código fuente y se definen en el archivo [`config.json`](CONFIG_SPEC.md), permitiendo ajustar cada arma sin necesidad de recompilar el binario `.sprx`.
