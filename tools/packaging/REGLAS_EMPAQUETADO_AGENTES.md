# REGLAS OBLIGATORIAS DE EMPAQUETADO PARA AGENTES DE IA

> **ATENCIÓN A CUALQUIER AGENTE / MODELO DE IA:**
> Este documento establece el procedimiento **ESTRICTAMENTE OBLIGATORIO** para compilar y empaquetar archivos `.pkg` en este repositorio.
> **NUNCA** intentes generar el binario del `.pkg` mediante scripts manuales o improvisados en Node.js, C o Python simple, ya que RPCS3 y las consolas PS3 rechazan paquetes sin el cifrado AES-128-CTR oficial de Sony NPDRM y su estructura estricta.

---

## 1. El Motor Oficial Obligatorio: PyKG

El repositorio cuenta con el motor oficial de empaquetado **PyKG** ubicado en:
`tools/packaging/PyKG`

Para automatizarlo vía CLI sin interfaz gráfica, se ha programado el conector directo:
`tools/packaging/build_pkg_pykg.py`

### Cómo ejecutarlo:
```powershell
& "C:\Users\grezu\AppData\Local\Programs\Python\Python313\python.exe" tools\packaging\build_pkg_pykg.py
```
O simplemente ejecutando:
```cmd
tools\packaging\empaquetar.bat
```

---

## 2. Parámetros Obligatorios del Paquete

| Parámetro | Valor Requerido |
| :--- | :--- |
| **Title ID** | `MCPS3MODS` |
| **Content ID** | `UP0001-MCPS3MODS_00-0000000000000000` |
| **Package Type** | `Game_Exec` (Type 5 / Homebrew) |
| **DRM Type** | `Free` (DRM 3) |
| **Source Folder** | `app_ps3/pkgfiles` |
| **Output PKG** | `Minecraft_PS3_Mod_Loader_v1.4.2.pkg` |

---

## 3. Estructura de `app_ps3/pkgfiles`

Antes de empaquetar, asegúrate de que contenga:
1. `PARAM.SFO`: Título y metadatos actualizados (`TITLE_ID=MCPS3MODS`).
2. `ICON0.PNG`: Icono para el XMB / RPCS3 (320x176 o similar).
3. `PIC1.PNG`: Fondo panorámico 1920x1080 (Minecraft Sunset).
4. `USRDIR/`: Directorio con los plugins `.sprx` y configuraciones.
   - `USRDIR/voxel3d/config.json`: Lista de mods habilitados y velocidad.
