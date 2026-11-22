/* Cargador SPRX para Minecraft PS3 (BLES01976).
 * Se inyecta en 0x2c8250 mediante RPCS3 patch engine calloc.
 * La ruta NO esta como ASCII inline: se construye con instrucciones PPU
 * dentro del frame de pila, porque RPCS3 decoda todo el cave como codigo.
 */
    .section .text
    .globl  loader_start
    .align  4
loader_start:
    /* --- prologo: salvar estado --- */
    mflr    %r0
    stdu    %r1, -0x100(%r1)
    std     %r0, 0x10(%r1)          /* LR */
    std     %r2, 0x18(%r1)          /* TOC */
    std     %r3, 0x20(%r1)
    std     %r4, 0x28(%r1)
    std     %r5, 0x30(%r1)
    std     %r6, 0x38(%r1)
    std     %r7, 0x40(%r1)
    std     %r8, 0x48(%r1)
    std     %r9, 0x50(%r1)
    std     %r10, 0x58(%r1)
    std     %r11, 0x60(%r1)
    std     %r12, 0x68(%r1)
    mfctr   %r0
    std     %r0, 0x70(%r1)          /* CTR */
    mfcr    %r0
    std     %r0, 0x78(%r1)          /* CR */
    mfxer   %r0
    std     %r0, 0x80(%r1)          /* XER */

    /* --- construir la ruta en la pila (0x90..0xcf) --- */
    addi    %r9, %r1, 0x90
@@PATH_BUILD@@

    /* --- sys_prx_load_module(path, 0, NULL) : syscall 480 --- */
    li      %r4, 0
    li      %r5, 0
    li      %r6, 0
    li      %r11, 480
    sc
    std     %r3, 0x88(%r1)          /* id */

    /* --- sys_prx_start_module(id,0,NULL,NULL,0,NULL) : syscall 481 --- */
    ld      %r3, 0x88(%r1)
    li      %r4, 0
    li      %r5, 0
    li      %r6, 0
    li      %r7, 0
    li      %r8, 0
    li      %r9, 0
    li      %r10, 0
    li      %r11, 481
    sc

    /* --- epilogo: restaurar estado --- */
    ld      %r0, 0x80(%r1)
    mtxer   %r0
    ld      %r0, 0x78(%r1)
    mtcr    %r0
    ld      %r0, 0x70(%r1)
    mtctr   %r0
    ld      %r2, 0x18(%r1)
    ld      %r4, 0x28(%r1)
    ld      %r5, 0x30(%r1)
    ld      %r6, 0x38(%r1)
    ld      %r7, 0x40(%r1)
    ld      %r8, 0x48(%r1)
    ld      %r9, 0x50(%r1)
    ld      %r10, 0x58(%r1)
    ld      %r11, 0x60(%r1)
    ld      %r12, 0x68(%r1)
    ld      %r3, 0x20(%r1)
    ld      %r0, 0x10(%r1)
    mtlr    %r0
    addi    %r1, %r1, 0x100

    /* --- instruccion original de 0x2c8250 --- */
    li      %r2, 0

    .section .note.GNU-stack,"",@progbits
