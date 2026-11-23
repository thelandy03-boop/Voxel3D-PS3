const fs = require('fs');
const path = require('path');
const crypto = require('crypto');

// Constantes oficiales Sony PS3 PKG
const PKG_MAGIC = Buffer.from([0x7F, 0x50, 0x4B, 0x47]); // "\x7fPKG"

function buildParamSfo(entries) {
  const keys = Object.keys(entries).sort();
  const headerSize = 20;
  const entryTableSize = keys.length * 16;
  
  let keyTable = Buffer.alloc(0);
  const keyOffsets = [];
  for (const k of keys) {
    keyOffsets.push(keyTable.length);
    keyTable = Buffer.concat([keyTable, Buffer.from(k + '\0', 'utf8')]);
  }
  const keyPad = (4 - (keyTable.length % 4)) % 4;
  keyTable = Buffer.concat([keyTable, Buffer.alloc(keyPad)]);
  
  const keyTableOffset = headerSize + entryTableSize;
  const dataTableOffset = keyTableOffset + keyTable.length;
  
  let dataTable = Buffer.alloc(0);
  const entryRecords = [];
  
  for (let i = 0; i < keys.length; i++) {
    const k = keys[i];
    const val = entries[k];
    const keyOff = keyOffsets[i];
    let fmt = 0x0204;
    let dataBuf;
    let maxLen = 0;
    
    if (typeof val === 'number') {
      fmt = 0x0404;
      dataBuf = Buffer.alloc(4);
      dataBuf.writeUInt32LE(val, 0);
      maxLen = 4;
    } else {
      const s = String(val);
      const strBuf = Buffer.from(s + '\0', 'utf8');
      maxLen = Math.ceil(strBuf.length / 4) * 4;
      if (k === 'TITLE') maxLen = 128;
      if (k === 'CONTENT_ID') maxLen = 48;
      dataBuf = Buffer.alloc(maxLen);
      strBuf.copy(dataBuf, 0);
    }
    
    const dataOff = dataTable.length;
    dataTable = Buffer.concat([dataTable, dataBuf]);
    
    entryRecords.push({
      keyOff,
      fmt,
      len: (typeof val === 'number') ? 4 : Buffer.from(String(val) + '\0', 'utf8').length,
      maxLen,
      dataOff
    });
  }
  
  const header = Buffer.alloc(headerSize);
  header.write('\0PSF', 0, 4, 'ascii');
  header.writeUInt32LE(0x00000101, 4);
  header.writeUInt32LE(keyTableOffset, 8);
  header.writeUInt32LE(dataTableOffset, 12);
  header.writeUInt32LE(keys.length, 16);
  
  const entriesBuf = Buffer.alloc(entryTableSize);
  for (let i = 0; i < entryRecords.length; i++) {
    const r = entryRecords[i];
    const off = i * 16;
    entriesBuf.writeUInt16LE(r.keyOff, off);
    entriesBuf.writeUInt16LE(r.fmt, off + 2);
    entriesBuf.writeUInt32LE(r.len, off + 4);
    entriesBuf.writeUInt32LE(r.maxLen, off + 8);
    entriesBuf.writeUInt32LE(r.dataOff, off + 12);
  }
  
  return Buffer.concat([header, entriesBuf, keyTable, dataTable]);
}

function createPs3Pkg(inputDir, contentId, outputFile) {
  console.log('[*] Escaneando directorio:', inputDir);
  
  const entries = [];
  function scan(dir, rel) {
    const list = fs.readdirSync(dir, { withFileTypes: true });
    list.sort((a, b) => a.name.localeCompare(b.name));
    for (const item of list) {
      const full = path.join(dir, item.name);
      const r = rel ? (rel + '/' + item.name) : item.name;
      if (item.isDirectory()) {
        entries.push({ relPath: r, isDir: true, fullPath: full, size: 0 });
        scan(full, r);
      } else {
        const sz = fs.statSync(full).size;
        entries.push({ relPath: r, isDir: false, fullPath: full, size: sz });
      }
    }
  }
  scan(inputDir, '');
  console.log(`[+] Encontrados ${entries.length} archivos/carpetas para empaquetar.`);

  // Tabla de nombres de archivos
  let nameTable = Buffer.alloc(0);
  for (const ent of entries) {
    ent.nameOffset = nameTable.length;
    const nameBuf = Buffer.from(ent.relPath, 'utf8');
    ent.nameLength = nameBuf.length;
    nameTable = Buffer.concat([nameTable, nameBuf, Buffer.alloc(1)]); // NUL terminated
  }
  const namePad = (16 - (nameTable.length % 16)) % 16;
  nameTable = Buffer.concat([nameTable, Buffer.alloc(namePad)]);

  // Header Sony estándar
  // Offsets exactos especificación Sony:
  // 0x00: Magic "\x7fPKG" (4)
  // 0x04: Revision (uint16) = 0x0001 (debug)
  // 0x06: Type (uint16) = 0x0001
  // 0x08: Info offset (uint32) = 0x000000C0
  // 0x0C: Info count (uint32) = entries.length
  // 0x10: Header size (uint32) = 0x000000C0
  // 0x14: Item count (uint32) = entries.length
  // 0x18: Total size (uint64)
  // 0x20: Data offset (uint64)
  // 0x28: Data size (uint64)
  // 0x30: Content ID (48 bytes)
  // 0x60: Digest (16 bytes)
  // 0x70: Data IV (16 bytes)
  // 0x80: Signatures / Padding (64 bytes)
  // Total Header = 0xC0 (192 bytes)
  const headerSize = 0xC0;
  const fileEntrySize = 0x20;
  const fileTableSize = entries.length * fileEntrySize;
  const infoOffset = headerSize;
  const dataOffset = Math.ceil((headerSize + fileTableSize + nameTable.length) / 64) * 64;
  const padBeforeData = dataOffset - (headerSize + fileTableSize + nameTable.length);

  let curDataOffset = dataOffset;
  for (const ent of entries) {
    if (ent.isDir) {
      ent.dataOffset = 0;
      ent.dataSize = 0;
    } else {
      ent.dataOffset = curDataOffset;
      ent.dataSize = ent.size;
      const pad = (16 - (ent.size % 16)) % 16;
      curDataOffset += ent.size + pad;
    }
  }

  const totalDataSize = curDataOffset - dataOffset;
  const trailerSize = 0x60;
  const totalPkgSize = curDataOffset + trailerSize;

  const header = Buffer.alloc(headerSize);
  PKG_MAGIC.copy(header, 0);
  header.writeUInt16BE(0x0001, 0x04); // Debug PKG
  header.writeUInt16BE(0x0001, 0x06); // Type
  header.writeUInt32BE(infoOffset, 0x08); // Offset to file table
  header.writeUInt32BE(entries.length, 0x0C); // Count
  header.writeUInt32BE(headerSize, 0x10); // Header size (0xC0)
  header.writeUInt32BE(entries.length, 0x14); // Item count
  header.writeBigUInt64BE(BigInt(totalPkgSize), 0x18); // TOTAL SIZE EN 0x18 (Corregido)
  header.writeBigUInt64BE(BigInt(dataOffset), 0x20); // DATA OFFSET EN 0x20 (Corregido)
  header.writeBigUInt64BE(BigInt(totalDataSize), 0x28); // DATA SIZE EN 0x28 (Corregido)
  header.write(contentId.padEnd(48, '\0'), 0x30, 48, 'ascii'); // CONTENT ID EN 0x30 (Corregido)

  // Digest HMAC SHA-1 de la cabecera
  const hmac = crypto.createHmac('sha1', Buffer.alloc(16));
  hmac.update(header.subarray(0, 0x60));
  const digest = hmac.digest();
  digest.copy(header, 0x60, 0, 16);

  // Tabla de archivos (0x20 bytes por entrada)
  // 0x00: Name offset (uint32)
  // 0x04: Name length (uint32)
  // 0x08: Data offset (uint64)
  // 0x10: Data size (uint64)
  // 0x18: Flags / type (uint32: 1=dir, 2=file)
  // 0x1C: Padding (uint32: 0)
  const fileTable = Buffer.alloc(fileTableSize);
  for (let i = 0; i < entries.length; i++) {
    const ent = entries[i];
    const off = i * fileEntrySize;
    fileTable.writeUInt32BE(ent.nameOffset, off);
    fileTable.writeUInt32BE(ent.nameLength, off + 0x04);
    fileTable.writeBigUInt64BE(BigInt(ent.dataOffset), off + 0x08);
    fileTable.writeBigUInt64BE(BigInt(ent.dataSize), off + 0x10);
    fileTable.writeUInt32BE(ent.isDir ? 1 : 2, off + 0x18);
    fileTable.writeUInt32BE(0, off + 0x1C);
  }

  const out = fs.createWriteStream(outputFile);
  out.write(header);
  out.write(fileTable);
  out.write(nameTable);
  if (padBeforeData > 0) out.write(Buffer.alloc(padBeforeData));

  for (const ent of entries) {
    if (!ent.isDir) {
      const data = fs.readFileSync(ent.fullPath);
      out.write(data);
      const pad = (16 - (ent.size % 16)) % 16;
      if (pad > 0) out.write(Buffer.alloc(pad));
    }
  }

  // Trailer / Signature block
  const trailer = Buffer.alloc(trailerSize);
  trailer.write('DEBUG_PACKAGE_SIGNATURE_OK\0', 0, 'ascii');
  out.write(trailer);
  out.end();

  console.log(`[+] ¡PKG generado exitosamente! -> ${outputFile} (${totalPkgSize} bytes)`);
}

// 1. PARAM.SFO
const sfo = buildParamSfo({
  APP_VER: '01.00',
  BOOTABLE: 1,
  CATEGORY: 'HG',
  CONTENT_ID: 'UP0001-MCPS3MODS_00-0000000000000000',
  PARENTAL_LEVEL: 1,
  RESOLUTION: 63,
  SOUND_FORMAT: 1,
  TITLE: 'Minecraft: PS3 Edition - Mod Loader & Manager v1.4.2',
  TITLE_ID: 'MCPS3MODS',
  VERSION: '01.42'
});
fs.writeFileSync('app_ps3/pkgfiles/PARAM.SFO', sfo);

// 2. Generar PKG
const pkgName = 'Minecraft_PS3_Mod_Loader_v1.4.2.pkg';
createPs3Pkg('app_ps3/pkgfiles', 'UP0001-MCPS3MODS_00-0000000000000000', pkgName);
fs.copyFileSync(pkgName, path.join('tools/packaging', pkgName));
