# Makefile de esqueleto para el plugin Voxel3D-PS3 (.sprx)
# Requiere PSL1GHT / ps3dev toolchain

TARGET = Voxel3D-PS3
OBJS = src/main.o src/hook.o

CFLAGS = -Wall -O2 -mcpu=cell -fno-strict-aliasing
CXXFLAGS = $(CFLAGS) -fno-rtti -fno-exceptions
INCLUDES = -Iinclude -I$(PS3DEV)/portlibs/ppu/include

LIBS = -lgcm_cmd -lsysmodule -lnet

all: $(TARGET).sprx

%.o: %.cpp
	@echo "Compilando $<..."
	# $(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(TARGET).sprx: $(OBJS)
	@echo "Esqueleto listo. Configura tu toolchain ps3dev para compilar el binario final."

clean:
	rm -f $(OBJS) $(TARGET).sprx $(TARGET).prx
