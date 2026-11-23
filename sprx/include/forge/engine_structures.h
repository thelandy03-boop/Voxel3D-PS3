#ifndef ENGINE_STRUCTURES_H
#define ENGINE_STRUCTURES_H

#include <stdint.h>
#include <stdbool.h>

#pragma pack(push, 4)

/**
 * Representación en memoria de un ítem en inventario / mano activa (4J Studios C++)
 */
class ItemStack {
public:
    uint32_t m_itemId;         // Offset 0x00: ID numérico del ítem (ej. 256 = Pala, 290 = Azada/Fusil)
    uint32_t m_stackSize;      // Offset 0x04: Cantidad en el stack
    uint32_t m_itemDamage;     // Offset 0x08: Durabilidad restante o metadatos
    void*    m_pTagCompound;   // Offset 0x0C: Puntero a NBT
    uint32_t m_animationsToGo; // Offset 0x10: Animación de recolección

    inline bool isValid() const { return m_itemId > 0 && m_stackSize > 0; }
    inline uint32_t getId() const { return m_itemId; }
    inline uint32_t getDamage() const { return m_itemDamage; }
};

/**
 * Inventario del jugador
 */
class InventoryPlayer {
public:
    void*      m_vtable;
    ItemStack* m_mainInventory[36];
    ItemStack* m_armorInventory[4];
    uint32_t   m_currentItemIndex;
    void*      m_pPlayerOwner;

    inline ItemStack* getCurrentItem() {
        if (m_currentItemIndex >= 9) return nullptr;
        return m_mainInventory[m_currentItemIndex];
    }
};

/**
 * Entidad de jugador local
 */
class EntityPlayer {
public:
    void*            m_vtable;
    uint32_t         m_entityId;
    double           m_posX;
    double           m_posY;
    double           m_posZ;
    float            m_rotationYaw;
    float            m_rotationPitch;
    float            m_prevRotationYaw;
    float            m_prevRotationPitch;
    uint8_t          _pad0[0x80];
    InventoryPlayer* m_pInventory;
};

/**
 * Renderizador de ítems y brazos en primera persona
 */
class ItemRenderer {
public:
    void*        m_vtable;
    void*        m_pMinecraft;
    ItemStack*   m_pItemToRender;        // Offset 0x08: Ítem sostenido actualmente
    float        m_equippedProgress;     // Offset 0x0C: Progreso de animación equipar [0..1]
    float        m_prevEquippedProgress; // Offset 0x10
};

#pragma pack(pop)

#endif // ENGINE_STRUCTURES_H
