# Pipeline de Creación de Mallas Vóxel (Asset Pipeline)

Esta guía define las restricciones técnicas, convenciones de ejes y presupuestos geométricos para modeladores 3D y artistas que creen armas o ítems vóxel destinados a **Voxel3D-PS3**.

---

## 1. Software de Modelado Soportado

* **Blockbench:** Ideal para modelado modular con cubos y exportación a JSON / `.bbmodel`.
* **MagicaVoxel:** Herramienta recomendada para vóxeles puros con exportación nativa a archivo binario `.vox`.

---

## 2. Convención de Ejes y Punto de Pivote (Origen)

Para que el modelo se acople de manera correcta a la mano derecha del jugador en primera persona sin requerir ajustes manuales en cada arma:

```
        +Y (Arriba)
         │
         │     ┌───────────────┐ (Cañón / Muzzle)
         │     │               ├──────► +Z (Dirección de disparo)
         │  ┌──┴───────────────┤
         │  │  (Empuñadura)    │
         └──┼──────────────────┴──────► +X (Lateral derecho)
          (0,0,0) Pivote de Mano
```

* **Punto de Pivote $(0,0,0)$:** Debe situarse exactamente en el centro de la empuñadura del arma (el punto donde los dedos del modelo del jugador sostienen el mango).
* **Eje $+Z$:** Dirección del cañón hacia adelante (línea de visión del disparo).
* **Eje $+Y$:** Dirección vertical hacia arriba (miras, cañón superior).
* **Eje $+X$:** Lateral derecho del arma.

---

## 3. Presupuesto Geométrico y Escala

La GPU RSX opera bajo límites estrictos de memoria GDDR3 y ancho de banda del command buffer.

### 3.1. Factor de Escala de Vóxeles:
* La cuadrícula de Minecraft se basa en cubos de $16 \times 16 \times 16$ unidades.
* **Factor de conversión:** $1\text{ vóxel} = 0.0625\text{ unidades de mundo}$ ($1/16$).
* Un modelo de $32$ vóxeles de largo equivaldrá exactamente a $2$ bloques de distancia en el espacio del juego.

### 3.2. Presupuestos por Tipo de Arma:

| Categoría | Dimensiones Máximas (Vóxeles) | Triángulos Máximos | Tamaño Estimado `.vx3` |
| :--- | :--- | :--- | :--- |
| **Arma Corta (Pistola/Revólver)** | $24 \times 16 \times 6$ | $\le 1,000$ | $10 - 15\text{ KB}$ |
| **Subfusil / Escopeta Corta** | $36 \times 18 \times 8$ | $\le 1,800$ | $15 - 22\text{ KB}$ |
| **Fusil de Asalto / Francotirador** | $48 \times 20 \times 8$ | $\le 2,500$ | $20 - 32\text{ KB}$ |
| **Ítems Especiales / Herramientas** | $20 \times 20 \times 6$ | $\le 1,200$ | $12 - 18\text{ KB}$ |

---

## 4. Reglas de Color y Materiales

1. **Vóxeles Opacos Obligatorios:**
   * No utilizar vóxeles con valores de alfa intermedios (semitransparentes como vidrio coloreado).
   * El renderizado en RSX se realiza en una sola pasada de dibujo indexado (`cellGcmSetDrawIndexArray`) con descarte binario de alfa (`alpha test > 0.1`). Las semitransparencias requieren ordenamiento de atrás hacia adelante en la CPU, lo cual degradaría severamente el rendimiento del PPU.

2. **Paleta de Color:**
   * En MagicaVoxel, utilizar la paleta de 256 colores estándar o la paleta indexada del atlas de Minecraft (`terrain.png`).
   * El conversor compacta los índices de color y coordenadas UV en el atributo de 2 bytes (`u, v`) del vértice.
