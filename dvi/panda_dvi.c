#include <stdint.h>

typedef volatile uint32_t vu32;
typedef volatile uint16_t vu16;

static inline void writel(uint32_t v, uint32_t a) { *(vu32 *)a = v; }
static inline uint32_t readl(uint32_t a) { return *(vu32 *)a; }
static inline void writew(uint16_t v, uint32_t a) { *(vu16 *)a = v; }

static inline void delay(volatile uint32_t n)
{
    while (n--) __asm__ volatile ("nop");
}

static int wait_set(uint32_t addr, uint32_t mask)
{
    for (uint32_t i = 0; i < 1000000; i++) {
        if ((readl(addr) & mask) == mask) return 0;
    }
    return -1;
}

static int wait_clear(uint32_t addr, uint32_t mask)
{
    for (uint32_t i = 0; i < 1000000; i++) {
        if ((readl(addr) & mask) == 0) return 0;
    }
    return -1;
}

static int wait_idlest(uint32_t addr)
{
    for (uint32_t i = 0; i < 1000000; i++) {
        if ((readl(addr) & (3u << 16)) == 0) return 0;
    }
    return -1;
}

/* ------------------------------------------------------------------------- */

#define FB_BASE     0x81000000u
#define FB_W        640u
#define FB_H        480u

static void framebuffer_init_once(void)
{
    volatile uint16_t *fb = (volatile uint16_t *)FB_BASE;

    for (uint32_t y = 0; y < FB_H; y++) {
        for (uint32_t x = 0; x < FB_W; x++) {
            fb[y * FB_W + x] = 0x001f;   /* blue */
        }
    }

    fb[(FB_H / 2u) * FB_W + (FB_W / 2u)] = 0xffff;   /* white dot */

    __asm__ volatile ("dsb" ::: "memory");
}

static void framebuffer_selftest(void)
{
    volatile uint16_t *fb = (volatile uint16_t *)FB_BASE;

    fb[0] = 0xf800;
    fb[1] = 0x07e0;
    fb[2] = 0x001f;
    fb[3] = 0xffff;

    __asm__ volatile ("dsb" ::: "memory");

    if (fb[0] != 0xf800 ||
        fb[1] != 0x07e0 ||
        fb[2] != 0x001f ||
        fb[3] != 0xffff) {
        while (1) {}
    }
}

/* ------------------------------------------------------------------------- */

#define CM_DSS_CLKSTCTRL        0x4A009100u
#define CM_DSS_DSS_CLKCTRL      0x4A009120u

#define CM_WKUP_CLKSTCTRL       0x4A307800u
#define CM_WKUP_GPIO1_CLKCTRL   0x4A307838u

static void enable_gpio1_clock(void)
{
    writel(0x2, CM_WKUP_CLKSTCTRL);
    writel(0x1, CM_WKUP_GPIO1_CLKCTRL);
    wait_idlest(CM_WKUP_GPIO1_CLKCTRL);
}

static void enable_dss_clock(void)
{
    writel(0x2, CM_DSS_CLKSTCTRL);
    writel((1u << 10) | (1u << 9) | (1u << 8) | 0x2u,
           CM_DSS_DSS_CLKCTRL);
    wait_idlest(CM_DSS_DSS_CLKCTRL);
}

/* ------------------------------------------------------------------------- */

#define OMAP4_PADCONF_CORE_BASE 0x4A100000u

#define MUX_MODE3               0x0003u
#define MUX_MODE5               0x0005u
#define PIN_OUTPUT              0x0000u

static const uint16_t dvi_dpi_pad_offsets[] = {
    0x162, 0x164, 0x166, 0x168, 0x16a, 0x16c,
    0x16e, 0x170, 0x172, 0x174, 0x176,
    0x1b4, 0x1b6, 0x1b8, 0x1ba,
    0x1bc, 0x1be, 0x1c0, 0x1c2,
    0x1c4, 0x1c6, 0x1c8, 0x1ca, 0x1cc,
    0x1ce, 0x1d0, 0x1d2, 0x1d4
};

static void pinmux_dvi(void)
{
    for (uint32_t i = 0;
         i < sizeof(dvi_dpi_pad_offsets) / sizeof(dvi_dpi_pad_offsets[0]);
         i++) {
        writew(PIN_OUTPUT | MUX_MODE5,
               OMAP4_PADCONF_CORE_BASE + dvi_dpi_pad_offsets[i]);
    }

    writew(PIN_OUTPUT | MUX_MODE3, OMAP4_PADCONF_CORE_BASE + 0x184);
}

/* ------------------------------------------------------------------------- */

#define GPIO1_BASE              0x4A310000u
#define GPIO_OE                 0x134
#define GPIO_SETDATAOUT         0x194

static void enable_tfp410(void)
{
    uint32_t oe = readl(GPIO1_BASE + GPIO_OE);

    oe &= ~(1u << 0);
    writel(oe, GPIO1_BASE + GPIO_OE);
    writel(1u << 0, GPIO1_BASE + GPIO_SETDATAOUT);

    delay(100000);
}

/* ------------------------------------------------------------------------- */

#define DSS_BASE                0x58000000u
#define DSS_CTRL                0x0040

#define DISPC_BASE              0x58001000u

#define DISPC_SYSCONFIG         0x0010
#define DISPC_SYSSTATUS         0x0014
#define DISPC_IRQSTATUS         0x0018

#define DISPC_CONTROL2          0x0238
#define DISPC_CONFIG2           0x0620
#define DISPC_DEFAULT_COLOR2    0x03AC

#define DISPC_SIZE_LCD2         0x03CC
#define DISPC_TIMING_H2         0x0400
#define DISPC_TIMING_V2         0x0404
#define DISPC_POL_FREQ2         0x0408
#define DISPC_DIVISOR2          0x040C

#define DISPC_VID1_BA0          0x00BC
#define DISPC_VID1_BA1          0x00C0
#define DISPC_VID1_POSITION     0x00C4
#define DISPC_VID1_SIZE         0x00C8
#define DISPC_VID1_ATTRIBUTES   0x00CC
#define DISPC_VID1_BUF_THRESHOLD 0x00D0
#define DISPC_VID1_ROW_INC      0x00DC
#define DISPC_VID1_PIXEL_INC    0x00E0
#define DISPC_VID1_PICTURE_SIZE 0x00E4
#define DISPC_VID1_FIR          0x00E8
#define DISPC_VID1_PRELOAD      0x0230

static uint32_t pack_size(uint32_t w, uint32_t h)
{
    return ((h - 1u) << 16) | (w - 1u);
}

static uint32_t pack_timing_h(uint32_t hsw, uint32_t hfp, uint32_t hbp)
{
    return ((hbp - 1u) << 20) |
           ((hfp - 1u) << 8)  |
           ((hsw - 1u) << 0);
}

static uint32_t pack_timing_v(uint32_t vsw, uint32_t vfp, uint32_t vbp)
{
    return (vbp << 20) |
           (vfp << 8)  |
           ((vsw - 1u) << 0);
}

static void dss_route_lcd2_parallel(void)
{
    uint32_t v = readl(DSS_BASE + DSS_CTRL);

    v &= ~(1u << 17);
    v &= ~(1u << 12);
    v &= ~(3u << 8);

    writel(v, DSS_BASE + DSS_CTRL);
}

static void dispc_reset(void)
{
    writel(1u << 1, DISPC_BASE + DISPC_SYSCONFIG);
    wait_set(DISPC_BASE + DISPC_SYSSTATUS, 1u << 0);

    writel(1u << 3, DISPC_BASE + DISPC_SYSCONFIG);
    writel(0xffffffffu, DISPC_BASE + DISPC_IRQSTATUS);
}

static void dispc_disable(void)
{
    writel(0x00000000u, DISPC_BASE + DISPC_CONTROL2);
    writel(0x00000000u, DISPC_BASE + DISPC_VID1_ATTRIBUTES);
    delay(100000);
}

static void dispc_lcd2_config(void)
{
    writel(pack_size(640, 480), DISPC_BASE + DISPC_SIZE_LCD2);
    writel(pack_timing_h(96, 16, 48), DISPC_BASE + DISPC_TIMING_H2);
    writel(pack_timing_v(2, 10, 33), DISPC_BASE + DISPC_TIMING_V2);

    writel((3u << 16) | 2u, DISPC_BASE + DISPC_DIVISOR2);
    writel((1u << 13) | (1u << 12), DISPC_BASE + DISPC_POL_FREQ2);

    writel(0x00000000u, DISPC_BASE + DISPC_DEFAULT_COLOR2);
    writel(0x00000000u, DISPC_BASE + DISPC_CONFIG2);
}

static uint32_t vid1_attr(uint32_t enable)
{
    return ((enable & 1u) << 0) |
           (0x6u << 1) |
           (0x2u << 14) |
           (0u << 16) |
           (1u << 30);
}

static void dispc_vid1_config_disabled(void)
{
    writel(0x00000000u, DISPC_BASE + DISPC_VID1_ATTRIBUTES);

    writel(FB_BASE, DISPC_BASE + DISPC_VID1_BA0);
    writel(FB_BASE, DISPC_BASE + DISPC_VID1_BA1);

    writel(0x00000000u, DISPC_BASE + DISPC_VID1_POSITION);
    writel(pack_size(FB_W, FB_H), DISPC_BASE + DISPC_VID1_SIZE);
    writel(pack_size(FB_W, FB_H), DISPC_BASE + DISPC_VID1_PICTURE_SIZE);

    writel(0x00000000u, DISPC_BASE + DISPC_VID1_FIR);

    writel(1u, DISPC_BASE + DISPC_VID1_PIXEL_INC);
    writel(1u, DISPC_BASE + DISPC_VID1_ROW_INC);

    writel(0x03c003ffu, DISPC_BASE + DISPC_VID1_BUF_THRESHOLD);
    writel(0x00000000u, DISPC_BASE + DISPC_VID1_PRELOAD);

    writel(vid1_attr(0), DISPC_BASE + DISPC_VID1_ATTRIBUTES);
}

static void dispc_commit(void)
{
    uint32_t v = 0;

    v |= (1u << 0);
    v |= (1u << 3);
    v |= (1u << 5);
    v |= (3u << 8);

    writel(v, DISPC_BASE + DISPC_CONTROL2);
    wait_clear(DISPC_BASE + DISPC_CONTROL2, 1u << 5);
}

static void dispc_vid1_enable(void)
{
    writel(0xffffffffu, DISPC_BASE + DISPC_IRQSTATUS);
    writel(vid1_attr(1), DISPC_BASE + DISPC_VID1_ATTRIBUTES);
    dispc_commit();
}

/* ------------------------------------------------------------------------- */

void main(void)
{
    framebuffer_selftest();
    framebuffer_init_once();

    enable_gpio1_clock();
    enable_dss_clock();

    pinmux_dvi();
    enable_tfp410();

    dss_route_lcd2_parallel();

    dispc_reset();
    dispc_disable();

    dispc_lcd2_config();
    dispc_vid1_config_disabled();

    dispc_commit();
    dispc_vid1_enable();

    while (1) {
        delay(1000000);
    }
}
