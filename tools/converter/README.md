# Herramienta de Conversión Offline (VOX / JSON / OBJ -> VX3)

Esta carpeta contendrá el script CLI (en Python/C++) encargado de:

1. Importar modelos vóxel desde MagicaVoxel (`.vox`) o Blockbench (`.bbmodel` / `.json`).
2. Realizar culling de caras ocluidas y simplificación (*greedy meshing*).
3. Empaquetar los datos en la estructura binaria plana `.vx3` optimizada para el RSX de PS3.
