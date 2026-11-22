# Índice Maestro de Documentación: Voxel3D-PS3

Bienvenido al centro de documentación técnica de **Voxel3D-PS3**. Este directorio contiene las especificaciones de bajo nivel, análisis de ingeniería inversa, manuales de compilación y guías de assets organizadas por áreas de especialidad.

---

## 🗂️ Organización de Documentos

```
docs/
├── README.md                  # Este índice general
├── 1. Entorno de Desarrollo y Compilación
│   ├── COMPILATION_GUIDE.md   # Instalación del toolchain ps3dev en Linux
│   ├── WINDOWS_SETUP.md       # Configuración para Windows (WSL2, MSYS2 y RPCS3)
│   └── DEBUG_AND_TOOLCHAIN.md # Depuración con GDB, RenderDoc y NetLogger UDP
├── 2. Arquitectura de Bajo Nivel y Hardware
│   ├── ARCHITECTURE.md        # Visión general del hook y flujo de render
│   ├── RSX_PIPELINE.md        # Comandos FIFO, vertex arrays y shaders libgcm
│   ├── MEMORY_MANAGEMENT.md   # Presupuestos VRAM GDDR3, XDR y alineación a 128 bytes
│   └── PPU_HOOKING.md         # Trampolines PowerPC de 16 bytes y coherencia de caché
├── 3. Ingeniería Inversa de Minecraft PS3 (4J Studios)
│   ├── REVERSE_ENGINEERING.md # Metodología de análisis en Ghidra/IDA y EBOOT
│   └── ENGINE_STRUCTURES.md   # Reconstrucción de clases C++ (ItemStack, ItemRenderer)
├── 4. Formatos de Assets y Configuración
│   ├── VX3_FORMAT.md          # Especificación del formato binario .vx3 (12 bytes/vert)
│   ├── ASSET_PIPELINE.md      # Guía para modeladores 3D (Blockbench / MagicaVoxel)
│   └── CONFIG_SPEC.md         # Esquema JSON data-driven de armas y físicas
└── 5. Gestión del Proyecto
    ├── ROADMAP.md             # Hoja de ruta, hitos reales y matriz de riesgos
    └── FAQ.md                 # Preguntas frecuentes técnicas y decisiones de diseño
```

---

## 📚 Resumen por Categorías

### 1. Entorno de Desarrollo y Compilación
* **[Guía de Compilación en Linux (`COMPILATION_GUIDE.md`)](COMPILATION_GUIDE.md):** Requisitos de paquetes, compilación de `ps3toolchain` y uso de `make` en el plugin.
* **[Guía para Windows (`WINDOWS_SETUP.md`)](WINDOWS_SETUP.md):** Configuración de WSL2 (Ubuntu), MSYS2, Python 3 nativo y vinculación con RPCS3.
* **[Depuración y Telemetría (`DEBUG_AND_TOOLCHAIN.md`)](DEBUG_AND_TOOLCHAIN.md):** Captura de frames en Vulkan con RenderDoc, conexión GDB y logger por red UDP.

### 2. Arquitectura de Bajo Nivel y Hardware
* **[Arquitectura General (`ARCHITECTURE.md`)](ARCHITECTURE.md):** Topología Cell/RSX, punto de intercepción en `ItemRenderer` y modelo cinemático.
* **[Pipeline Gráfico RSX (`RSX_PIPELINE.md`)](RSX_PIPELINE.md):** Mapeo con `cellGcmAddressToOffset`, binding de 4 canales de atributos y Cero Fuga de Estado.
* **[Gestión de Memoria (`MEMORY_MANAGEMENT.md`)](MEMORY_MANAGEMENT.md):** Pool cerrado estático de 4 MB en memoria GDDR3 local y prevención de colapsos por OOM.
* **[Hooking en PPU (`PPU_HOOKING.md`)](PPU_HOOKING.md):** Secuencia de salto absoluto (`lis/ori/mtctr/bctr`) e invalidación de caché de instrucciones (`dcbst/sync/icbi/isync`).

### 3. Ingeniería Inversa de 4J Studios
* **[Metodología de Reversing (`REVERSE_ENGINEERING.md`)](REVERSE_ENGINEERING.md):** Descifrado de `EBOOT.BIN` a `EBOOT.ELF` y localización de cadenas clave y vtables.
* **[Estructuras del Motor (`ENGINE_STRUCTURES.md`)](ENGINE_STRUCTURES.md):** Layouts en memoria de `ItemStack`, `InventoryPlayer`, `EntityPlayer` y matrices column-major.

### 4. Formatos de Assets y Configuración
* **[Especificación Binaria VX3 (`VX3_FORMAT.md`)](VX3_FORMAT.md):** Cabecera de 16 bytes y estructura `VoxelVertex` de 12 bytes (`int16_t` y normales empaquetadas).
* **[Pipeline de Modelado 3D (`ASSET_PIPELINE.md`)](ASSET_PIPELINE.md):** Convención de ejes, pivote en empuñadura y presupuestos de triángulos por tipo de arma.
* **[Configuración Data-Driven (`CONFIG_SPEC.md`)](CONFIG_SPEC.md):** Parámetros de armas en JSON (retroceso instantáneo, amortiguación exponencial y apuntado ADS).

### 5. Gestión del Proyecto y Buenas Prácticas
* **[Hoja de Ruta e Hitos (`ROADMAP.md`)](ROADMAP.md):** Seguimiento del avance real, tareas pendientes y matriz de riesgos técnicos.
* **[Preguntas Frecuentes (`FAQ.md`)](FAQ.md):** Respuestas técnicas sobre `libgcm`, rendimiento de CPU/GPU y políticas de red.
* **[Guía de Contribución (`../CONTRIBUTING.md`)](../CONTRIBUTING.md):** Normas de código C++ para Cell PPU y política de cero métricas falsas.
* **[Seguridad y Estabilidad (`../SECURITY.md`)](../SECURITY.md):** Prevención de congelamientos de consola y advertencias sobre PlayStation Network.
