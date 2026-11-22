# Entorno de Compilación, Depuración y Telemetría

Este documento detalla la configuración del toolchain de desarrollo, el entorno de emulación y depuración en **RPCS3**, la integración con hardware real (PS3 CFW/HEN) y la arquitectura de telemetría por red.

---

## 1. Toolchain de Compilación cruzada (Cross-Compilation)

### Opciones de Toolchain:
1. **PSL1GHT v2 / ps3dev (Open Source - Recomendado):**
   * GCC 7.2.0 PPU con target `ppu-lv2`.
   * Herramientas de empaquetado: `sprx.ld`, `ppu-lv2-prxgen`.
2. **Sony Cell SDK oficial (SNC / ProDG):**
   * Compilador de alto rendimiento `sncPPU.exe` con optimizaciones de microcódigo específicas para Cell.

### Flags de Compilación Obligatorios:
```makefile
PPU_CC       = ppu-lv2-gcc
PPU_CXX      = ppu-lv2-g++
PPU_PRXGEN   = ppu-lv2-prxgen

CFLAGS   = -O2 -mcpu=cell -m32 -fno-strict-aliasing -fno-common -Wall
CXXFLAGS = $(CFLAGS) -fno-rtti -fno-exceptions -fno-threadsafe-statics
INCLUDES = -Iinclude -I$(PS3DEV)/portlibs/ppu/include

# Enlace de bibliotecas del sistema
LIBS     = -lgcm_cmd -lsysmodule -lnet -lsysutil -lsysfs
```

---

## 2. Flujo de Depuración Rápida en Emulador (RPCS3)

RPCS3 ofrece un entorno determinista para depuración sin riesgo de congelamiento ni tiempos muertos de reinicio en hardware físico.

```
 [ Código Fuente C++ ] ──► [ Compilación .sprx ]
                                  │
                   ┌──────────────┴──────────────┐
                   ▼                             ▼
       [ RPCS3 GDB Stub ]               [ RenderDoc (Vulkan) ]
       • Inspección paso a paso         • Captura frame por frame
       • Breakpoints en el Hook PPU     • Inspección de VBOs y Draw Calls RSX
       • Validación de matrices MVP     • Verificación de Culling y Shaders
```

### Configuración del GDB Stub en RPCS3:
1. En RPCS3: `Configuración > Debug > Enable GDB Server` (Puerto por defecto `28011`).
2. Conectar depurador desde la terminal de desarrollo:
   ```bash
   gdb-multiarch -ex "target remote localhost:28011"
   ```
3. Establecer breakpoints en la dirección del hook para verificar el flujo de llamada.

### Análisis Gráfico con RenderDoc:
1. Configurar RPCS3 para usar el backend gráfico **Vulkan**.
2. Lanzar RPCS3 a través de **RenderDoc**.
3. Capturar un frame en el momento en que el jugador sostiene un ítem vóxel.
4. Validar:
   * Que el draw call (`vkCmdDrawIndexed`) coincida con el recuento de índices del archivo `.vx3`.
   * El layout del buffer de vértices (posiciones, normales y UVs).
   * La matriz de proyección y transformación en las constantes de uniforms.

---

## 3. Despliegue en Hardware Real (PS3 CFW / HEN)

### Entorno de la Consola:
* **Firmware:** Evilnat 4.90 / 4.91 Cobra 8.4 (CEX / PEX / DEX) o PS3HEN.
* **Módulos de Carga:** Cobra Plugin Manager o webMAN MOD.

### Despliegue Automatizado por FTP:
Para no manipular pendrives USB entre cada compilación, el `Makefile` puede sincronizar directamente con la IP de la PS3:

```makefile
PS3_IP = 192.168.1.50
TARGET_PATH = /dev_hdd0/game/BLES01976/USRDIR/voxel3d

deploy: $(TARGET).sprx
	@echo "Enviando plugin por FTP a $(PS3_IP)..."
	curl -T $(TARGET).sprx ftp://$(PS3_IP):21$(TARGET_PATH)/
	@echo "Despliegue completado."
```

### Inyección Dinámica del Módulo:
El plugin puede ser cargado dinámicamente mediante el comando de webMAN MOD sin reiniciar la consola:
`GET http://<PS3_IP>/install.ps3<TARGET_PATH>/Voxel3D-PS3.sprx`

---

## 4. Telemetría y Logging por Red (NetLogger)

En un juego comercial en ejecución (`EBOOT.BIN`), las llamadas estándar `printf()` o `puts()` se descartan silenciosamente hacia `/dev/null`.

Para depurar variables en tiempo real en la consola física sin congelamientos:

### Implementación del NetLogger UDP en C++:
```cpp
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdarg.h>

static int s_log_socket = -1;
static struct sockaddr_in s_dev_pc_addr;

void netlog_init(const char* pc_ip, uint16_t port) {
    s_log_socket = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&s_dev_pc_addr, 0, sizeof(s_dev_pc_addr));
    s_dev_pc_addr.sin_family = AF_INET;
    s_dev_pc_addr.sin_port = htons(port);
    inet_pton(AF_INET, pc_ip, &s_dev_pc_addr.sin_addr);
}

void netlog_printf(const char* fmt, ...) {
    if (s_log_socket < 0) return;
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    if (len > 0) {
        sendto(s_log_socket, buffer, len, 0, 
               (struct sockaddr*)&s_dev_pc_addr, sizeof(s_dev_pc_addr));
    }
}
```

### Script Receptor en Python (PC Host):
Un script simple en la máquina del desarrollador escucha los mensajes transmitidos en tiempo real:

```python
import socket

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("0.0.0.0", 3000))
print("[Voxel3D NetLogger] Escuchando logs de PS3 en puerto 3000...")

while True:
    data, addr = sock.recvfrom(1024)
    print(f"[{addr[0]}] {data.decode('utf-8', errors='ignore')}", end="")
```
Esto permite capturar eventos de carga de modelos, fallos de asignación en VRAM y coordenadas de matrices de renderizado con cero impacto perceptible en el framerate del juego.
