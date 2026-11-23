# Especificación del Transpilador de Mods: jar2sprx (Java a C++ / PS3 SPRX)

## 1. Visión General
El objetivo de **`jar2sprx`** es permitir que mods clásicos de Minecraft Java Edition (empaquetados en archivos `.jar`, típicamente de versiones como 1.6.4 / 1.7.10) puedan ser convertidos de forma automática en mods nativos en C++ compilados como módulos dinámicos `.sprx` para Minecraft PlayStation 3 Edition (LCE - 4J Studios).

Dado que la PS3 cuenta únicamente con 256 MB de memoria RAM XDR y una arquitectura Cell PPU/RSX sin JVM, la traducción directa a código nativo en C++ garantiza un rendimiento fluido a 60 FPS sin sobrecarga de memoria ni Garbage Collector.

---

## 2. Anatomía del Pipeline de Traducción

```
                   ┌───────────────────────────────────┐
                   │           Mod.jar (Java)          │
                   └─────────────────┬─────────────────┘
                                     │
                                     ▼
                      [ Fase 1: Extractor de Recursos ]
                     - Texturas (.png) -> DDS / PS3 GTF
                     - Modelos JSON/Java -> .vx3 Voxel
                     - Archivos de idioma (en_US.lang)
                                     │
                                     ▼
                      [ Fase 2: Analizador de Bytecode ]
                     - Desensambla clases de ítems/bloques
                     - Detecta propiedades (daño, stack, texturas)
                     - Extrae eventos de interacción (onRightClick, etc.)
                                     │
                                     ▼
                      [ Fase 3: Transpilador a C++ ]
                     - Genera clases C++ heredadas de `ForgeShim`
                     - Traduce tipos Java (String -> std::string, etc.)
                     - Genera `config.json` para Voxel3D
                                     │
                                     ▼
                      [ Fase 4: Compilador Toolchain PS3 ]
                     - Compila el C++ generado con GCC PPU
                     - Enlaza contra `modloader.sprx`
                                     │
                                     ▼
                   ┌───────────────────────────────────┐
                   │             Mod.sprx              │
                   │    (Listo para RPCS3 y PS3)       │
                   └───────────────────────────────────┘
```

---

## 3. Capa de Compatibilidad C++ (Forge Shim)

Para que el código transpilado compile en C++, se proporciona un conjunto de cabeceras C++ que replican la API de Forge (`cpw.mods.fml` y `net.minecraft`):

```cpp
// include/forge/Item.h
namespace net {
namespace minecraft {
namespace item {

class Item {
public:
    int itemId;
    std::string unlocalizedName;
    int maxStackSize;

    Item(int id) : itemId(id), maxStackSize(64) {}
    virtual ~Item() {}

    virtual Item* setUnlocalizedName(const std::string& name) {
        this->unlocalizedName = name;
        return this;
    }

    virtual bool onItemUse(...) { return false; }
};

}}}
```

---

## 4. Clasificación de Mods Soportados

| Tipo de Mod | Nivel de Automatización | Ejemplos |
| :--- | :--- | :--- |
| **Armas y Herramientas** | 95% Automático | Balkon's WeaponMod, Mo' Swords, Guns Mods |
| **Bloques y Decoración** | 90% Automático | Furniture Mod, DecoCraft básico, Custom Ores |
| **Comidas y Consumibles** | 95% Automático | Pam's HarvestCraft (ítems), Simple Foods |
| **Mecánicas Complejas / Dimensiones** | Semi-automático (Requiere hooks manuales) | Twilight Forest, Aether, IndustrialCraft |

---

## 5. Salida Estructurada para Voxel3D-PS3
Para cada mod procesado, `jar2sprx` genera:
1. `models/<item_name>.vx3`: El modelo tridimensional optimizado para el RSX.
2. `config.json`: Mapeo de IDs dentro del motor de Minecraft PS3.
3. `mod.cpp` y `mod.sprx`: La lógica nativa de ejecución.
