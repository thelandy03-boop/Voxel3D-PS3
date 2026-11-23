# ==============================================================================
# Voxel3D-PS3 Master Makefile (Enterprise PS3 Project Architecture)
# ==============================================================================

.PHONY: all sprx app_ps3 clean help

all: sprx app_ps3

sprx:
	@echo "[*] Compilando Voxel3D Core SPRX..."
	@$(MAKE) -C sprx

app_ps3:
	@echo "[*] Compilando Minecraft PS3 Mod Loader (App PS3)..."
	@$(MAKE) -C app_ps3

clean:
	@echo "[*] Limpiando artefactos de compilacion..."
	@$(MAKE) -C sprx clean
	@$(MAKE) -C app_ps3 clean

help:
	@echo "Comandos disponibles:"
	@echo "  make all      - Compila el Core SPRX y la App Homebrew"
	@echo "  make sprx     - Compila solo voxel3d.sprx"
	@echo "  make app_ps3  - Compila solo la App Homebrew (EBOOT.BIN / PKG)"
	@echo "  make clean    - Elimina archivos temporales y binarios"
