# Guía de Compilación del Entorno (Toolchain Setup)

Este documento detalla los pasos reales y requisitos para configurar el entorno de desarrollo cruzado y compilar el módulo nativo `.sprx` de **Voxel3D-PS3**.

---

## 1. Requisitos del Sistema (Host Linux)

Se recomienda un sistema operativo basado en Linux (Ubuntu 20.04/22.04 LTS, Debian 11/12, Arch Linux o Fedora).

### Paquetes y Dependencias Base:

En sistemas basados en Debian / Ubuntu:
```bash
sudo apt update
sudo apt install -y build-essential autoconf automake bison flex texinfo \
                    libelf-dev libgmp-dev libmpfr-dev libmpc-dev zlib1g-dev \
                    python3 python3-pip git wget curl
```

En Arch Linux:
```bash
sudo pacman -S base-devel autoconf automake bison flex texinfo libelf \
               gmp mpfr libmpc zlib python git wget curl
```

---

## 2. Instalación del Toolchain Open Source (ps3dev / PSL1GHT v2)

El plugin requiere el compilador GCC configurado con la arquitectura objetivo `ppu-lv2` (PowerPC Cell).

### 2.1. Clonado y Construcción de ps3dev:
```bash
# Definir directorio de instalación (por ejemplo, /usr/local/ps3dev)
sudo mkdir -p /usr/local/ps3dev
sudo chown -R $USER /usr/local/ps3dev

# Clonar el entorno automatizado de construcción
git clone https://github.com/ps3dev/ps3toolchain.git
cd ps3toolchain

# Iniciar la compilación de binutils, gcc (PPU/SPU) y PSL1GHT
./toolchain.sh
```

### 2.2. Configuración de Variables de Entorno:
Añade las siguientes líneas a tu archivo `~/.bashrc` o `~/.zshrc`:

```bash
export PS3DEV=/usr/local/ps3dev
export PSL1GHT=$PS3DEV
export PATH=$PATH:$PS3DEV/bin:$PS3DEV/ppu/bin:$PS3DEV/spu/bin
```

Aplica los cambios en la terminal actual:
```bash
source ~/.bashrc
```

Verifica que el compilador esté disponible en el path:
```bash
ppu-lv2-gcc --version
```

---

## 3. Compilación del Plugin Voxel3D-PS3

Una vez configurado el toolchain:

1. Navega hasta el directorio del código fuente del plugin:
   ```bash
   cd sprx
   ```

2. Ejecuta la compilación con `make`:
   ```bash
   make
   ```

3. **Artefactos generados:**
   * `src/main.o`: Código objeto compilado para la arquitectura Cell PPU.
   * `Voxel3D-PS3.prx`: Ejecutable relocable sin cifrar.
   * `Voxel3D-PS3.sprx`: Módulo dinámico final empaquetado para CellOS Lv2.

4. Limpieza del árbol de compilación:
   ```bash
   make clean
   ```

---

## 4. Solución de Problemas Comunes

* **`ppu-lv2-gcc: command not found`:** Las variables de entorno `PS3DEV` o `PATH` no están cargadas en la sesión actual de la terminal.
* **Fallo en inclusión de cabeceras (`voxel3d.h` o `sysmodule`):** Asegúrate de que `$PS3DEV/portlibs/ppu/include` contenga las bibliotecas base de PSL1GHT.
