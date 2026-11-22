# Arquitectura del Sistema: Voxel3D-PS3

Este documento detalla el diseño técnico del pipeline de renderizado vóxel 3D e intercepción para PlayStation 3.

---

## 1. El Reto de Rendimiento en el RSX

La GPU RSX (basada en NV47/G70) cuenta con 256 MB de memoria GDDR3 dedicada conectada a través de un bus de 128 bits. Si bien tiene un ancho de banda considerable, presenta una latencia notable al comunicarse con la memoria principal XDR del Cell a través de FlexIO.

* **Objetivo de diseño:** Cero transferencias PPU-RSX en caliente durante el frame loop.
* **Estrategia:** Todos los modelos vóxel precalculados se cargan una sola vez al inicializar el plugin o al cargar el mapa, alojándose en VRAM local (`CELL_GCM_LOCATION_LOCAL`).

---

## 2. Intercepción del Hook (PPU)

El motor de Minecraft: PlayStation 3 Edition (4J Studios) ejecuta el bucle de renderizado en el hilo principal del PPU.

### Punto de Hook:
* Función objetivo: `ItemRenderer::renderItemInFirstPerson(EntityPlayer* player, float partialTicks)`
* **Mecanismo:** Hook tipo *trampoline* o sustitución de vtable/dirección de función en el binario descifrado en memoria.
* **Lógica del Hook:**
  1. Inspeccionar el ítem sostenido en la mano derecha (`player->inventory->getCurrentItem()`).
  2. Verificar si el ID o metadata del ítem corresponde a un modelo vóxel 3D registrado.
  3. **Si no coincide:** Invocar la función original (*trampoline*) para conservar el comportamiento por defecto (renderizado de bloques o sprites 2D extruidos).
  4. **Si coincide:**
     * Suprimir el renderizado 2D original.
     * Recuperar la matriz de transformación del brazo (`RenderPlayer::renderFirstPersonArm`).
     * Aplicar la matriz de compensación + retroceso (*recoil offset*).
     * Enviar el comando de dibujo al RSX mediante libgcm (`cellGcmSetDrawIndexArray`).

---

## 3. Lógica Matemática de Retroceso y Apuntado (Recoil LERP)

Para recrear la sensación de *MrCrayfish's Gun Mod*:

* Cada disparo genera un impulso de retroceso instantáneo en traslación Z y rotación Pitch/Yaw.
* Se utiliza una función de interpolación cúbica / suave hacia el estado de reposo:

$$P_{actual} = \text{lerp}(P_{recoil}, P_{rest}, \Delta t \cdot \lambda_{recovery})$$

* Cuando el jugador apunta (ADS - *Aim Down Sights*), la matriz del modelo se traslada suavemente al centro de la cámara (FOV dinámico e interpolación lineal de posición).
