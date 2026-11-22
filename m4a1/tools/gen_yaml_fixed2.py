"""Genera imported_patch.yml con un cave formado SOLO por instrucciones PPU.

Se ensambla loader_instr.s con ppu-as. La ruta del SPRX se construye con
lis/ori/stw en la pila; no hay ni una sola palabra ASCII en el cave, asi que
RPCS3 no puede caer en 'Unsupported calloc instruction'.
"""
import pathlib
import struct
import subprocess
import tempfile

SRC = pathlib.Path(r"C:\Users\USER\AppData\Local\Temp\opencode\loader_instr.s")
OUTS = [
    pathlib.Path(r"D:\Descargas de OperaGX\MOD_M4A1\patches\imported_patch.yml"),
    pathlib.Path(r"D:\rpcs3-v0.0.43-20644-3bfac607_win64_msvc\patches\imported_patch.yml"),
]
KEYS = [
    "PPU-a51d222f3658b515f567d7d63d6b560c7f7e2972",
    "BLES01976-PPU-a51d222f3658b515f567d7d63d6b560c7f7e2972",
    "PPU-d29a1ced1051a3dd6ff04f6ce2b33a322ab8db94",
    "BLES01976-PPU-d29a1ced1051a3dd6ff04f6ce2b33a322ab8db94",
]
PATH = b"/dev_hdd0/game/BLES01976/USRDIR/voxel3d/voxel3d.sprx"
AS = r"C:\ps3sdk\ppu\bin\ppu-as.exe"
OBJCOPY = r"C:\ps3sdk\ppu\bin\ppu-objcopy.exe"

route = PATH + b"\0"
route += b"\0" * ((-len(route)) % 4)
words = [int.from_bytes(route[i:i + 4], "big") for i in range(0, len(route), 4)]

build = ["\tlis %r10,0x2f64"]  # placeholder, se sobrescribe abajo
build = []
for i, w in enumerate(words):
    hi, lo = (w >> 16) & 0xFFFF, w & 0xFFFF
    if hi:
        build.append(f"\tlis %r10,0x{hi:04x}")
    else:
        build.append("\tli %r10,0")
    if lo:
        build.append(f"\tori %r10,%r10,0x{lo:04x}")
    build.append(f"\tstw %r10,{i * 4}(%r9)")
build.append("\tmr %r3,%r9")

asm = SRC.read_text(encoding="utf-8").replace("@@PATH_BUILD@@", "\n".join(build))

with tempfile.TemporaryDirectory() as td:
    s = pathlib.Path(td) / "loader.s"
    o = pathlib.Path(td) / "loader.o"
    b = pathlib.Path(td) / "loader.bin"
    s.write_text(asm, encoding="utf-8", newline="\n")
    subprocess.run([AS, "-o", str(o), str(s)], check=True)
    subprocess.run([OBJCOPY, "-O", "binary", "--only-section=.text", str(o), str(b)], check=True)
    raw = b.read_bytes()
assert len(raw) % 4 == 0
cave = [struct.unpack_from(">I", raw, i)[0] for i in range(0, len(raw), 4)]
assert all(w != 0 for w in cave), "palabra 0x00000000 en el cave"
N = len(cave)

header = """Version: 1.2


{key}:
  Auto-load M4A1 SPRX:
    Games:
      Minecraft:
        BLES01976: [ All ]
    Author: MOD M4A1
    Patch Version: 1.2
    Notes:
      - "Carga e inicia automaticamente voxel3d.sprx al arrancar el juego."
      - "Code cave sin ASCII inline: la ruta se construye con instrucciones PPU en la pila."
    Patch:
      - [ calloc, 0x2c8250, {n} ]
"""
out = []
for key in KEYS:
    out.append(header.format(key=key, n=N))
    out.extend(f"      - [ be32, 0x0, 0x{w:08x} ]\n" for w in cave)
    out.append("\n")
content = "".join(out)
for p in OUTS:
    p.write_text(content, encoding="utf-8", newline="\n")
    print("WROTE", p, len(content))

pathlib.Path(r"C:\Users\USER\AppData\Local\Temp\opencode\loader_final.bin").write_bytes(raw)
print("CAVE_WORDS", N, "BYTES", len(raw))
