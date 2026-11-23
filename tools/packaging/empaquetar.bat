@echo off
setlocal
echo =======================================================
echo   EMPAQUETADOR OFICIAL PYKG PARA PS3 / RPCS3
echo =======================================================
echo.

set "PY313=C:\Users\grezu\AppData\Local\Programs\Python\Python313\python.exe"
if exist "%PY313%" (
    echo [*] Ejecutando motor PyKG via Python...
    "%PY313%" "%~dp0build_pkg_pykg.py"
    goto :fin
)

echo [*] Buscando Python en el sistema...
python "%~dp0build_pkg_pykg.py"
if %ERRORLEVEL% EQU 0 goto :fin

echo [!] Ejecutando interfaz grafica de PyKG.exe...
start "" "%~dp0PyKG\PyKG.exe"

:fin
echo.
echo =======================================================
echo   Proceso finalizado. El .pkg se encuentra en la raiz.
echo =======================================================
