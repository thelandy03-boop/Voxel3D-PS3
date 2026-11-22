# Estructuras Internas del Motor (4J Studios C++)

Este documento compila la ingeniería inversa de los layouts de memoria de las clases C++ principales de **Minecraft: PlayStation 3 Edition**, reconstruidas a partir de análisis estático en Ghidra/IDA Pro del binario desprotegido (`EBOOT.ELF`).

---

## 1. Características del Compilador y Modelo de Objetos

* **Compilador Original:** GCC PPU / SNC PPU (Sony Network Compiler) con optimización `-O2` o `-O3`.
* **RTTI:** Deshabilitado (`-fno-rtti`). Las clases polimórficas se identifican mediante sus tablas virtuales (`vtable`) ubicadas en la sección `.rodata`.
* **Alineación de Miembros:** Alineación natural estándar de 32 bits (punteros a 4 bytes, enteros primitivos alineados a su tamaño, tipos vectoriales `vec_float4` alineados a 16 bytes).

---

## 2. Reconstrucción de Clases del Motor

### 2.1. `ItemStack`
Representa una instancia de un ítem en un inventario o sostenido en la mano.

```cpp
#pragma pack(push, 4)
class ItemStack {
public:
    uint32_t m_itemId;         // Offset 0x00: ID numérico del ítem (ej. 256 = Pala de hierro)
    uint32_t m_stackSize;      // Offset 0x04: Cantidad de ítems en el slot
    uint32_t m_itemDamage;     // Offset 0x08: Metadatos o durabilidad restante
    void*    m_pTagCompound;   // Offset 0x0C: Puntero a estructura NBT (encantamientos, nombres)
    uint32_t m_animationsToGo; // Offset 0x10: Temporizador de animación de recolección
    
    // Métodos virtuales si aplica
    inline bool isValid() const { return m_itemId > 0 && m_stackSize > 0; }
    inline uint32_t getId() const { return m_itemId; }
    inline uint32_t getDamage() const { return m_itemDamage; }
};
#pragma pack(pop)
```

### 2.2. `InventoryPlayer`
Gestión de slots de inventario del jugador.

```cpp
#pragma pack(push, 4)
class InventoryPlayer {
public:
    void*      m_vtable;                // Offset 0x00
    ItemStack* m_mainInventory[36];     // Offset 0x04: Slots 0-8 (Hotbar), 9-35 (Inventario principal)
    ItemStack* m_armorInventory[4];     // Offset 0x94: Casco, Pechera, Pantalones, Botas
    uint32_t   m_currentItemIndex;      // Offset 0xA4: Índice activo de la hotbar (0 - 8)
    void*      m_pPlayerOwner;          // Offset 0xA8: Puntero a EntityPlayer
    
    inline ItemStack* getCurrentItem() {
        if (m_currentItemIndex >= 9) return nullptr;
        return m_mainInventory[m_currentItemIndex];
    }
};
#pragma pack(pop)
```

### 2.3. `EntityPlayer` / `EntityClientPlayerMP`
Instancia del jugador local en el espacio 3D.

```cpp
#pragma pack(push, 4)
class EntityPlayer {
public:
    void*            m_vtable;            // Offset 0x00
    uint32_t         m_entityId;          // Offset 0x04
    // Vectores de posición (coordenadas de mundo)
    double           m_posX;              // Offset 0x08
    double           m_posY;              // Offset 0x10
    double           m_posZ;              // Offset 0x18
    // Vectores de rotación de cámara
    float            m_rotationYaw;       // Offset 0x20
    float            m_rotationPitch;     // Offset 0x24
    float            m_prevRotationYaw;   // Offset 0x28
    float            m_prevRotationPitch; // Offset 0x2C
    
    uint8_t          _pad0[0x80];         // Datos intermedios de física/AABB
    
    InventoryPlayer* m_pInventory;        // Puntero al inventario del jugador
};
#pragma pack(pop)
```

### 2.4. `ItemRenderer`
Clase responsable de la vista de ítems y brazos en primera persona.

```cpp
#pragma pack(push, 4)
class ItemRenderer {
public:
    void*        m_vtable;                // Offset 0x00
    void*        m_pMinecraft;            // Offset 0x04: Puntero a la instancia global
    ItemStack*   m_pItemToRender;         // Offset 0x08: Ítem actualmente en transición de dibujo
    float        m_equippedProgress;      // Offset 0x0C: Progreso de equipamiento [0.0 - 1.0]
    float        m_prevEquippedProgress;  // Offset 0x10
    
    // Funciones miembro objetivo de hook
    virtual void renderItemInFirstPerson(EntityPlayer* player, float partialTicks);
    virtual void renderFirstPersonArm(EntityPlayer* player);
    virtual void renderItem(EntityPlayer* entity, ItemStack* item, uint32_t renderPass);
};
#pragma pack(pop)
```

---

## 3. Convención de Matrices y Transformaciones

El motor de 4J Studios implementa matrices de transformación en orden de columnas (*column-major*) `float[16]`.

```
       ┌                                    ┐
       │  m[0]   m[4]   m[8]   m[12] (Tx)   │
  M =  │  m[1]   m[5]   m[9]   m[13] (Ty)   │
       │  m[2]   m[6]   m[10]  m[14] (Tz)   │
       │  m[3]   m[7]   m[11]  m[15] (1.0)  │
       └                                    ┘
```

Durante la ejecución de `renderItemInFirstPerson`:
* El origen $(0,0,0)$ de la matriz local del ítem está referenciado al codo/muñeca del jugador.
* La mano derecha aplica una rotación base de aproximadamente $-45^\circ$ en Yaw y $+15^\circ$ en Roll para presentar el ítem en ángulo hacia el centro de la pantalla.

---

## 4. Tabla de Offsets por TitleID y Versión

*Nota: Los offsets se calculan con respecto a la dirección base de carga del ejecutable (`0x00010000`).*

| Title ID | Región | Versión / TU | Función Objetivo | Offset Relativo PPU |
| :--- | :--- | :--- | :--- | :--- |
| **BLES01976** | Europa (Disco) | Update 1.84 | `ItemRenderer::renderItemInFirstPerson` | *TBD (En reversing activo)* |
| **BLES01976** | Europa (Disco) | Update 1.84 | `ItemRenderer::renderFirstPersonArm` | *TBD (En reversing activo)* |
| **NPUB31419** | USA (Digital)  | Update 1.84 | `ItemRenderer::renderItemInFirstPerson` | *TBD (En reversing activo)* |
| **NPEB01899** | Europa (Digital)| Update 1.84 | `ItemRenderer::renderItemInFirstPerson` | *TBD (En reversing activo)* |

### Patrón de Búsqueda Binaria (AOB Signature):
Para localizar `renderItemInFirstPerson` independientemente de variaciones menores de compilación:

```
# Patrón de prólogo estándar PowerPC:
# mflr  r0
# stw   r0, 4(r1)
# stwu  r1, -framesize(r1)
# ... referencias a cadenas "renderItemInFirstPerson" o comprobación de ItemStack::isValid
```
