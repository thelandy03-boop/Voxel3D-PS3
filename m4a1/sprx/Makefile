# Makefile del plugin M4A1 -> voxel3d.sprx
# Toolchain nativo de Windows: ps3-sdk-v0.28.0-windows-x86_64 en C:\ps3sdk
#
#   make                 -> compila y genera voxel3d.sprx
#   make clean           -> limpia objetos y artefactos
#   make PS3DK=D:/ps3sdk -> usa otra ubicacion del SDK

PS3DK ?= C:/ps3sdk
export PS3DK

BIN     = $(PS3DK)/ppu/bin
HOSTBIN = $(PS3DK)/bin

CXX      = "$(BIN)/ppu-g++.exe"
OBJCOPY  = "$(BIN)/ppu-objcopy.exe"
PRXGEN   = "$(HOSTBIN)/prx-gen.exe"
MAKESPRX = "$(HOSTBIN)/make_sprx.exe"
FSELF    = "$(HOSTBIN)/fself.exe"

SHELL := cmd.exe

TARGET    = voxel3d.sprx
FAKE      = voxel3d.fake.sprx
ELF       = voxel3d.elf
PRX       = voxel3d.prx
OBJS      = src/main.o src/hook.o src/m4a1_vx3.o

ASSET_SRC = ..\build_ps3\assets\m4a1.vx3
ASSET     = assets\m4a1.vx3
ASSET_OBJ = src/m4a1_vx3.o

CXXFLAGS = -Wall -O2 -mcpu=cell -std=c++11 -fno-rtti -fno-exceptions \
           -fno-strict-aliasing -ffunction-sections -fdata-sections \
           -Iinclude -I$(PS3DK)/ppu/include

# Enlace Lv-2 PRX: relocaciones retenidas (-Wl,-q), sin crt de ejecutable,
# specs y script de PRX del SDK, y el crt especifico de modulo.
PRX_LDFLAGS = -specs=$(PS3DK)/ppu/lib/lv2-prx.specs \
              -nostartfiles -Wl,-q -Wl,--no-warn-rwx-segments \
              -Wl,-T,$(PS3DK)/ppu/lib/lv2-prx.ld \
              $(PS3DK)/ppu/lib/lv2-prx-crt.o \
              -L$(PS3DK)/ppu/lib -lgcm_cmd -lgcm_sys_stub -lio_stub \
              -lsysmodule -lc

ifeq ($(wildcard $(PS3DK)/ppu/bin/ppu-g++.exe),)
$(error No se encontro ppu-g++.exe en $(PS3DK)/ppu/bin)
endif

.PHONY: all clean

all: $(TARGET) $(FAKE)
	@echo Generado: $(TARGET) y $(FAKE)

$(ASSET):
	if not exist assets mkdir assets
	copy /Y $(ASSET_SRC) $(ASSET) >nul

$(ASSET_OBJ): $(ASSET)
	$(OBJCOPY) -I binary -O elf64-powerpc -B powerpc \
		--rename-section .data=.rodata,alloc,load,readonly,data,contents \
		$(ASSET) $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(ELF): $(OBJS)
	$(CXX) $(OBJS) $(PRX_LDFLAGS) -o $@

$(PRX): $(ELF)
	$(PRXGEN) build $(ELF) -o $(PRX) --name voxel3d
	$(PRXGEN) check $(PRX)

$(TARGET): $(PRX)
	$(MAKESPRX) $(PRX) $(TARGET)

$(FAKE): $(PRX)
	$(FSELF) $(PRX) $(FAKE)

clean:
	-del /q src\main.o src\hook.o src\m4a1_vx3.o 2>nul
	-del /q $(TARGET) $(FAKE) $(ELF) $(PRX) 2>nul
	-del /q assets\m4a1.vx3 2>nul
	-if exist assets rmdir assets
