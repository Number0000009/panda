.global _start
.extern main

_start:
    cpsid if

    /* Disable MMU, D-cache, I-cache. */
    mrc p15, 0, r0, c1, c0, 0
    bic r0, r0, #(1 << 0)      @ MMU off
    bic r0, r0, #(1 << 2)      @ D-cache off
    bic r0, r0, #(1 << 12)     @ I-cache off
    mcr p15, 0, r0, c1, c0, 0
    dsb
    isb

    ldr sp, =0x88000000

    bl main

1:
    wfe
    b 1b
