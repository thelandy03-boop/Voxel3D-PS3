# Guía de Instalación y Desarrollo en Windows

Este documento detalla cómo configurar un entorno de compilación, prueba y desarrollo para **Voxel3D-PS3** en sistemas operativos **Windows 10 / Windows 11 (64-bit)**.

---

## 1. Métodos Soportados en Windows

Para compilar código de PlayStation 3 (Cell PPU) en Windows, disponemos de dos vías técnicas:

| Método | Dificultad | Rendimiento | Recomendación |
| :--- | :---: | :---: | :--- |
| **Opción A: WSL2 (Windows Subsystem for Linux)** | Baja / Media | Nativo | **Altamente Recomendado** (Estable y rápido) |
| **Opción B: MSYS2 / MinGW64** | Media / Alta | Bueno | Alternativa sin virtualización Hyper-V |

---

## 2. Opción A: Configuración con WSL2 (Recomendada)

WSL2 ejecuta un kernel Linux real dentro de Windows, permitiendo utilizar el toolchain estándar `ps3dev / PSL1GHT` sin incompatibilidades de rutas o librerías POSIX.

### 2.1. Habilitar WSL2 e Instalar Ubuntu:
Abre **PowerShell** como Administrador y ejecuta:
```powershell
wsl --install -d Ubuntu
```
Reinicia la computadora si Windows lo solicita.

### 2.2. Instalar Dependencias dentro de Ubuntu (WSL):
Abre la terminal de Ubuntu y ejecuta:
```bash
sudo apt update && sudo apt upgrade -y
sudo apt install -y build-essential autoconf automake bison flex texinfo \
                    libelf-dev libgmp-dev libmpfr-dev libmpc-dev zlib1g-dev \
                    python3 python3-pip git wget curl
```

### 2.3. Compilar el Toolchain ps3dev:
```bash
sudo mkdir -p /usr/local/ps3dev
sudo chown -R $USER /usr/local/ps3dev

git clone https://github.com/ps3dev/ps3toolchain.git
cd ps3toolchain
./toolchain.sh
```

### 2.4. Configurar Variables de Entorno en WSL:
Edita `~/.bashrc`:
```bash
echo 'export PS3DEV=/usr/local/ps3dev' >> ~/.bashrc
echo 'export PSL1GHT=$PS3DEV' >> ~/.bashrc
echo 'export PATH=$PATH:$PS3DEV/bin:$PS3DEV/ppu/bin:$PS3DEV/spu/bin' >> ~/.bashrc
source ~/.bashrc
```

### 2.5. Acceder a los Archivos del Proyecto desde Windows:
Los archivos ubicados en tu disco `C:` son accesibles desde WSL en `/mnt/c/`.
Por ejemplo, si clonaste el repositorio en `C:\Proyectos\Voxel3D-PS3`:
```bash
cd /mnt/c/Proyectos/Voxel3D-PS3/sprx
make
```

---

## 3. Opción B: Entorno Nativo con MSYS2

Si no puedes utilizar WSL2:

1. Descarga e instala **MSYS2** desde [msys2.org](https://www.msys2.org/).
2. Abre la terminal **MSYS2 MINGW64** y actualiza los repositorios:
   ```bash
   pacman -Syu
   pacman -S base-devel mingw-w64-x86_64-toolchain git python
   ```
3. Clona y compila el toolchain siguiendo las instrucciones de `ps3toolchain` adaptando los prefijos a rutas de Windows sin espacios (ej. `C:/ps3dev`).

---

## 4. Herramientas Nativas de Windows para el Proyecto

### 4.1. Emulador RPCS3 (Depuración y Pruebas):
* Descarga la versión oficial de RPCS3 para Windows.
* Configura la carpeta de almacenamiento de datos en:
  `%APPDATA%\rpcs3\dev_hdd0\game\BLES01976\USRDIR\`
* **Habilitar GDB Server:** `Settings > Debug > Enable GDB Server` (Escucha en `127.0.0.1:28011`).

### 4.2. RenderDoc (Inspección de Shaders y VBOs):
* Instala **RenderDoc** para Windows.
* Lanza el ejecutable `rpcs3.exe` desde RenderDoc utilizando el backend **Vulkan**.
* Captura un frame mientras se ejecuta Minecraft PS3 para inspeccionar los draw calls y buffers RSX.

### 4.3. Herramientas de Modelado y Python:
* Instala **Python 3.10+** desde la Microsoft Store o `python.org` (asegúrate de marcar *"Add Python to PATH"*).
* Ejecuta los scripts del conversor y del roadmap directamente en PowerShell:
  ```powershell
  python tools\generate_status_svg.py
  ```
* **Blockbench / MagicaVoxel:** Ambos cuentan con ejecutables nativos `.exe` para Windows para crear y exportar modelos.
