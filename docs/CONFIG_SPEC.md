# Especificación de Configuración Data-Driven (Items & Guns)

Para evitar recompilar el plugin `.sprx` cada vez que se agrega o ajusta un modelo, `Voxel3D-PS3` utiliza un pipeline de configuración data-driven basado en un archivo JSON ubicado en el almacenamiento interno de la consola.

---

## 1. Ubicación y Ciclo de Vida en PlayStation 3

### Rutas en el Sistema de Archivos (`CellFS`):
* **Ruta Principal del Juego:**
  `/dev_hdd0/game/<TITLE_ID>/USRDIR/voxel3d/config.json`
* **Ruta Alternativa / Compartida:**
  `/dev_hdd0/plugins/voxel3d/config.json`

### Ciclo de Carga:
1. El plugin lee el archivo durante `voxel3d_prx_start()` utilizando la API nativa `cellFsOpen` / `cellFsRead`.
2. Los datos se parsean mediante una implementación ligera de JSON en C (diseñada para no exceder el presupuesto de pila del hilo, normalmente 64 KB).
3. Se genera una tabla estática indexada por `(item_id, metadata)` en la memoria XDR (consumo estimado: $< 64\text{ KB}$ para 256 armas).

---

## 2. Esquema JSON de Configuración

```json
{
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "version": 1,
  "plugin": "Voxel3D-PS3",
  "settings": {
    "debug_draw_muzzle": false,
    "global_recoil_intensity": 1.0,
    "interpolation_mode": "cubic_lerp"
  },
  "items": [
    {
      "name": "AK-47 Voxel Rifle",
      "item_id": 290,
      "metadata": 0,
      "model_path": "/dev_hdd0/game/BLES01976/USRDIR/voxel3d/models/ak47.vx3",
      "transform": {
        "scale": [0.0625, 0.0625, 0.0625],
        "rest": {
          "position": [0.25, -0.20, 0.45],
          "rotation": [-5.0, -15.0, 5.0]
        },
        "ads": {
          "position": [0.0, -0.14, 0.28],
          "rotation": [0.0, 0.0, 0.0],
          "fov_multiplier": 0.70
        }
      },
      "recoil": {
        "kick_position": [0.0, 0.02, -0.08],
        "kick_rotation": [12.0, 1.5, -2.0],
        "recovery_speed": 16.0
      },
      "fx": {
        "muzzle_flash_pos": [0.0, 0.05, 0.65],
        "sound_event": "random.explode",
        "sound_volume": 1.0,
        "sound_pitch": 1.8
      }
    }
  ]
}
```

---

## 3. Diccionario de Parámetros

### 3.1. Identificación y Modelo
* `item_id`: ID numérico del ítem de Minecraft al que se le sobreescribirá el renderizado (ej. `290` = Azadón de madera, ideal para reemplazar con fusiles).
* `metadata`: Sub-ID o daño específico. Un valor de `-1` actúa como comodín (*wildcard*).
* `model_path`: Ruta absoluta al archivo binario `.vx3` generado por el conversor offline.

### 3.2. Transformaciones Espaciales (`transform`)
* `scale`: Factor de escala tridimensional $(x, y, z)$ aplicado al modelo en relación con la cuadrícula de bloques de Minecraft (normalmente `0.0625` equivale a $1/16$, es decir, 1 vóxel = 1 píxel de bloque estándar).
* `rest`: Posición $(x, y, z)$ y rotación en grados $(\text{Pitch}, \text{Yaw}, \text{Roll})$ cuando el arma está en reposo sostenida en la mano derecha.
* `ads`: Posición y rotación cuando el jugador mantiene presionado el gatillo izquierdo (L2 / apuntado). El arma se alinea con el centro de la pantalla $(X=0)$.
* `fov_multiplier`: Factor de zoom de la cámara en modo ADS ($0.70 = 70\%$ del FOV original).

### 3.3. Dinámica de Retroceso (`recoil`)
* `kick_position`: Impulso instantáneo de traslación aplicado al frame en que se detecta el disparo. (Valores negativos en $Z$ empujan el arma hacia la cámara).
* `kick_rotation`: Impulso angular instantáneo. Los valores positivos en $X$ levantan el cañón (*muzzle climb*).
* `recovery_speed` ($\lambda$): Coeficiente de retorno exponencial. Valores más altos generan una recuperación más rápida y seca.

$$P_{actual} = P_{target} + (P_{current} - P_{target}) \cdot e^{-\lambda \cdot \Delta t}$$

### 3.4. Efectos Visuales y Sonoros (`fx`)
* `muzzle_flash_pos`: Coordenadas relativas al pivote del arma donde se renderizará el destello de disparo y desde donde nacerán los proyectiles o partículas de humo.
* `sound_event`: Nombre del evento de audio nativo de Minecraft invocado al disparar.

---

## 4. Estructura Interna en C++ para Almacenamiento Rápido

Para garantizar búsquedas en tiempo constante $O(1)$ sin asignaciones de memoria dinámicas durante el bucle de renderizado:

```cpp
#pragma pack(push, 4)
struct VoxelItemConfig {
    uint32_t item_id;
    int32_t  metadata;
    char     model_path[128];
    float    scale[3];
    float    rest_pos[3];
    float    rest_rot[3];
    float    ads_pos[3];
    float    ads_rot[3];
    float    ads_fov;
    float    recoil_kick_pos[3];
    float    recoil_kick_rot[3];
    float    recoil_recovery;
    float    muzzle_pos[3];
    uint32_t model_slot_index; // Índice hacia el pool de VRAM
    bool     active;
};
#pragma pack(pop)
```
Al arrancar, el plugin precarga los archivos `.vx3` correspondientes a cada ítem activo y asigna `model_slot_index` para que el hook de dibujo no requiera consultas de disco ni de cadenas de texto.
