#!/usr/bin/env python3
"""Cambia el DEFAULT_PORT del plugin GhidraMCP dentro del jar (8080 -> 18080)."""
import os
import shutil
import struct
import zipfile

JAR = (r"D:\Descargas de OperaGX\ghidra_12.2_PUBLIC_20261016\ghidra_12.2_PUBLIC"
       r"\Ghidra\Extensions\GhidraMCP\lib\GhidraMCP.jar")
CLS = "com/lauriewired/GhidraMCPPlugin.class"
OLD, NEW = 8080, 18080

z = zipfile.ZipFile(JAR)
entries = [(i, z.read(i.filename)) for i in z.infolist()]
z.close()

def patch_class(b):
    b = bytearray(b)
    i = 8
    n = struct.unpack_from(">H", b, i)[0]
    i += 2
    idx = 1
    patched = []
    while idx < n:
        tag = b[i]
        start = i
        i += 1
        if tag == 3:  # Integer
            val = struct.unpack_from(">i", b, i)[0]
            if val == OLD:
                patched.append(i)
            i += 4
        elif tag == 4:  # Float
            i += 4
        elif tag == 5:  # Long
            i += 8; idx += 1
        elif tag == 6:  # Double
            i += 8; idx += 1
        elif tag in (7, 8, 16, 19, 20):
            i += 2
        elif tag == 15:
            i += 3
        elif tag in (9, 10, 11, 12, 17, 18):
            i += 4
        elif tag == 1:  # Utf8
            ln = struct.unpack_from(">H", b, i)[0]
            i += 2 + ln
        else:
            raise ValueError("tag CP desconocido %d en %d" % (tag, start))
        idx += 1
    for off in patched:
        struct.pack_into(">i", b, off, NEW)
    # El codigo usa 'sipush 8080' (0x11 0x1F 0x90) en el constructor y en init()
    sip = bytearray((0x11, (OLD >> 8) & 0xFF, OLD & 0xFF))
    rep = bytearray((0x11, (NEW >> 8) & 0xFF, NEW & 0xFF))
    pos = 0
    while True:
        pos = b.find(sip, pos)
        if pos < 0:
            break
        b[pos:pos + 3] = rep
        patched.append(pos)
        pos += 3
    if not patched:
        raise ValueError("no se encontro 8080 ni en constant pool ni en sipush")
    return bytes(b), len(patched)

out = []
for info, b in entries:
    if info.filename == CLS:
        b, cnt = patch_class(b)
        print("parcheados %d Constant_Integer(8080 -> 18080)" % cnt)
    out.append((info, b))

tmp = JAR + ".tmp"
with zipfile.ZipFile(tmp, "w", zipfile.ZIP_DEFLATED) as z2:
    for info, b in out:
        z2.writestr(info, b)
shutil.copy2(JAR, JAR + ".bak-8080")
os.replace(tmp, JAR)
print("jar actualizado:", JAR)
