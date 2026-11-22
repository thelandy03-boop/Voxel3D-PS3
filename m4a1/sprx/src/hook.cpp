// Voxel3D-PS3 :: Instalador del hook PPU en 0x005DA890
// - Parche de 16 bytes con salto absoluto (lis/ori/mtctr/bctr).
// - Trampoline con el prologo original (3 insns) + handler + emulacion del
//   'bl 0x005DBBD0' original (PC-relativo, no se puede copiar tal cual).
// - OPD (function descriptor) y TOC (r2) preservados.
// -Flush de D-Cache/I-Cache con dcbst/sync/icbi/isync.

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "hook.h"

// ---------------------------------------------------------------------------
// Sin simbolos weak sin resolver: el PRX debe quedar totalmente enlazado.
// ---------------------------------------------------------------------------

static VX3HookContext   g_hook;
static Vox3RenderHandler g_handler_fn = 0;
static void**           g_handler_opd = 0;

static volatile uint64_t g_saved_lr   = 0;   // +0
static volatile uint64_t g_saved_toc  = 0;   // +8
static volatile uint64_t g_saved_args[10];   // r3..r10 en +0..+56, f1 en +64

// ---------------------------------------------------------------------------
// Coherencia de caches. El PPU no mantiene D-Cache e I-Cache coherentes para
// codigo automodificable: hay que hacer store + sync + invalidate + isync.
// ---------------------------------------------------------------------------
void vox3_flush_icache(void* address, size_t size) {
    uintptr_t start = (uintptr_t)address;
    uintptr_t end   = start + size;
    start &= ~(uintptr_t)127;

    for (uintptr_t p = start; p < end; p += 128) {
        __asm__ volatile(
            "dcbst 0, %0\n\t"
            "sync\n\t"
            "icbi 0, %0\n\t"
            "isync\n\t"
            :
            : "r"(p)
            : "memory"
        );
    }
}

static bool vox3_write_memory(void* dst, const void* src, size_t size) {
    memcpy(dst, src, size);
    return true;
}

// ---------------------------------------------------------------------------
// Ensambladores de instrucciones PPC64 (big-endian).
// ---------------------------------------------------------------------------
static inline uint32_t ppc_lis(int rt, uint16_t imm) {
    return 0x3C000000u | ((uint32_t)rt << 21) | (uint32_t)imm;
}
static inline uint32_t ppc_ori(int ra, int rs, uint16_t imm) {
    return 0x60000000u | ((uint32_t)rs << 21) | ((uint32_t)ra << 16) |
           (uint32_t)imm;
}
static inline uint32_t ppc_ld(int rt, int ra, int16_t d) {
    return 0xE8000000u | ((uint32_t)rt << 21) | ((uint32_t)ra << 16) |
           (uint16_t)d;
}
static inline uint32_t ppc_std(int rs, int ra, int16_t d) {
    return 0xF8000000u | ((uint32_t)rs << 21) | ((uint32_t)ra << 16) |
           (uint16_t)d;
}
static inline uint32_t ppc_stfd(int frs, int ra, int16_t d) {
    return 0xD8000000u | ((uint32_t)frs << 21) | ((uint32_t)ra << 16) |
           (uint16_t)d;
}
static inline uint32_t ppc_lfd(int frs, int ra, int16_t d) {
    return 0xC8000000u | ((uint32_t)frs << 21) | ((uint32_t)ra << 16) |
           (uint16_t)d;
}
static inline uint32_t ppc_mtctr(int rs) {
    return 0x7C0903A6u | ((uint32_t)rs << 21);   // mtspr 9 (CTR)
}
static inline uint32_t ppc_mtlr(int rs) {
    return 0x7C0803A6u | ((uint32_t)rs << 21);   // mtspr 8 (LR)
}
static inline uint32_t ppc_li(int rt, int16_t imm) {
    return 0x38000000u | ((uint32_t)rt << 21) | (uint16_t)imm;
}

static const uint32_t PPC_BCTR = 0x4E800420u;
static const uint32_t PPC_BLR  = 0x4E800020u;

static inline void emit_abs(uint32_t* code, size_t& k, int reg,
                            uint32_t addr) {
    code[k++] = ppc_lis(reg, (uint16_t)(addr >> 16));
    code[k++] = ppc_ori(reg, reg, (uint16_t)(addr & 0xFFFF));
}

static inline uint32_t rd_be32(const uint8_t* p) {
    return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) |
           (uint32_t(p[2]) << 8)  |  uint32_t(p[3]);
}

// ---------------------------------------------------------------------------
// Construye el trampoline. El codigo queda en g_hook.trampoline[] (64 palabras).
//
//  [0..2]   prologo original (3 insns / 12 bytes, sin PC-relativos)
//  [3..]    guarda r0(LR), r2(TOC), r3..r10 y f1 en g_saved_*
//           carga OPD del handler -> r2 = TOC del SPRX, CTR = codigo
//           LR = direccion del stub de restauracion
//           r3 = Vox3HookArgs*, bctr -> llama al handler
//  restore: recupera r0/r2, r3..r10 y f1, y emula el 'bl 0x005DBBD0' original con salto
//           absoluto (CTR) y LR = target+16. El 'blr' del callee vuelve solo.
// ---------------------------------------------------------------------------
static void build_trampoline(uint32_t target, Vox3RenderHandler handler) {
    uint32_t* t = g_hook.trampoline;
    memset(t, 0, sizeof(g_hook.trampoline));

    // Prologo original verificado en 0x005DA890:
    //   7c0802a6  mflr r0
    //   f821ff91  stdu r1,-0x70(r1)
    //   f8010080  std  r0,0x80(r1)
    // La 4a instruccion es 'bl 0x005DBBD0' (PC-relativa), asi que no se copia:
    // se reemite como salto absoluto mas abajo.
    memcpy(t, g_hook.original, 12);

    size_t k = 3;
    const uint32_t save_addr = (uint32_t)(uintptr_t)&g_saved_lr;
    const uint32_t args_addr = (uint32_t)(uintptr_t)&g_saved_args[0];

    emit_abs(t, k, 12, save_addr);
    t[k++] = ppc_std(0, 12, 0);      // guarda LR original (r0)
    t[k++] = ppc_std(2, 12, 8);      // guarda TOC del juego (r2)

    // Guarda los argumentos originales antes de sobrescribir r3/r12.
    emit_abs(t, k, 12, args_addr);
    t[k++] = ppc_std(3, 12, 0);
    t[k++] = ppc_std(4, 12, 8);
    t[k++] = ppc_std(5, 12, 16);
    t[k++] = ppc_std(6, 12, 24);
    t[k++] = ppc_std(7, 12, 32);
    t[k++] = ppc_std(8, 12, 40);
    t[k++] = ppc_std(9, 12, 48);
    t[k++] = ppc_std(10, 12, 56);
    t[k++] = ppc_stfd(1, 12, 64);    // guarda f1 (partialTicks) del juego

    // OPD: &handler apunta al descriptor {code, toc, env} en la ABI PPC64 SysV.
    g_handler_fn = handler;
    g_handler_opd = reinterpret_cast<void**>(
        reinterpret_cast<uintptr_t>(g_handler_fn));

    const uint32_t opd_addr = (uint32_t)(uintptr_t)&g_handler_opd;
    emit_abs(t, k, 12, opd_addr);
    t[k++] = ppc_ld(12, 12, 0);      // r12 = descriptor
    t[k++] = ppc_ld(2, 12, 8);       // r2  = TOC del SPRX
    t[k++] = ppc_ld(12, 12, 0);      // r12 = codigo del handler
    t[k++] = ppc_mtctr(12);

    // Parcheamos el LR mas adelante, cuando sepamos la direccion del stub.
    const size_t lr_hi = k;
    const size_t lr_lo = k + 1;
    k += 2;
    t[k++] = ppc_mtlr(11);           // LR = stub de restauracion
    emit_abs(t, k, 3, args_addr);    // r3 = Vox3HookArgs* para debug
    t[k++] = PPC_BCTR;               // salta al handler; el 'blr' vuelve al stub

    // --- stub de restauracion ---
    const size_t restore_index = k;
    emit_abs(t, k, 12, save_addr);
    t[k++] = ppc_ld(0, 12, 0);       // recupera r0 (LR original)
    t[k++] = ppc_ld(2, 12, 8);       // recupera r2 (TOC del juego)

    // El handler es C++ y clobbera volátiles: restaurar los argumentos que
    // el 'bl 0x005DBBD0' original todavía necesita.
    emit_abs(t, k, 12, args_addr);
    t[k++] = ppc_ld(3, 12, 0);
    t[k++] = ppc_ld(4, 12, 8);
    t[k++] = ppc_ld(5, 12, 16);
    t[k++] = ppc_ld(6, 12, 24);
    t[k++] = ppc_ld(7, 12, 32);
    t[k++] = ppc_ld(8, 12, 40);
    t[k++] = ppc_ld(9, 12, 48);
    t[k++] = ppc_ld(10, 12, 56);
    t[k++] = ppc_lfd(1, 12, 64);     // recupera f1 (partialTicks)

    // Decodifica el 'bl' original en target+12 y lo reemite de forma absoluta.
    const uint32_t bl = rd_be32(g_hook.original + 12);
    const uint32_t bl_ret = target + 16;
    uint32_t bl_target = bl_ret;
    if ((bl & 0xFC000000u) == 0x48000000u) {   // opcode 18: b / bl / ba / bla
        uint32_t imm = bl & 0x03FFFFFCu;
        const bool absolute = (bl & 0x00000002u) != 0;  // AA
        int32_t delta = (int32_t)imm;
        if (imm & 0x02000000u) delta = (int32_t)(imm | 0xFC000000u);
        bl_target = absolute ? imm
                             : (uint32_t)((int32_t)(target + 12) + delta);
    }

    emit_abs(t, k, 11, bl_target);
    t[k++] = ppc_mtctr(11);
    emit_abs(t, k, 12, bl_ret);      // el callee hara blr a target+16
    t[k++] = ppc_mtlr(12);
    t[k++] = PPC_BCTR;

    const uint32_t tramp_base = (uint32_t)(uintptr_t)t;
    const uint32_t restore_addr = tramp_base + (uint32_t)restore_index * 4;
    t[lr_hi] = ppc_lis(11, (uint16_t)(restore_addr >> 16));
    t[lr_lo] = ppc_ori(11, 11, (uint16_t)(restore_addr & 0xFFFF));

    vox3_flush_icache(t, sizeof(g_hook.trampoline));
}

int vox3_hook_install(uint32_t target, Vox3RenderHandler handler) {
    if (g_hook.installed || !handler) return -1;

    g_hook.target = target;
    memcpy(g_hook.original, (const void*)(uintptr_t)target, 16);
    build_trampoline(target, handler);

    const uint32_t tramp = (uint32_t)(uintptr_t)g_hook.trampoline;
    uint32_t patch[4];
    patch[0] = ppc_lis(11, (uint16_t)(tramp >> 16));
    patch[1] = ppc_ori(11, 11, (uint16_t)(tramp & 0xFFFF));
    patch[2] = ppc_mtctr(11);
    patch[3] = PPC_BCTR;

    if (!vox3_write_memory((void*)(uintptr_t)target, patch, sizeof(patch))) {
        return -2;
    }
    vox3_flush_icache((void*)(uintptr_t)target, sizeof(patch));

    g_hook.installed = true;
    return 0;
}

int vox3_hook_remove(void) {
    if (!g_hook.installed) return 0;
    if (!vox3_write_memory((void*)(uintptr_t)g_hook.target,
                           g_hook.original, sizeof(g_hook.original))) {
        return -1;
    }
    vox3_flush_icache((void*)(uintptr_t)g_hook.target, sizeof(g_hook.original));
    g_hook.installed = false;
    return 0;
}
