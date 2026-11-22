@echo off
setlocal
cd /d "%~dp0"

set "PS3DK=C:\ps3sdk"
set "PATH=%PS3DK%\bin;%PS3DK%\ppu\bin;%PS3DK%\make\bin;%PATH%"

echo ============================================
echo  Compilando voxel3d.sprx (M4A1 PS3 MOD)
echo  SDK: %PS3DK%
echo ============================================
echo.

if not exist "%PS3DK%\ppu\bin\ppu-g++.exe" (
    echo ERROR: no se encontro "%PS3DK%\ppu\bin\ppu-g++.exe"
    echo Extrae ps3-sdk-v0.28.0-windows-x86_64.zip en C:\ps3sdk
    pause
    exit /b 1
)

"%PS3DK%\make\bin\make.exe" -C sprx
set "RC=%ERRORLEVEL%"

echo.
if exist "sprx\voxel3d.sprx" (
    echo LISTO: sprx\voxel3d.sprx
    if exist "sprx\voxel3d.fake.sprx" echo LISTO: sprx\voxel3d.fake.sprx
) else (
    echo No se genero el SPRX. Revisa los errores de arriba.
)

if not "%RC%"=="0" exit /b %RC%
exit /b 0
