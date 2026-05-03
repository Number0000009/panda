/*
 * A9 app at 0x82000000
 * IVA-HD power/clock/reset bring-up
 * ICONT1 firmware loaded to 0x5A008000
 * ICONT2 firmware loaded to 0x5A018000
 * ICONT1 DMem debug ring at 0x5A000000
 * ICONT2 DMem debug ring at 0x5A010000
*/

#include <stdint.h>

#define REG8(addr)  (*(volatile uint8_t  *)(uintptr_t)(addr))
#define REG32(addr) (*(volatile uint32_t *)(uintptr_t)(addr))

extern const uint8_t _binary_icont_bin_start[];
extern const uint8_t _binary_icont_bin_end[];

static inline void dsb(void) { __asm__ volatile ("dsb sy" ::: "memory"); }
static inline void dmb(void) { __asm__ volatile ("dmb sy" ::: "memory"); }
static inline void isb(void) { __asm__ volatile ("isb sy" ::: "memory"); }

static inline uint8_t rd8(uint32_t addr)
{
    return REG8(addr);
}

static inline uint32_t rd32(uint32_t addr)
{
    return REG32(addr);
}

static inline void wr32(uint32_t addr, uint32_t value)
{
    REG32(addr) = value;
    dsb();
}

static inline void rmw32(uint32_t addr, uint32_t mask, uint32_t value)
{
    uint32_t v = rd32(addr);
    v &= ~mask;
    v |= value & mask;
    wr32(addr, v);
}

static void delay(volatile uint32_t n)
{
    while (n--) {
        __asm__ volatile ("nop");
    }
}

/* UART3 / PandaBoard console */
#define UART3_BASE      0x48020000u
#define UART_THR        0x00u
#define UART_LSR        0x14u
#define UART_LSR_THRE   (1u << 5)

static void uart_putc(char c)
{
    if (c == '\n') {
        uart_putc('\r');
    }

    while ((rd32(UART3_BASE + UART_LSR) & UART_LSR_THRE) == 0) {
    }

    wr32(UART3_BASE + UART_THR, (uint32_t)c);
}

static void uart_puts(const char *s)
{
    while (*s) {
        uart_putc(*s++);
    }
}

static void uart_hex32(uint32_t v)
{
    static const char h[] = "0123456789ABCDEF";

    uart_puts("0x");

    for (int i = 7; i >= 0; i--) {
        uart_putc(h[(v >> (i * 4)) & 0xF]);
    }
}

static void uart_dec(uint32_t v)
{
    char buf[11];
    int i = 0;

    if (v == 0) {
        uart_putc('0');
        return;
    }

    while (v && i < 10) {
        buf[i++] = (char)('0' + (v % 10));
        v /= 10;
    }

    while (i--) {
        uart_putc(buf[i]);
    }
}

static void uart_int(int32_t v)
{
    if (v < 0) {
        uart_putc('-');
        uart_dec((uint32_t)(-v));
    } else {
        uart_dec((uint32_t)v);
    }
}

/* A9 D-cache clean by MVA */
static inline void dcache_clean_mva(uint32_t mva)
{
    __asm__ volatile ("mcr p15, 0, %0, c7, c10, 1"
                      :
                      : "r"(mva)
                      : "memory");
}

static void dcache_clean_range(uint32_t start, uint32_t size)
{
    uint32_t p = start & ~31u;
    uint32_t e = (start + size + 31u) & ~31u;

    while (p < e) {
        dcache_clean_mva(p);
        p += 32;
    }

    dsb();
}

/* IVAHD CM2 registers */
#define CM_IVAHD_CLKSTCTRL        0x4A008F00u
#define CM_IVAHD_STATICDEP        0x4A008F04u
#define CM_IVAHD_IVAHD_CLKCTRL    0x4A008F20u
#define CM_IVAHD_SL2_CLKCTRL      0x4A008F28u

/* IVAHD PRM registers */
#define PM_IVAHD_PWRSTCTRL        0x4A306F00u
#define PM_IVAHD_PWRSTST          0x4A306F04u
#define RM_IVAHD_RSTCTRL          0x4A306F10u
#define RM_IVAHD_RSTST            0x4A306F14u

/* A9 / L3-visible ICONT memory windows */
#define ICONT1_DMEM               0x5A000000u
#define ICONT1_IMEM               0x5A008000u
#define ICONT2_DMEM               0x5A010000u
#define ICONT2_IMEM               0x5A018000u

/* DMem debug layout */
#define DBG_MAGIC                 0x00u
#define DBG_COUNTER               0x04u
#define DBG_HEAD                  0x08u
#define DBG_TAIL                  0x0Cu

#define DBG_CFG_MAGIC             0x20u
#define DBG_CFG_IDCHAR            0x24u

#define DBG_TXBUF                 0x100u
#define DBG_TX_MASK               0xFFu

#define IVAHD_RST_ICONT1          (1u << 0)
#define IVAHD_RST_ICONT2          (1u << 1)
#define IVAHD_RST_LOGIC_SL2       (1u << 2)
#define IVAHD_RST_ALL             (IVAHD_RST_ICONT1 | IVAHD_RST_ICONT2 | IVAHD_RST_LOGIC_SL2)

#define CLKTRCTRL_MASK            0x3u
#define CLKTRCTRL_SW_WKUP         0x2u

#define MODULEMODE_MASK           0x3u
#define MODULEMODE_AUTO           0x1u

#define IDLEST_MASK               (0x3u << 16)
#define IDLEST_SHIFT              16
#define IDLEST_DISABLED           0x3u

#define CLKACTIVITY_IVAHD_CLK     (1u << 8)

#define IVAHD_STATDEP_MEMIF       (1u << 4)
#define IVAHD_STATDEP_L3_1        (1u << 5)

#define POWERSTATE_MASK           0x3u
#define POWERSTATE_ON             0x3u

#define ICONT_MAGIC_1             0x968E0001u
#define ICONT_MAGIC_2             0x968E0002u

volatile uint32_t g_icont1_magic;
volatile uint32_t g_icont1_counter;
volatile uint32_t g_icont1_head;
volatile uint32_t g_icont1_tail;

volatile uint32_t g_icont2_magic;
volatile uint32_t g_icont2_counter;
volatile uint32_t g_icont2_head;
volatile uint32_t g_icont2_tail;

volatile int32_t g_result;

static void print_regs(void)
{
    uart_puts("CM_IVAHD_CLKSTCTRL     = "); uart_hex32(rd32(CM_IVAHD_CLKSTCTRL)); uart_puts("\n");
    uart_puts("CM_IVAHD_STATICDEP     = "); uart_hex32(rd32(CM_IVAHD_STATICDEP)); uart_puts("\n");
    uart_puts("CM_IVAHD_IVAHD_CLKCTRL = "); uart_hex32(rd32(CM_IVAHD_IVAHD_CLKCTRL)); uart_puts("\n");
    uart_puts("CM_IVAHD_SL2_CLKCTRL   = "); uart_hex32(rd32(CM_IVAHD_SL2_CLKCTRL)); uart_puts("\n");
    uart_puts("PM_IVAHD_PWRSTCTRL     = "); uart_hex32(rd32(PM_IVAHD_PWRSTCTRL)); uart_puts("\n");
    uart_puts("PM_IVAHD_PWRSTST       = "); uart_hex32(rd32(PM_IVAHD_PWRSTST)); uart_puts("\n");
    uart_puts("RM_IVAHD_RSTCTRL       = "); uart_hex32(rd32(RM_IVAHD_RSTCTRL)); uart_puts("\n");
    uart_puts("RM_IVAHD_RSTST         = "); uart_hex32(rd32(RM_IVAHD_RSTST)); uart_puts("\n");
}

static int wait_bits_set(uint32_t addr, uint32_t bits, const char *name)
{
    for (uint32_t i = 0; i < 1000000; i++) {
        if ((rd32(addr) & bits) == bits) {
            return 0;
        }
    }

    uart_puts("TIMEOUT bits_set ");
    uart_puts(name);
    uart_puts(" val=");
    uart_hex32(rd32(addr));
    uart_puts(" wanted=");
    uart_hex32(bits);
    uart_puts("\n");

    return -1;
}

static int wait_idlest_not_disabled(uint32_t addr, const char *name)
{
    for (uint32_t i = 0; i < 1000000; i++) {
        uint32_t idlest = (rd32(addr) & IDLEST_MASK) >> IDLEST_SHIFT;

        if (idlest != IDLEST_DISABLED) {
            return 0;
        }
    }

    uart_puts("TIMEOUT idlest ");
    uart_puts(name);
    uart_puts(" val=");
    uart_hex32(rd32(addr));
    uart_puts("\n");

    return -1;
}

static int wait_power_not_off(void)
{
    for (uint32_t i = 0; i < 1000000; i++) {
        uint32_t p = rd32(PM_IVAHD_PWRSTST) & POWERSTATE_MASK;

        if (p != 0) {
            return 0;
        }
    }

    uart_puts("TIMEOUT power val=");
    uart_hex32(rd32(PM_IVAHD_PWRSTST));
    uart_puts("\n");

    return -1;
}

static void clear_words(uint32_t addr, uint32_t words)
{
    volatile uint32_t *p = (volatile uint32_t *)(uintptr_t)addr;

    for (uint32_t i = 0; i < words; i++) {
        p[i] = 0;
    }

    dsb();
}

static void prepare_icont_dmem(uint32_t dmem, uint32_t magic, char idchar)
{
    clear_words(dmem, 256);

    wr32(dmem + DBG_CFG_MAGIC, magic);
    wr32(dmem + DBG_CFG_IDCHAR, (uint32_t)idchar);

    dcache_clean_range(dmem, 1024);
}

static void load_icont_imem(uint32_t imem)
{
    const uint8_t *src = _binary_icont_bin_start;
    const uint8_t *end = _binary_icont_bin_end;
    uint32_t size = (uint32_t)(end - src);

    volatile uint32_t *dst = (volatile uint32_t *)(uintptr_t)imem;

    for (uint32_t i = 0; i < size; i += 4) {
        uint32_t v = 0;

        if (i + 0 < size) v |= ((uint32_t)src[i + 0]) << 0;
        if (i + 1 < size) v |= ((uint32_t)src[i + 1]) << 8;
        if (i + 2 < size) v |= ((uint32_t)src[i + 2]) << 16;
        if (i + 3 < size) v |= ((uint32_t)src[i + 3]) << 24;

        dst[i >> 2] = v;
    }

    dcache_clean_range(imem, size);
    dsb();
}

static void drain_icont_log(uint32_t dmem)
{
    uint32_t head = rd32(dmem + DBG_HEAD) & DBG_TX_MASK;
    uint32_t tail = rd32(dmem + DBG_TAIL) & DBG_TX_MASK;

    while (tail != head) {
        char c = (char)rd8(dmem + DBG_TXBUF + tail);

        tail = (tail + 1u) & DBG_TX_MASK;

        /*
         * Publish new tail before slow UART output.
         */
        wr32(dmem + DBG_TAIL, tail);

        uart_putc(c);

        head = rd32(dmem + DBG_HEAD) & DBG_TX_MASK;
    }
}

static void snapshot_icont_state(void)
{
    g_icont1_magic   = rd32(ICONT1_DMEM + DBG_MAGIC);
    g_icont1_counter = rd32(ICONT1_DMEM + DBG_COUNTER);
    g_icont1_head    = rd32(ICONT1_DMEM + DBG_HEAD);
    g_icont1_tail    = rd32(ICONT1_DMEM + DBG_TAIL);

    g_icont2_magic   = rd32(ICONT2_DMEM + DBG_MAGIC);
    g_icont2_counter = rd32(ICONT2_DMEM + DBG_COUNTER);
    g_icont2_head    = rd32(ICONT2_DMEM + DBG_HEAD);
    g_icont2_tail    = rd32(ICONT2_DMEM + DBG_TAIL);
}

static void print_icont_state(void)
{
    snapshot_icont_state();

    uart_puts("[A9] ICONT1 magic=");
    uart_hex32(g_icont1_magic);
    uart_puts(" counter=");
    uart_hex32(g_icont1_counter);
    uart_puts(" head=");
    uart_hex32(g_icont1_head);
    uart_puts(" tail=");
    uart_hex32(g_icont1_tail);

    uart_puts(" | ICONT2 magic=");
    uart_hex32(g_icont2_magic);
    uart_puts(" counter=");
    uart_hex32(g_icont2_counter);
    uart_puts(" head=");
    uart_hex32(g_icont2_head);
    uart_puts(" tail=");
    uart_hex32(g_icont2_tail);
    uart_puts("\n");
}

static int ivahd_power_clock_on(void)
{
    uart_puts("IVAHD: initial regs\n");
    print_regs();

    uart_puts("IVAHD: power ON\n");
    rmw32(PM_IVAHD_PWRSTCTRL, POWERSTATE_MASK, POWERSTATE_ON);

    uart_puts("IVAHD: static deps\n");
    rmw32(CM_IVAHD_STATICDEP,
          IVAHD_STATDEP_MEMIF | IVAHD_STATDEP_L3_1,
          IVAHD_STATDEP_MEMIF | IVAHD_STATDEP_L3_1);

    uart_puts("IVAHD: clock domain SW_WKUP\n");
    rmw32(CM_IVAHD_CLKSTCTRL, CLKTRCTRL_MASK, CLKTRCTRL_SW_WKUP);

    uart_puts("IVAHD: modulemode AUTO\n");
    rmw32(CM_IVAHD_IVAHD_CLKCTRL, MODULEMODE_MASK, MODULEMODE_AUTO);
    rmw32(CM_IVAHD_SL2_CLKCTRL,   MODULEMODE_MASK, MODULEMODE_AUTO);

    delay(10000);

    uart_puts("IVAHD: after clock writes\n");
    print_regs();

    if (wait_power_not_off() < 0) {
        print_regs();
        return -101;
    }

    if (wait_bits_set(CM_IVAHD_CLKSTCTRL, CLKACTIVITY_IVAHD_CLK, "IVAHD_CLKACTIVITY") < 0) {
        print_regs();
        return -102;
    }

    if (wait_idlest_not_disabled(CM_IVAHD_IVAHD_CLKCTRL, "IVAHD_CLKCTRL") < 0) {
        print_regs();
        return -103;
    }

    if (wait_idlest_not_disabled(CM_IVAHD_SL2_CLKCTRL, "SL2_CLKCTRL") < 0) {
        print_regs();
        return -104;
    }

    uart_puts("IVAHD: power/clock OK\n");
    print_regs();

    return 0;
}

static int boot_arm968_both(void)
{
    int r;

    r = ivahd_power_clock_on();
    if (r < 0) {
        return r;
    }

    uart_puts("IVAHD: assert all resets\n");
    rmw32(RM_IVAHD_RSTCTRL, IVAHD_RST_ALL, IVAHD_RST_ALL);
    delay(10000);
    print_regs();

    uart_puts("IVAHD: clear old reset status\n");
    wr32(RM_IVAHD_RSTST, IVAHD_RST_ALL);

    uart_puts("IVAHD: release logic/SL2 reset only\n");
    rmw32(RM_IVAHD_RSTCTRL, IVAHD_RST_LOGIC_SL2, 0);
    delay(10000);
    print_regs();

    uart_puts("ICONT: prepare DMem config\n");
    prepare_icont_dmem(ICONT1_DMEM, ICONT_MAGIC_1, '1');
    prepare_icont_dmem(ICONT2_DMEM, ICONT_MAGIC_2, '2');

    uart_puts("ICONT: load IMem firmware\n");
    load_icont_imem(ICONT1_IMEM);
    load_icont_imem(ICONT2_IMEM);

    dsb();
    isb();

    uart_puts("ICONT1: release reset\n");
    rmw32(RM_IVAHD_RSTCTRL, IVAHD_RST_ICONT1, 0);
    delay(10000);
    print_regs();

    uart_puts("ICONT2: release reset\n");
    rmw32(RM_IVAHD_RSTCTRL, IVAHD_RST_ICONT2, 0);
    delay(10000);
    print_regs();

    uart_puts("ICONT: polling magic and draining DMem logs\n");

    for (uint32_t i = 0; i < 10000000; i++) {
        drain_icont_log(ICONT1_DMEM);
        drain_icont_log(ICONT2_DMEM);

        snapshot_icont_state();

        if (g_icont1_magic == ICONT_MAGIC_1 &&
            g_icont2_magic == ICONT_MAGIC_2) {
            uart_puts("ICONT: both magic values OK\n");
            print_icont_state();
            return 0;
        }
    }

    uart_puts("ICONT: magic timeout\n");
    print_regs();
    print_icont_state();

    return -200;
}

int main(void)
{
    int r;

    uart_puts("\n\nARM968 / ICONT DMem debug-channel test\n");
    uart_puts("A9 app linked/running at 0x82000000\n");

    r = boot_arm968_both();
    g_result = r;

    uart_puts("boot_arm968_both result = ");
    uart_int(r);
    uart_puts("\n");

    uint32_t loops = 0;

    while (1) {
        drain_icont_log(ICONT1_DMEM);
        drain_icont_log(ICONT2_DMEM);

        if ((loops++ & 0x3FFFFu) == 0) {
            print_icont_state();
        }

        dmb();
    }
}
