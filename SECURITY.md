# Política de Seguridad y Estabilidad del Sistema

Este documento establece las directrices de seguridad técnica, estabilidad de hardware y prevención de riesgos para el desarrollo y uso de **Voxel3D-PS3**.

---

## 1. Riesgos de Hardware y Estabilidad en Consola Física

El desarrollo de software nativo a bajo nivel en PlayStation 3 (Cell PPU y GPU RSX) interactúa directamente con la memoria física del sistema operativo (CellOS Lv2).

### 1.1. Prevención de Congelamientos Críticos (Hard Freezes)
* **Causa:** Desincronización del bus FIFO de comandos del RSX, bucles infinitos en el hilo principal del PPU o excepciones de memoria no controladas.
* **Impacto:** Un congelamiento total obliga a apagar la consola desconectando la alimentación o manteniendo presionado el botón de encendido. Esto puede corromper el sistema de archivos `UFS2` en `/dev_hdd0/`, forzando al sistema a ejecutar la reconstrucción de la base de datos de almacenamiento.
* **Protocolo de Mitigación Obligatorio:**
  1. Todo código nuevo o modificado debe ser probado y validado primero en el emulador **RPCS3**.
  2. Solo tras comprobar que no se generan desalineaciones de pila ni fallos de segmentación en emulación, se autoriza la prueba en consolas físicas con Custom Firmware (CFW) o HEN.

### 1.2. Límites Térmicos y Sobrecarga del Cell PPU
* El bucle de renderizado de Minecraft se ejecuta a 60 frames por segundo en el hilo principal de la PPU.
* El código del hook debe ejecutarse en tiempo constante $O(1)$ sin realizar operaciones de entrada/salida (I/O) a disco ni cálculos pesados en caliente.
* Se recomienda supervisar las temperaturas de PPU y RSX mediante el menú térmico de webMAN MOD durante pruebas continuas.

---

## 2. Política de Red y Advertencia de PlayStation Network (PSN)

> [!WARNING]
> **Riesgo de Bloqueo de Consola (IDPS Ban):**
> Este proyecto está diseñado única y exclusivamente para investigación, experimentación y juego **offline / local** en consolas con Custom Firmware o emulador.

* La inyección de código dinámico o módulos `.sprx` en la memoria del juego mientras se mantiene una sesión activa en PlayStation Network (PSN) viola los términos de servicio de la plataforma y activará los mecanismos antitrampas de Sony.
* Lo anterior puede derivar en la suspensión permanente de la cuenta de usuario y el baneo definitivo del identificador de la consola (`IDPS`).
* **Regla estricta:** Desactivar el inicio de sesión automático de PSN y deshabilitar llamadas de red externas no autorizadas en el código del plugin.

---

## 3. Reporte de Vulnerabilidades o Inestabilidades

Si identificas una condición de carrera, fuga de memoria no controlada en VRAM o instrucción que provoque corrupción del kernel Lv2:

1. Documenta el TitleID exacto del juego (ej. `BLES01976`), la versión del firmware (ej. `Evilnat 4.91`) y el escenario de reproducción.
2. Abre un reporte en la pestaña de *Issues* del repositorio con la etiqueta `stability` o `bug`.
