
    .syntax unified
    .cpu cortex-a9
    .arm
    .global _start

/* Simple helper macro for 32-bit MMIO writes */
.macro WRITE addr, value
    ldr r0, =\addr
    ldr r1, =\value
    str r1, [r0]
.endm

/* Optional tiny delay to let writes settle (very conservative) */
.macro SPIN count
    ldr r2, =\count
1:  subs r2, r2, #1
    bne 1b
.endm

_start:
    /* === Ducati / M3 reset + boot address setup === */
    WRITE 0x4A008920, 0x00000001     /* PM_ULPWKUP_RSTTIME_REG: de-assert reset isolation (per platform) */
    WRITE 0x4A008900, 0x00000002     /* PM_ULPWKUP_PWRSTATECTRL: enable domain (if required) */
    WRITE 0x4A307100, 0x00007C00     /* RSTCTRL (or BOOTADDR) for M3: set boot vector (per your sequence) */

    /* Follow the documented reset sequence for RM_MPU_M3_RSTCTRL (address 0x4A306910) */
    WRITE 0x4A306910, 0x00000007     /* assert local + global + cold reset */
    WRITE 0x4A306910, 0x00000003     /* release local/global, keep cold as per script */

/* TODO
    # 6. Wait until RST3 (bit 2) is latched in RM_MPU_M3_RSTST
    md.l 0x4A306914       # Look for bit 2 == 1 (value = 0x4)
*/
    SPIN 100000

    WRITE 0x4A306914, 0x00000004     /* RM_MPU_M3_RSTST: clear reset status bits */

    /* === Ducati MMU/TLB programming window (0x550820xx) === */
    WRITE 0x55082044, 0x00000000     /* CNTL: MMU disable */
    WRITE 0x55082050, 0x00000000     /* LOCK: base=0, victim=0 */

    /* 1MiB mapping: VA 0x00000000 -> PA 0x40300000 (M3 SRAM) */
    WRITE 0x55082058, 0x00000004     /* CAM: VA 0x00000000, V=1, 1MiB */
    WRITE 0x5508205C, 0x40300000     /* RAM: PA 0x40300000 */
    WRITE 0x55082054, 0x00000001     /* LD_TLB */
    WRITE 0x55082050, 0x00000410     /* LOCK: BASEVALUE=1, CURRENTVICTIM=1 */

    /* 4KiB mapping for UART3 regs at 0x48020000 */
    WRITE 0x55082058, 0x48020006     /* CAM: V=1, PGSZ=4KB, VATAG=0x48020000 */
    WRITE 0x5508205C, 0x48020000     /* RAM: PA (ENDIAN=0, ELSZ=8, MIXED=0) */
    WRITE 0x55082054, 0x00000001     /* LD_TLB */
    WRITE 0x55082050, 0x00000820     /* LOCK: base=2, vict=2 (protect 0..1) */

    WRITE 0x55082044, 0x00000002     /* CNTL: MMU_EN = 1 */

    /* === Populate M3 SRAM bootstrap at 0x40300000 === */
    WRITE 0x40300000, 0x00010000     /* [0] SP = 0x00010000 (any valid RAM top) */
    WRITE 0x40300004, 0x00000101     /* [4] PC = 0x00000100 | Thumb */
    WRITE 0x40300100, 0x21214803     /* LDR r0,[PC,#12]; MOVS r1,#33 ('!') */
    WRITE 0x40300104, 0xE7FE7001     /* STRB r1,[r0,#0]; B . */
    WRITE 0x40300110, 0x48020000     /* UART3 base */

    /* === Finally release the M3 from reset to start executing === */
    WRITE 0x4A306910, 0x00000002     /* release cold reset only */

    b .
