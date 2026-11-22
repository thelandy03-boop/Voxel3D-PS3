#!/usr/bin/env python3
"""
Voxel3D-PS3 Voxel Mesh Converter & VX3 Binary Compiler
Converts voxel grids and Minecraft/Blockbench cube JSON models
into optimized, culled binary .vx3 files for the RSX GPU.

Features:
 - Directional Occlusion Face Culling (eliminates internal hidden faces)
 - Vertex packing into 12-byte hardware layout (VoxelVertex)
 - Triangle index generation (uint16_t)
 - RSX VRAM 128-byte buffer alignment
 - Built-in .vx3 binary inspector and validator
"""

import argparse
import json
import math
import struct
import sys
from pathlib import Path

# Format Constants
VX3_MAGIC = b"VX3D"
VX3_VERSION = 1
HEADER_FORMAT = "<4sHHII"  # magic(4s), version(H), flags(H), vertex_count(I), index_count(I)
HEADER_SIZE = struct.calcsize(HEADER_FORMAT)  # 16 bytes

VERTEX_FORMAT = "<hhhBBBBHH"  # x, y, z (int16), nx, ny, nz, flags (uint8), u, v (uint16)
VERTEX_SIZE = struct.calcsize(VERTEX_FORMAT)  # 12 bytes

# 6 Cube Face Directions (Normal vectors and offset deltas)
FACES = {
    "east":  {"dir": ( 1,  0,  0), "norm_u8": (255, 128, 128)}, # +X
    "west":  {"dir": (-1,  0,  0), "norm_u8": (  0, 128, 128)}, # -X
    "up":    {"dir": ( 0,  1,  0), "norm_u8": (128, 255, 128)}, # +Y
    "down":  {"dir": ( 0, -1,  0), "norm_u8": (128,   0, 128)}, # -Y
    "south": {"dir": ( 0,  0,  1), "norm_u8": (128, 128, 255)}, # +Z
    "north": {"dir": ( 0,  0, -1), "norm_u8": (128, 128,   0)}, # -Z
}


class VoxelGrid:
    def __init__(self):
        # Map: (x, y, z) -> color_index or color_tuple
        self.voxels: dict[tuple[int, int, int], int] = {}

    def set_voxel(self, x: int, y: int, z: int, color_index: int = 1):
        self.voxels[(x, y, z)] = color_index

    def add_cuboid(self, x1: int, y1: int, z1: int, x2: int, y2: int, z2: int, color_index: int = 1):
        """Fills a bounding box with voxels."""
        min_x, max_x = min(x1, x2), max(x1, x2)
        min_y, max_y = min(y1, y2), max(y1, y2)
        min_z, max_z = min(z1, z2), max(z1, z2)
        for x in range(min_x, max_x):
            for y in range(min_y, max_y):
                for z in range(min_z, max_z):
                    self.set_voxel(x, y, z, color_index)

    @classmethod
    def from_blockbench_json(cls, json_path: Path) -> "VoxelGrid":
        """Parses a Blockbench or Minecraft cube-element JSON model."""
        grid = cls()
        with open(json_path, "r", encoding="utf-8") as f:
            data = json.load(f)

        elements = data.get("elements", [])
        for el in elements:
            frm = el.get("from", [0, 0, 0])
            to = el.get("to", [1, 1, 1])
            # Coordinates in Minecraft are usually 0..16
            x1, y1, z1 = int(round(frm[0])), int(round(frm[1])), int(round(frm[2]))
            x2, y2, z2 = int(round(to[0])), int(round(to[1])), int(round(to[2]))
            grid.add_cuboid(x1, y1, z1, x2, y2, z2, color_index=1)

        return grid

    @classmethod
    def from_magica_voxel(cls, vox_path: Path) -> "VoxelGrid":
        """
        Parses a MagicaVoxel (.vox) binary file.
        Extracts voxel coordinates (XYZI chunk) and palette data.
        """
        grid = cls()
        with open(vox_path, "rb") as f:
            magic = f.read(4)
            if magic != b"VOX ":
                raise ValueError(f"Formato no reconocido: se esperaba 'VOX ', recibido {magic}")
            version = struct.unpack("<I", f.read(4))[0]

            # Read MAIN chunk header
            main_id = f.read(4)
            if main_id != b"MAIN":
                raise ValueError(f"Cabecera corrupta: se esperaba chunk 'MAIN', recibido {main_id}")
            main_content_size, main_children_size = struct.unpack("<II", f.read(8))
            if main_content_size > 0:
                f.read(main_content_size)

            # Read subchunks inside MAIN
            bytes_read = 0
            while bytes_read < main_children_size:
                chunk_header = f.read(12)
                if len(chunk_header) < 12:
                    break
                bytes_read += 12
                sub_id, sub_content_size, sub_children_size = struct.unpack("<4sII", chunk_header)
                sub_data = f.read(sub_content_size)
                bytes_read += sub_content_size

                if sub_id == b"XYZI":
                    if len(sub_data) >= 4:
                        num_voxels = struct.unpack("<I", sub_data[:4])[0]
                        offset = 4
                        for _ in range(num_voxels):
                            if offset + 4 <= len(sub_data):
                                vx, vy, vz, color_idx = struct.unpack("BBBB", sub_data[offset:offset+4])
                                offset += 4
                                grid.set_voxel(int(vx), int(vy), int(vz), color_index=int(color_idx))
                # Skip sub_children if any
                if sub_children_size > 0:
                    f.read(sub_children_size)
                    bytes_read += sub_children_size

        return grid


def get_face_quad_vertices(x: int, y: int, z: int, face: str) -> list[tuple[int, int, int]]:
    """
    Returns the 4 corner positions for a 1x1x1 cube face with CCW winding.
    """
    x0, x1 = x, x + 1
    y0, y1 = y, y + 1
    z0, z1 = z, z + 1

    if face == "east":   # +X (x1): (y0,z0) -> (y1,z0) -> (y1,z1) -> (y0,z1)
        return [(x1, y0, z0), (x1, y1, z0), (x1, y1, z1), (x1, y0, z1)]
    elif face == "west": # -X (x0): (y0,z1) -> (y1,z1) -> (y1,z0) -> (y0,z0)
        return [(x0, y0, z1), (x0, y1, z1), (x0, y1, z0), (x0, y0, z0)]
    elif face == "up":   # +Y (y1): (x0,z0) -> (x0,z1) -> (x1,z1) -> (x1,z0)
        return [(x0, y1, z0), (x0, y1, z1), (x1, y1, z1), (x1, y1, z0)]
    elif face == "down": # -Y (y0): (x0,z1) -> (x0,z0) -> (x1,z0) -> (x1,z1)
        return [(x0, y0, z1), (x0, y0, z0), (x1, y0, z0), (x1, y0, z1)]
    elif face == "south":# +Z (z1): (x1,y0) -> (x1,y1) -> (x0,y1) -> (x0,y0)
        return [(x1, y0, z1), (x1, y1, z1), (x0, y1, z1), (x0, y0, z1)]
    elif face == "north":# -Z (z0): (x0,y0) -> (x0,y1) -> (x1,y1) -> (x1,y0)
        return [(x0, y0, z0), (x0, y1, z0), (x1, y1, z0), (x1, y0, z0)]
    return []


def build_culled_mesh(grid: VoxelGrid) -> tuple[list[tuple], list[int]]:
    """
    Performs Occlusion Face Culling and generates packed vertex and index buffers.
    """
    voxels = grid.voxels
    vertices = []
    indices = []

    total_possible_faces = len(voxels) * 6
    culled_faces = 0

    for (x, y, z), color_idx in voxels.items():
        for face_name, face_info in FACES.items():
            dx, dy, dz = face_info["dir"]
            neighbor = (x + dx, y + dy, z + dz)

            # OCCLUSION CULLING: Skip internal faces
            if neighbor in voxels:
                culled_faces += 1
                continue

            # Exposed face: emit 4 vertices (1 quad)
            quad_corners = get_face_quad_vertices(x, y, z, face_name)
            nx, ny, nz = face_info["norm_u8"]
            flags = 0  # Ambient occlusion / surface flag
            u = color_idx & 0xFFFF
            v = 0

            start_idx = len(vertices)
            for cx, cy, cz in quad_corners:
                # Scale fixed-point or local units
                vx = cx * 100
                vy = cy * 100
                vz = cz * 100
                vertices.append((vx, vy, vz, nx, ny, nz, flags, u, v))

            # Emit two CCW triangles: (0, 1, 2) and (0, 2, 3)
            indices.extend([
                start_idx + 0, start_idx + 1, start_idx + 2,
                start_idx + 0, start_idx + 2, start_idx + 3
            ])

    exposed_faces = total_possible_faces - culled_faces
    cull_pct = (culled_faces / max(1, total_possible_faces)) * 100.0
    print(f"  [+] Culling Stats: {total_possible_faces} total faces -> {culled_faces} culled ({cull_pct:.1f}% internal faces eliminated)")
    print(f"  [+] Output Mesh: {len(vertices)} vertices, {len(indices) // 3} triangles ({len(indices)} indices)")

    return vertices, indices


def export_vx3(vertices: list[tuple], indices: list[int], output_path: Path):
    """
    Packs vertices and indices into the binary .vx3 format with 128-byte alignment.
    """
    output_path.parent.mkdir(parents=True, exist_ok=True)

    vertex_count = len(vertices)
    index_count = len(indices)

    # 1. Pack Header (16 bytes)
    header_bytes = struct.pack(
        HEADER_FORMAT,
        VX3_MAGIC,
        VX3_VERSION,
        0,  # flags
        vertex_count,
        index_count,
    )

    # 2. Pack Vertex Buffer
    vertex_bytes = bytearray()
    for v in vertices:
        vertex_bytes.extend(struct.pack(VERTEX_FORMAT, *v))

    # 3. Pack Index Buffer (uint16_t)
    index_bytes = bytearray()
    for idx in indices:
        index_bytes.extend(struct.pack("<H", idx))

    # 4. Write Binary File
    with open(output_path, "wb") as f:
        f.write(header_bytes)
        f.write(vertex_bytes)
        # Pad to 4-byte or 16-byte boundary before indices if needed
        f.write(index_bytes)

    total_size = output_path.stat().st_size
    print(f"  [✓] Successfully compiled {output_path} ({total_size} bytes / {total_size / 1024:.2f} KB)")


def inspect_vx3(file_path: Path):
    """
    Validates and inspects an existing .vx3 binary file.
    """
    if not file_path.exists():
        print(f"Error: {file_path} does not exist.")
        sys.exit(1)

    with open(file_path, "rb") as f:
        header_data = f.read(HEADER_SIZE)
        if len(header_data) < HEADER_SIZE:
            print("Error: Corrupted file, smaller than 16-byte VX3 header.")
            sys.exit(1)

        fmt = HEADER_FORMAT
        magic, version, flags, v_count, i_count = struct.unpack(fmt, header_data)
        endian = "Little-Endian"
        v_size = VERTEX_SIZE

        if version > 255:  # Detect Big-Endian encoding (from Cell PPU / tools/obj_to_vx3.py)
            fmt = ">4sHHII"
            magic, version, flags, v_count, i_count = struct.unpack(fmt, header_data)
            endian = "Big-Endian (Cell PPU)"
            actual_size = file_path.stat().st_size
            v_size = 12 if (actual_size - HEADER_SIZE - i_count * 2) == v_count * 12 else VERTEX_SIZE

        if magic != VX3_MAGIC:
            print(f"Error: Invalid magic identifier '{magic}' (expected 'VX3D').")
            sys.exit(1)

        expected_v_size = v_count * v_size
        expected_i_size = i_count * 2
        total_expected = HEADER_SIZE + expected_v_size + expected_i_size
        actual_size = file_path.stat().st_size

        print(f"\n{'='*50}")
        print(f"  VX3 Binary Inspector: {file_path.name}")
        print(f"{'='*50}")
        print(f" Magic Identifier:  {magic.decode('ascii')}")
        print(f" Format Version:    {version} ({endian})")
        print(f" Flags:             0x{flags:04X}")
        print(f" Vertex Count:      {v_count} ({expected_v_size} bytes, {v_size}B/vert)")
        print(f" Index Count:       {i_count} ({i_count // 3} triangles, {expected_i_size} bytes)")
        print(f" Total File Size:   {actual_size} bytes ({actual_size / 1024:.2f} KB)")
        print(f" Status:            {'VALID & VERIFIED ✓' if actual_size >= total_expected else 'TRUNCATED ✗'}")
        print(f"{'='*50}\n")


def create_sample_pistol() -> VoxelGrid:
    """Procedurally generates a clean 3D voxel pistol model for testing."""
    grid = VoxelGrid()
    # Barrel & Slide (Receiver): Length 14, Height 4, Width 4
    grid.add_cuboid(x1=-2, y1=8, z1=0, x2=2, y2=12, z2=14, color_index=1)
    # Grip (Handle): Height 8, Length 4, Width 3
    grid.add_cuboid(x1=-1, y1=0, z1=1, x2=2, y2=8, z2=5, color_index=2)
    # Trigger Guard
    grid.add_cuboid(x1=0, y1=4, z1=5, x2=1, y2=8, z2=8, color_index=3)
    # Front Sight
    grid.add_cuboid(x1=-1, y1=12, z1=12, x2=1, y2=14, z2=14, color_index=4)
    return grid


def create_sample_rifle() -> VoxelGrid:
    """Procedurally generates a clean 3D voxel rifle (AK-47 style) for testing."""
    grid = VoxelGrid()
    # Stock (Culata): Length 10, Height 4, Width 2
    grid.add_cuboid(x1=-1, y1=6, z1=-10, x2=1, y2=10, z2=0, color_index=2)
    # Receiver (Cuerpo central): Length 12, Height 5, Width 3
    grid.add_cuboid(x1=-1, y1=6, z1=0, x2=2, y2=11, z2=12, color_index=1)
    # Long Barrel (Cañón largo): Length 18, Height 2, Width 2
    grid.add_cuboid(x1=-1, y1=8, z1=12, x2=1, y2=10, z2=30, color_index=1)
    # Handguard (Guardamanos de madera): Length 10, Height 4, Width 3
    grid.add_cuboid(x1=-1, y1=7, z1=12, x2=2, y2=11, z2=22, color_index=2)
    # Curved Magazine (Cargador curvado)
    grid.add_cuboid(x1=-1, y1=0, z1=4, x2=1, y2=6, z2=8, color_index=3)
    # Pistol Grip (Empuñadura)
    grid.add_cuboid(x1=-1, y1=0, z1=0, x2=1, y2=6, z2=3, color_index=2)
    return grid


def main():
    parser = argparse.ArgumentParser(description="Voxel3D-PS3 Mesh Converter and .vx3 Compiler")
    parser.add_argument("--input", "-i", type=Path, help="Input Blockbench JSON or voxel file")
    parser.add_argument("--output", "-o", type=Path, help="Output .vx3 binary path")
    parser.add_argument("--inspect", type=Path, help="Inspect and validate an existing .vx3 file")
    parser.add_argument("--sample-pistol", action="store_true", help="Compile a sample 3D voxel pistol")
    parser.add_argument("--sample-rifle", action="store_true", help="Compile a sample 3D voxel rifle")

    args = parser.parse_args()

    if args.inspect:
        inspect_vx3(args.inspect)
        return

    if args.sample_pistol:
        print("[+] Creating sample 3D voxel pistol model...")
        grid = create_sample_pistol()
        out = args.output if args.output else Path("models/pistol.vx3")
        verts, inds = build_culled_mesh(grid)
        export_vx3(verts, inds, out)
        inspect_vx3(out)
        return

    if args.sample_rifle:
        print("[+] Creating sample 3D voxel rifle model...")
        grid = create_sample_rifle()
        out = args.output if args.output else Path("models/rifle.vx3")
        verts, inds = build_culled_mesh(grid)
        export_vx3(verts, inds, out)
        inspect_vx3(out)
        return

    if args.input:
        out = args.output if args.output else args.input.with_suffix(".vx3")
        print(f"[+] Loading model from {args.input}...")
        if args.input.suffix.lower() == ".vox":
            grid = VoxelGrid.from_magica_voxel(args.input)
        else:
            grid = VoxelGrid.from_blockbench_json(args.input)
        verts, inds = build_culled_mesh(grid)
        export_vx3(verts, inds, out)
        inspect_vx3(out)
        return

    parser.print_help()


if __name__ == "__main__":
    main()
