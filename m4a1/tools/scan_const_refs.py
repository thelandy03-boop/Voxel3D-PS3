#!/usr/bin/env python3
"""Busca todas las instrucciones que leen la tabla de constantes via r2=0x160000."""
import struct
from capstone import Cs, CS_ARCH_PPC, CS_MODE_64, CS_MODE_BIG_ENDIAN

ELF = (r"D:\Descargas de OperaGX\Minecraft - PlayStation 3 Edition (Europe) "
       r"(EnJaFrDeEsItNlPtSvNoDaFiZhKoPlRuTr)\Minecraft - PlayStation 3 Edition "
       r"(Europe) (En,Ja,Fr,De,Es,It,Nl,Pt,Sv,No,Da,Fi,Zh,Ko,Pl,Ru,Tr)"
       r"\PS3_GAME\USRDIR\EBOOT.elf")
BIAS = 0x10000
R2 = 0x160000

CONSTS = {
    0x0015D1E8: "-0.40f/double table start",
    0x0015D1F0: "-0.72f (anchor)",
    0x0015D24C: "0.56f",
    0x0015D250: "-0.52f",
    0x0015D254: "60.0f",
}

data = open(ELF, "rb").read()
md = Cs(CS_ARCH_PPC, CS_MODE_64 | CS_MODE_BIG_ENDIAN)

hits = []
for off in range(0, len(data) - 3, 4):
    w = struct.unpack_from(">I", data, off)[0]
    op = w >> 26
    if op not in (48, 50):          # lfs=48, lfd=50
        continue
    if ((w >> 16) & 31) != 2:       # RA == r2
        continue
    d = w & 0xFFFF
    va = (R2 + (d - 0x10000 if d & 0x8000 else d)) & 0xFFFFFFFF
    if va in CONSTS:
        hits.append((off + BIAS, va, w))

print("Referencias r2-relativas a la tabla:", len(hits))
for va, target, w in hits[:200]:
    print("  0x%08X -> 0x%08X (%s)  bytes=%08X" % (va, target, CONSTS[target], w))

# Funciones candidates: agrupar por bloque de 0x400 hacia atras
def va_bytes(va, n):
    return data[va - BIAS:va - BIAS + n]

def find_prologue(va):
    start = va - 0x500
    code = va_bytes(start, 0x520)
    best = None
    for ins in md.disasm(code, start):
        if ins.mnemonic == "mflr" and ins.op_str == "r0":
            nxt = list(md.disasm(va_bytes(ins.address, 20), ins.address))
            if any(x.mnemonic == "stdu" and x.op_str.startswith("r1,") for x in nxt[:4]):
                best = ins.address
    return best

print("\n--- Funciones que referencian la tabla ---")
funcs = {}
for va, target, w in hits:
    p = find_prologue(va)
    funcs.setdefault(p, []).append((va, target))

for p, refs in sorted(funcs.items(), key=lambda x: (x[0] is None, x[0] or 0)):
    print("  prologo = %s   refs=%d" % ("0x%08X" % p if p else "??", len(refs)))
    for va, target in refs[:12]:
        print("        0x%08X -> 0x%08X %s" % (va, target, CONSTS[target]))
