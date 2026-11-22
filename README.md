# Voxel3D-PS3 🎮🔫

**Voxel3D-PS3** es un proyecto de investigación y desarrollo de un plugin nativo en C++ (`.sprx`) para **Minecraft: PlayStation 3 Edition** (binario nativo C++ compilado originalmente por 4J Studios).

Su propósito principal es reemplazar el pipeline original de renderizado de ítems en primera persona (sprites planos 2D con extrusión dinámica de quads) por **mallas vóxel 3D precalculadas cargadas directamente en la VRAM local (RSX)**, emulando la estética de mods icónicos como *MrCrayfish's Gun Mod* sin colapsar el estrecho presupuesto de memoria y procesamiento de la consola.

---

## 1. El Problema Técnico en PlayStation 3

* **Arquitectura de hardware:**
  * **Cell Broadband Engine:** PPU (PowerPC 64-bit) + 6 SPUs disponibles.
  * **GPU RSX (Reality Synthesizer):** Arquitectura NVIDIA G70.
  * **Memoria segmentada asimétrica:** 256 MB XDR RAM (sistema/PPU) y 256 MB GDDR3 VRAM (video/RSX).
* **Limitaciones del motor 4J Studios:**
  * El renderizado en primera persona dentro del `EBOOT.BIN` está hardcodeado y bifurca únicamente en dos caminos:
    1. Bloques cúbicos completos de terreno (renderizados como voxel 16x16 estándar).
    2. Sprites planos 2D generados con extrusión dinámica de quads (alto coste de CPU/PPU al recalcular quads por frame).
  * No existe un parser de modelos dinámicos (JSON/OBJ) en tiempo de ejecución.
  * Emitir draw calls individuales desde la CPU saturaría el command buffer del RSX, provocando cuelgues o caídas críticas de framerate.

---

## 2. Pipeline de Solución Propuesto

```
[ Modelo Vóxel (Blockbench / MagicaVoxel) ]
                     │
                     ▼ (Conversor CLI Offline - Python/C++)
[ Archivo Plano Binario (.vx3) ]
  • Array contiguo de vértices optimizados (16-bit half-float / short)
  • Malla optimizada con culling de caras internas ocultas
  • Tamaño estimado: ~15-25 KB por modelo
                     │
                     ▼ (Carga en memoria PS3)
[ Plugin Voxel3D-PS3 (.sprx) ]
  • Alocación de VBO estático en memoria local GDDR3 RSX (dev_gcm)
  • Hook en ItemRenderer::renderItemInFirstPerson (PPU)
  • Detección de ID de ítem/arma personalizada
  • Emisión de 1 solo Draw Call al RSX (cellGcmSetDrawIndexArray)
  • Interpolación matemática (LERP) de retroceso y apuntado ligada al brazo
```

---

## 3. Estructura del Repositorio

```
Voxel3D-PS3/
├── README.md                  # Visión general y estado del proyecto
├── docs/                      # Especificaciones técnicas y guías de reversing
│   ├── ARCHITECTURE.md        # Arquitectura del hook y ciclo de render RSX GCM
│   ├── REVERSE_ENGINEERING.md # Metodología de análisis de EBOOT.BIN (Ghidra/IDA)
│   └── VX3_FORMAT.md          # Especificación del formato binario .vx3
├── sprx/                      # Código fuente del plugin nativo C++ (.sprx)
│   ├── Makefile               # Script de compilación PSL1GHT / ps3dev
│   ├── include/               # Cabeceras (.h)
│   └── src/                   # Implementación del hook y llamadas libgcm
├── tools/                     # Utilidades offline (conversor de mallas vóxel)
│   └── converter/             # Script CLI para empaquetar modelos a .vx3
└── models/                    # Modelos de prueba y especificaciones
```

---

## 4. Fases de Desarrollo

1. **Ingeniería Inversa:**
   * Análisis estático y dinámico del `EBOOT.BIN` de Minecraft PS3.
   * Localización de `ItemRenderer::renderItemInFirstPerson`, matrices de proyección/modelo y matrices de transformación del brazo.
2. **Scaffold del Hook C++:**
   * Configuración de plugin dinámico `.sprx` utilizando el toolchain `PSL1GHT` / `ps3dev`.
   * Intercepción limpia de funciones del PPU sin romper la sincronización del hilo principal.
3. **Pipeline de Mallas (.vx3):**
   * Definición del formato binario compacto (`.vx3`) con vértices comprimidos.
   * Script offline para exportar desde MagicaVoxel / Blockbench eliminando oclusión interna.
4. **Renderizado RSX de Bajo Nivel:**
   * Inicialización de buffers estáticos en VRAM local mediante libgcm (`cellGcmSetDrawIndexArray`).
   * Aplicación de transformaciones matriciales (rotación, traslación, retroceso lerp).

---

## 5. Requisitos y Herramientas

* **Toolchain de Compilación:** `ps3dev` / `PSL1GHT`
* **Desensamblador:** Ghidra (con extensión PowerPC 64 / Cell PPU) o IDA Pro
* **Entorno de Prueba:** PlayStation 3 con Custom Firmware (CFW Evilnat / DEX) o emulador RPCS3 con soporte de debugging
* **Python 3.10+** (para herramientas del pipeline de modelos)

---

## Licencia

Proyecto de investigación educativa y desarrollo de software experimental.
