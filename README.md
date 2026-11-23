# Voxel3D-PS3: Minecraft PS3 Modding Framework & Loader

**Voxel3D-PS3** es un ecosistema profesional de modding para **Minecraft: PlayStation 3 Edition (BLES01976)**. Proporciona una arquitectura nativa de alto rendimiento en C++ que permite inyectar modelos voxel en 3D, cargar módulos `.sprx` a 60 FPS sin sobrecargar los 256 MB de RAM de la consola, transpilar mods clásicos de Forge Java (`.jar`), y gestionar todo desde una aplicación Homebrew oficial basada en **Apollo Save Tool**.

---

## 🏛️ Arquitectura del Proyecto

```
Voxel3D-PS3/
├── app/                  # Aplicación Homebrew de PS3 (UI de Apollo Save Tool / Lanzador de Juego)
│   ├── data/             # Texturas nativas, fuentes TTF y sonidos
│   ├── include/          # Cabeceras C (menu, draw, pad, etc.)
│   ├── pkgfiles/         # ICON0.PNG, PIC1.PNG, metadatos de empaquetado
│   ├── source/           # Código fuente C (draw.c, menu_main.c, main.c)
│   ├── Makefile          # Compilación de EBOOT.BIN / PKG
│   └── sfo.xml           # Configuración PARAM.SFO (Title: Minecraft PS3 Mod Loader)
│
├── core/                 # Motor Central Inyectable en el Juego (voxel3d.sprx)
│   ├── include/          # Cabeceras PPU (rsx_pipeline, hook, engine_structures, etc.)
│   ├── src/              # Implementación PPU en C++ (trampolines de salto, render RSX)
│   ├── test/             # Pruebas unitarias de host
│   └── Makefile          # Compilación de voxel3d.sprx
│
├── sdk/                  # SDK de Desarrollo de Mods para Creadores
│   ├── include/
│   │   └── forge/        # Capa de compatibilidad de Forge API en C++ (ForgeShim.h)
│   └── README.md         # Guía de creación de mods
│
├── mods/                 # Mods y Paquetes de Contenido
│   └── m4a1/             # Mod de referencia M4A1 3D Voxel Weapon
│       ├── config.json   # Matrices de transformación, ADS y retroceso
│       └── models/       # Modelo binario m4a1.vx3
│
├── tools/                # Herramientas de PC y Pipeline de Conversión
│   ├── converter/        # extract_crayfish_data.py, voxel_converter.py
│   ├── jar2sprx/         # Transpilador automático de mods Java (.jar) a C++
│   └── packaging/        # Herramientas de empaquetado (package.conf, empaquetar.bat, PyKG)
│
├── docs/                 # Documentación Técnica de Ingeniería Inversa
├── Makefile              # Makefile Maestro (compila core y app)
└── README.md
```

---

## 🚀 Componentes Principales

1. **`app/` (El Mod Loader de PS3):**
   * Corre como una aplicación nativa en el menú XMB de la consola y en RPCS3.
   * Utiliza el motor gráfico 2D oficial de **Apollo Save Tool** de Bucanero.
   * Permite activar o desactivar mods con el mando DualShock 3 y lanza el juego directamente con `sys_game_process_exitspawn2`.

2. **`core/` (El Motor In-Game `voxel3d.sprx`):**
   * Módulo dinámico inyectado en el proceso de Minecraft.
   * Intercepta el bucle de renderizado en primera persona (`renderItemInFirstPerson`).
   * Dibuja mallas de voxeles de alta fidelidad enviando llamadas de vértices e índices directamente a la GPU RSX.

3. **`tools/jar2sprx/` (El Transpilador de Mods Java):**
   * Convierte mods clásicos de Forge (`.jar`) en código C++ nativo listo para PS3 sin consumo excesivo de memoria.

---

## 🛠️ Requisitos de Compilación
* **Toolchain:** PSL1GHT v2 o Sony PS3 SDK (`ppu-lv2-gcc`).
* **Emulador de Pruebas:** RPCS3 v0.0.40+ (Windows / Linux).
* **Consola Física:** PlayStation 3 con Custom Firmware (CFW) o PS3HEN.

---

## 📄 Licencia
Distribuido bajo los términos de la licencia MIT y las licencias de componentes de código abierto asociadas.
