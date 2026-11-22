#!/usr/bin/env python3
"""Localiza el prologo de la funcion que contiene 0x005DA920 en el EBOOT.elf."""
import struct
from capstone import Cs, CS_ARCH_PPC, CS_MODE_64, CS_MODE_BIG_ENDIAN

ELF = (r"D:\Descargas de OperaGX\Minecraft - PlayStation 3 Edition (Europe) "
       r"(EnJaFrDeEsItNlPtSvNoDaFiZhKoPlRuTr)\Minecraft - PlayStation 3 Edition "
       r"(Europe) (En,Ja,Fr,De,Es,It,Nl,Pt,Sv,No,Da,Fi,Zh,Ko,Pl,Ru,Tr)"
       r"\PS3_GAME\USRDIR\EBOOT.elf")

TARGET = 0x005DA920
BIAS = 0x00010000          # PH[00]: off 0 -> vaddr 0x10000

data = open(ELF, "rb").read()
assert data[:4] == b"\x7fELF" and data[4] == 2 and data[5] == 2, "no es ELF64 BE PPC"

def rd(va, n):
    o = va - BIAS
    return data[o:o + n]

md = Cs(CS_ARCH_PPC, CS_MODE_64 | CS_MODE_BIG_ENDIAN)
md.detail = False

print("Bytes en 0x005DA920:", rd(TARGET, 16).hex(" "))
print("Bytes en 0x0015D1E8:", rd(0x0015D1E8, 32).hex(" "))

for a in (0x0015D1E8, 0x0015D1F0, 0x0015D24C, 0x0015D250, 0x0015D254):
    b = rd(a, 4)
    print("  0x%08X: %s  float=% .6f" % (a, b.hex(), struct.unpack(">f", b)[0]))
d = rd(0x0015D1E8, 8)
print("  0x0015D1E8 double =", struct.unpack(">d", d)[0])

# Ventana hacia atras: buscamos el primer 'mflr r0' seguido de 'stwu r1'
START_SCAN = TARGET - 0x400
code = rd(START_SCAN, 0x420)
insns = list(md.disasm(code, START_SCAN))

for i, ins in enumerate(insns):
    if ins.address <= TARGET <= ins.address + ins.size - 1:
        print("\n>>> 0x005DA920 cae en:", ins.mnemonic, ins.op_str, "@ 0x%X" % ins.address)

print("\n--- Candidatos a prologo (mflr / stwu r1) ---")
for i, ins in enumerate(insns):
    if ins.mnemonic in ("mflr", "stwu") and ("r0" in ins.op_str or "r1" in ins.op_str):
        nxt = " | ".join("%s %s" % (x.mnemonic, x.op_str) for x in insns[i:i + 4])
        print("0x%08X: %-6s %-18s  => %s" % (ins.address, ins.mnemonic, ins.op_str, nxt))

print("\n--- Desensamblado 0x005DA8F0 .. 0x005DA950 ---")
for ins in insns:
    if 0x005DA8F0 <= ins.address <= 0x005DA950:
        print("0x%08X: %-8s %s" % (ins.address, ins.bytes.hex(), ins.mnemonic + " " + ins.op_str))
