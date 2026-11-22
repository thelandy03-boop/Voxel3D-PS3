# Metodología de Ingeniería Inversa: Minecraft PS3 EBOOT.BIN

Guía para el análisis de símbolos, tablas virtuales y offsets en el ejecutable de Minecraft: PlayStation 3 Edition (4J Studios).

---

## 1. Preparación del Binario

1. **Descifrado de EBOOT.BIN:**
   * Utilizar `scetool` o `trueancestor` para descifrar el `EBOOT.BIN` de la versión objetivo (ej. BLES01976 / NPUB31419 / última actualización 1.84).
   * Obtener el archivo `EBOOT.ELF` desprotegido.

2. **Carga en Ghidra / IDA Pro:**
   * **Arquitectura:** PowerPC:BE:64:default o PowerPC:BE:32 (Cell PPU ejecuta modo 32-bit o 64-bit según el SDK; 4J Studios usa ABI de 32-bit de punteros en la mayoría de títulos PS3).
   * Cargar las tablas de tipos (PS3 SDK TIB/til).

---

## 2. Búsqueda de Símbolos y Cadenas Clave

Aunque los binarios comerciales de 4J Studios tienen símbolos recortados (*stripped*), existen cadenas de depuración y aserciones (*assert strings*):

* **Strings de búsqueda:**
  * `"ItemRenderer"`
  * `"renderItemInFirstPerson"`
  * `"renderFirstPersonArm"`
  * `"glPushMatrix"` / `"glPopMatrix"` (o las envolventes GCM equivalentes)
  * Referencias a texturas de ítems: `"textures/items/"` o atlas de terreno `"terrain.png"`

---

## 3. Identificación del Hook

1. Rastrear las funciones que acceden al inventario local del jugador local (`Minecraft::getMinecraft()->thePlayer`).
2. Aislar la bifurcación condicional donde el motor comprueba `Item::isFull3D()` o si el renderizador usa extrusión de quads mediante la rutina similar a `ItemRenderer::renderItem`.
3. Anotar el offset relativo a la base del ejecutable para inyectar el salto en el plugin `.sprx`.
