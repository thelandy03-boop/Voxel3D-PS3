# MOD M4A1 para Minecraft PS3 (BLES01976)

**Estado: COMPLETO.** SPRX compilado con el toolchain nativo de Windows, validado
con `prx-gen check` y listo para copiar a la consola. El `EBOOT.BIN` firmado no
se ha tocado.

## Estado final

| Pieza | Estado |
|---|---|
| Carpetas del proyecto | Listas (`tools/`, `sprx/include/`, `sprx/src/`, `build_ps3/assets/`) |
| `M4A1.obj` -> `m4a1.vx3` | Listo: **1742 vértices / 3492 índices / 27904 bytes** |
| Formato de vértice | `>3h3b3B` = **12 bytes**, Big-Endian (`VX3D v1`) |
| Offset del hook | `0x005DA890` (prólogo verificado con capstone) |
| Fuentes SPRX | Listos (`Makefile`, `offsets.h`, `vx3_format.h`, `hook.cpp`, `main.cpp`, `config.h`, `hook.h`) |
| Carga RSX | `CELL_GCM_LOCATION_LOCAL` + `cellGcmSetVertexDataArray` + `cellGcmSetDrawIndexArray` |
| ADS / retroceso | L2 y R2 con LERP usando los valores de `config.json` |
| Compilación del `.sprx` | **Hecha**: `sprx\voxel3d.sprx` (firmado, 75.808 B) y `sprx\voxel3d.fake.sprx` (fake-signed), con el fix de registros del trampolín |
| Validación del módulo | `prx-gen check` -> **OK** (1 PT_LOAD, 2392 relocaciones, 3 librerías importadas) |
| Paquete de instalación | `PS3_INSTALL\USRDIR\voxel3d\` listo para copiar |
| Carga automática en RPCS3 | **Hecha**: `patches\imported_patch.yml` (patch engine v1.2, `calloc`) |
| Firma / repack del `EBOOT.BIN` | **No hacer**: el cargador es un parche en memoria de RPCS3 |

## Compilación (reproducible)

El SDK usado es `ps3-sdk-v0.28.0-windows-x86_64`, extraído en `C:\ps3sdk`
(quitar la carpeta raíz del ZIP `D:\Descargas de OperaGX\ps3-sdk-v0.28.0-windows-x86_64.zip`).
Incluye `ppu-g++` (GCC 16.1.0), `make` 4.4.1, `prx-gen`, `make_sprx` y `fself`.

```bat
cmd /c "D:\Descargas de OperaGX\MOD_M4A1\COMPILAR.bat"
```

También basta con hacer doble clic en `COMPILAR.bat`. El script posiciona el
PATH con `%PS3DK%\bin;%PS3DK%\ppu\bin;%PS3DK%\make\bin` y ejecuta
`make.exe -C sprx`. Para limpiar y reconstruir desde cero:

```bat
"C:\ps3sdk\make\bin\make.exe" -C "D:\Descargas de OperaGX\MOD_M4A1\sprx" clean
cmd /c "D:\Descargas de OperaGX\MOD_M4A1\COMPILAR.bat"
```

Pipeline: `ppu-g++` -> `ppu-objcopy` (asset embebido) -> `prx-gen build` ->
`prx-gen check` -> `make_sprx` (SELF firmado) / `fself` (fake-signed).
`sprxlinker.exe` del SDK no interviene: en este toolchain es post-link de SELF
de aplicación, no de módulos PRX.

Salidas:

```
sprx\voxel3d.elf         sin firmar, con relocaciones
sprx\voxel3d.prx         módulo Lv-2 validado por prx-gen
sprx\voxel3d.sprx        SELF firmado (75.808 bytes)
sprx\voxel3d.fake.sprx   variante fake-signed (≈444 KB)
```

## Instalación en consola

Copiar el contenido de `PS3_INSTALL\USRDIR\voxel3d\` a:

```
/dev_hdd0/game/BLES01976/USRDIR/voxel3d/
├── config.json
├── models/
│   └── m4a1.vx3
└── voxel3d.sprx
```

- `voxel3d.sprx` es la variante por defecto. Si RPCS3 / CFW / HEN rechaza la
  firma, usar `alternativo\voxel3d.fake.sprx` renombrado a `voxel3d.sprx`.
- La malla va **embebida** dentro del PRX (objcopy), así que el juego no depende
  de leer el `.vx3` en caliente. Se incluye `models\m4a1.vx3` y `config.json`
  para mantener el esquema data-driven (`item_id` 290, `metadata` -1).
- El `EBOOT.BIN` firmado **no se toca**. En RPCS3 el módulo se carga solo
  mediante `patches\imported_patch.yml` (ver sección siguiente). En consola
  hace falta un plugin loader de CFW/HEN que llame a `sys_prx_load_module`.

## Carga automática en RPCS3 (ya instalada)

Se creó `D:\rpcs3-v0.0.43-20644-3bfac607_win64_msvc\patches\imported_patch.yml`
(copia en `patches\imported_patch.yml` de este proyecto) con el motor de parches
v1.2 de RPCS3:

- `Version: 1.2`, claves `PPU-<sha1>` / `BLES01976-PPU-<sha1>` del `EBOOT.elf`.
- `[ calloc, 0x2c8250, 100 ]`: intercepta la **primera función real** del juego
  (la tabla OPD en `e_entry = 0xbbe520` apunta a `0x2c8250`, TOC `0xbd7998`).
- Los 100 `be32` siguientes forman el code cave PPU: guarda LR/TOC/r3-r12/CTR/CR/XER,
  **construye la ruta con `lis`/`ori`/`stw` dentro del frame de pila**, ejecuta
  `sys_prx_load_module` (syscall 480) y `sys_prx_start_module` (syscall 481), restaura
  el estado y reemite la instrucción original `li r2,0`. RPCS3 coloca el branch de
  retorno a `0x2c8254`.
- Importante: **ninguna palabra del cave es ASCII**. La versión anterior guardaba
  el string `/dev_hdd0/...` como datos `be32` dentro del mismo rango y RPCS3 0.0.43
  lo decodificaba como PPU, muriendo con
  `Unsupported calloc instruction 0x4c455330 at 0xd80100` (`LES0` de `BLES01976`).
  Ese era el verdadero origen del cuelgue en "Applying PPU code 88%": no era el SPRX.
- El cave se genera ensamblando `loader_instr.s` con `ppu-as` (sin encodings a mano),
  y el YAML se reconstruye con `gen_yaml_fixed2.py`.
- La ruta cargada es `/dev_hdd0/game/BLES01976/USRDIR/voxel3d/voxel3d.sprx`.
- El mismo SPRX se copió tanto en `dev_hdd0\game\BLES01976\USRDIR\voxel3d\` como
  en `PS3_GAME\USRDIR\voxel3d\` del juego, porque RPCS3 redirige esa ruta virtual
  a la carpeta desde la que se arranca el título.

Además se creó `patch_config.yml` en la raíz de RPCS3 (y copia en `patches\`)
con `Enabled: true` para las 4 claves. En este build los parches vienen
desactivados por defecto y ese archivo es el que los habilita; no hay que
tocar nada en la GUI. Para desactivar el mod, poner `Enabled: false` o
renombrar `imported_patch.yml`.

## Diagnóstico rápido (logs)

El SPRX tiene el `printf` de diagnóstico activado temporalmente
(`#define VOX3_DEBUG_LOG 1` en `main.cpp`). Escribe estas líneas en
`D:\rpcs3-v0.0.43-20644-3bfac607_win64_msvc\log\RPCS3.log`:

```text
[M4A1] voxel3d.sprx loaded, installing hook at 0x005da890
[M4A1] render hook hit: r3=... r4=... r5=... r6=... r7=... r8=... r9=... r10=...
[M4A1] game gcm ctx global 0015d084 -> xxxxxxxx (local=xxxxxxxx)
[M4A1] upload_asset OK vtx=xxxxxxxx idx=xxxxxxxx
```

- Si no aparece la primera línea, el parche no se aplicó o el PRX no cargó.
  Comprueba que `patches\imported_patch.yml` sea el de `Auto-load M4A1 SPRX`
  (claves `PPU-a51d...`, `calloc` en `0x2c8250` con **100** palabras) y no el
  parche viejo `Auto load and start voxel3d.sprx before main` (ese usa `jumpf`
  y no sirve).
- Si el log muestra `Unsupported calloc instruction 0x........`, el YAML que se
  aplicó sigue siendo el viejo con el string ASCII inline: vuelve a copiar
  `patches\imported_patch.yml` del proyecto y reinicia RPCS3.
- Si aparece la primera pero no la segunda, el módulo cargó pero el hook no se
  ejecutó.
- Si aparece `game gcm ctx global ... -> 00000000`, RPCS3 no ha rellenado la
  global del contexto; revisa que el EBOOT no esté modificado y que el parche
  se esté aplicando antes de `cellGcmInit`.
- Si aparece `upload_asset FAILED`, el `.vx3` incrustado no coincide con las
  constantes `VX3_VERT_COUNT` / `VX3_INDEX_COUNT`.
- Si el contexto y el asset son válidos pero no se ve el arma, el problema está
  en el estado RSX (vertex format, shader, matriz, depth/blend), no en el item.

Tras sustituir `imported_patch.yml` o el SPRX, reinicia RPCS3 y vuelve a arrancar
el juego.

Nota: el trampolín guarda y restaura `r3..r10` y `f1` alrededor del handler, porque
el render original `0x005DBBD0` todavía los necesita. Sin esa restauración el juego
se congela al entrar en el hook.

Causa del "no aparece el arma" detectada el 2026-11-22: el handler usaba
`gCellGcmCurrentContext`, que en este SDK es un alias de la variable `gGcmContext`
**del propio PRX**. Como el plugin nunca llama a `cellGcmInit`, esa copia siempre
era `NULL` y el handler salía antes de dibujar. Ahora se lee el puntero real del
juego desde `0x0015D084` (`r2 - 0x2F7C` con `r2 = 0x00160000`), que es la global
que el wrapper de `cellGcmInit` en `0x005D27D0` pasa a `_cellGcmInitBody`.

## Regenerar el modelo

```bat
python tools\obj_to_vx3.py
```

Salida esperada:

```
OBJ: 518 caras -> 1164 triangulos
VX3: 1742 vertices (max 2500), 3492 indices (max 3750)
formato vertice: >3h3b3B = 12 bytes, Big-Endian
```

## Detalles técnicos del hook

- Entrada real: `0x005DA890` (`mflr r0`, `stdu r1,-0x70(r1)`, `std r0,0x80(r1)`).
- El parche sobrescribe 16 bytes con `lis/ori/mtctr/bctr`.
- El trampolín ejecuta el prólogo original (`mflr`, `stdu`, `std`), guarda
  `r0`/`r2`, carga el **OPD** del handler (`{code, toc, env}`), llama al plugin
  con su propio TOC, restaura el TOC del juego y reemite el `bl 0x005DBBD0`
  original como salto absoluto con `LR = 0x005DA8A0` (`target+16`). El `bl`
  original es PC-relativo y no se puede copiar tal cual al trampolín.
- Antes y después se ejecuta `dcbst` / `sync` / `icbi` / `isync` en cada línea
  de caché de 128 bytes.
- El módulo declara `SYS_MODULE_INFO` / `SYS_MODULE_START` / `SYS_MODULE_STOP`;
  instala el hook en `vox3_prx_start()` y lo revierte en `vox3_prx_stop()`.
- El PRX queda totalmente enlazado: no se usan símbolos weak sin resolver
  (por eso el parche se escribe con `memcpy`).
- Tiempo de frame con `sys_time_get_system_time()`, contexto GCM real leído desde
  la global del EBOOT `0x0015D084` y VRAM vía `cellGcmMapLocalMemory()`.

## Archivos del proyecto

```
D:\Descargas de OperaGX\MOD_M4A1\
├── COMPILAR.bat                  <- compilación en un paso con C:\ps3sdk
├── config.json                   <- item 290, metadata -1, ADS, recoil, FX
├── README.md
├── PS3_INSTALL\USRDIR\voxel3d\   <- paquete listo para copiar a la PS3
├── patches\
│   ├── imported_patch.yml        <- cargador SPRX automático para RPCS3
│   └── patch_config.yml          <- Enabled: true para las 4 claves PPU
├── tools\
│   ├── obj_to_vx3.py             <- OBJ -> VX3 Big-Endian (12 bytes/vert)
│   ├── patch_ghidramcp_port.py   <- parche del puerto 18080 (GhidraMCP)
│   ├── find_prologue.py          <- localizó 0x005DA890 con capstone
│   └── scan_const_refs.py        <- refs a -0.40f / -0.72f / 0.56f / -0.52f / 60.0f
├── build_ps3\assets\m4a1.vx3     <- 27904 bytes, header VX3D v1
└── sprx\
    ├── Makefile                  <- PRX native SDK + prx-gen + firma
    ├── voxel3d.sprx / .fake.sprx <- artefactos generados
    ├── include\
    │   ├── config.h
    │   ├── hook.h
    │   ├── offsets.h
    │   ├── voxel3d.h
    │   └── vx3_format.h
    └── src\
        ├── hook.cpp
        └── main.cpp
```
