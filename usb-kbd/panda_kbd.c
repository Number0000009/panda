/* panda_kbd.c */
#include <stdint.h>

static inline void w32(uint32_t a, uint32_t v){ *(volatile uint32_t*)a = v; }
static inline uint32_t r32(uint32_t a){ return *(volatile uint32_t*)a; }
static inline void w16(uint32_t a, uint16_t v){ *(volatile uint16_t*)a = v; }

static void delay(volatile uint32_t n){ while(n--) __asm__ volatile("nop"); }
static void barrier(void){ __asm__ volatile("dsb; isb" ::: "memory"); }

#define UART3 0x48020000u
static void putc(char c){
    if(c == '\n') putc('\r');
    while(!(r32(UART3 + 0x14) & 0x20)){}
    w32(UART3, c);
}
static void puts(const char *s){ while(*s) putc(*s++); }
static void hex32(uint32_t v){
    for(int i = 28; i >= 0; i -= 4) putc("0123456789ABCDEF"[(v >> i) & 15]);
}
static void dump(const char *s, uint32_t a){ puts(s); puts("="); hex32(r32(a)); puts("\n"); }
static void dumpv(const char *s, uint32_t v){ puts(s); puts("="); hex32(v); puts("\n"); }
static int fail(const char *s){ puts(s); return -1; }

#define CM_WKUP_GPIO1_CLKCTRL        0x4A307838u
#define CM_L4PER_GPIO2_CLKCTRL       0x4A009560u
#define CM_L4PER_GPIO6_CLKCTRL       0x4A009580u
#define CM_L3INIT_HSUSBHOST_CLKCTRL  0x4A009358u
#define CM_L3INIT_HSUSBTLL_CLKCTRL   0x4A009368u
#define CM_L3INIT_FSUSB_CLKCTRL      0x4A0093D0u
#define CM_L3INIT_USBPHY_CLKCTRL     0x4A0093E0u
#define ALTCLKSRC_SCRM               0x4A30A110u
#define AUXCLK1_SCRM                 0x4A30A314u
#define AUXCLK3_SCRM                 0x4A30A31Cu

#define CTRL_CORE 0x4A100000u
#define CTRL_WKUP 0x4A31E000u

#define GPIO1 0x4A310000u
#define GPIO2 0x48055000u
#define GPIO_OE  0x134u
#define GPIO_SET 0x194u
#define GPIO_CLR 0x190u

#define UHH 0x4A064000u
#define UHH_SYSCONFIG   0x10u
#define UHH_HOSTCONFIG  0x40u

#define USBTLL 0x4A062000u
#define USBTLL_SYSCONFIG 0x10u
#define USBTLL_SYSSTATUS 0x14u

#define EHCI_BASE 0x4A064C00u
static uint32_t opbase;
static unsigned ehci_nports = 1;
static unsigned root_port = 0;

#define HCS_PARAMS    0x04u
#define USB_CMD       0x00u
#define USB_STS       0x04u
#define USB_INTR      0x08u
#define PERIODIC_ADDR 0x14u
#define ASYNC_ADDR    0x18u
#define CONFIG_FLAG   0x40u
#define PORTSC1       0x44u
#define INSNREG04     0xA0u
#define INSNREG05     0xA4u

#define ULPI_CTRL     (1u << 31)
#define ULPI_OP_READ  (3u << 22)
#define ULPI_OP_WRITE (2u << 22)
#define ULPI_ERROR    0x100u

#define PORT_CONNECT  0x00000001u
#define PORT_CHANGES  0x0000002Au
#define PORT_ENABLE   0x00000004u
#define PORT_RESET    0x00000100u
#define PORT_POWER    0x00001000u

typedef struct {
    uint32_t next;
    uint32_t altnext;
    uint32_t token;
    uint32_t buf[5];
} qtd_t;

typedef struct {
    uint32_t horiz;
    uint32_t epchar;
    uint32_t epcap;
    uint32_t current;
    qtd_t overlay;
} qh_t;

static volatile qh_t qh __attribute__((aligned(64)));
static volatile qtd_t td[3] __attribute__((aligned(64)));
static volatile uint32_t periodic[1024] __attribute__((aligned(4096)));
static volatile uint8_t setup[8] __attribute__((aligned(64)));
static volatile uint8_t buf[256] __attribute__((aligned(64)));
static volatile uint8_t led_report[1] __attribute__((aligned(64)));

static void memzero(volatile void *p, unsigned n){
    volatile uint8_t *x = (volatile uint8_t*)p;
    while(n--) *x++ = 0;
}

static void gpio_out(uint32_t base, unsigned bit, int v){
    uint32_t m = 1u << bit;
    w32(base + GPIO_OE, r32(base + GPIO_OE) & ~m);
    if(v) w32(base + GPIO_SET, m);
    else  w32(base + GPIO_CLR, m);
}

static void padmux(void){
    w16(CTRL_CORE + 0x08C, 0x000B);
    w16(CTRL_CORE + 0x186, 0x0003);
    w16(CTRL_CORE + 0x19A, 0x0000);
    w16(CTRL_CORE + 0x19C, 0x011B);
    w16(CTRL_WKUP + 0x058, 0x0000);

    w16(CTRL_CORE + 0x0C2, 0x010C);
    w16(CTRL_CORE + 0x0C4, 0x0004);
    w16(CTRL_CORE + 0x0C6, 0x010C);
    w16(CTRL_CORE + 0x0C8, 0x010C);
    w16(CTRL_CORE + 0x0CA, 0x010C);
    w16(CTRL_CORE + 0x0CC, 0x010C);
    w16(CTRL_CORE + 0x0CE, 0x010C);
    w16(CTRL_CORE + 0x0D0, 0x010C);
    w16(CTRL_CORE + 0x0D2, 0x010C);
    w16(CTRL_CORE + 0x0D4, 0x010C);
    w16(CTRL_CORE + 0x0D6, 0x010C);
    w16(CTRL_CORE + 0x0D8, 0x010C);
}

static int wait_clr(uint32_t a, uint32_t m){
    for(uint32_t i = 0; i < 10000000; i++)
        if(!(r32(a) & m)) return 0;
    return -1;
}

static int wait_set(uint32_t a, uint32_t m){
    for(uint32_t i = 0; i < 10000000; i++)
        if(r32(a) & m) return 0;
    return -1;
}

static void board_usb_init(void){
    puts("padmux\n");
    padmux();

    puts("clocks\n");
    w32(CM_WKUP_GPIO1_CLKCTRL, 0x2);
    w32(CM_L4PER_GPIO2_CLKCTRL, 0x2);
    w32(CM_L4PER_GPIO6_CLKCTRL, 0x2);
    w32(ALTCLKSRC_SCRM, (r32(ALTCLKSRC_SCRM) & ~3u) | 0xDu);
    w32(AUXCLK1_SCRM, 0x000F0104);
    w32(AUXCLK3_SCRM, 0x00010100);
    w32(CM_L3INIT_HSUSBHOST_CLKCTRL, 0x01000F02);
    w32(CM_L3INIT_HSUSBTLL_CLKCTRL, 0x2);
    w32(CM_L3INIT_FSUSB_CLKCTRL, 0x2);
    w32(CM_L3INIT_USBPHY_CLKCTRL, 0x00000301);
    delay(30000000);

    dump("CM_HOST ", CM_L3INIT_HSUSBHOST_CLKCTRL);
    dump("CM_TLL  ", CM_L3INIT_HSUSBTLL_CLKCTRL);

    puts("hub power/reset\n");
    gpio_out(GPIO1, 1, 0);
    gpio_out(GPIO2, 30, 0);
    delay(300000000);

    gpio_out(GPIO1, 1, 1);
    delay(300000000);

    gpio_out(GPIO2, 30, 1);
    delay(1500000000);

    puts("uhh/tll\n");
    w32(UHH + UHH_SYSCONFIG, 0x1);
    delay(1000000);
    if(wait_clr(UHH + UHH_SYSCONFIG, 0x1)) puts("UHH reset timeout\n");

    w32(USBTLL + USBTLL_SYSCONFIG, 0x2);
    if(wait_set(USBTLL + USBTLL_SYSSTATUS, 0x1)) puts("TLL reset timeout\n");
    w32(USBTLL + USBTLL_SYSCONFIG, 0x10C);

    w32(UHH + UHH_SYSCONFIG, 0x14);
    w32(UHH + UHH_HOSTCONFIG, 0x8000001C);
    delay(30000000);

    dump("UHH_HOST", UHH + UHH_HOSTCONFIG);
}

static uint32_t port_addr(unsigned port){
    return opbase + PORTSC1 + port * 4u;
}

static void dump_port(const char *s, unsigned port){
    puts(s);
    putc('1' + port);
    puts("=");
    hex32(r32(port_addr(port)));
    puts("\n");
}

static int ulpi_wait_ctrl(void){
    for(uint32_t i = 0; i < 1000000; i++){
        if(!(r32(EHCI_BASE + INSNREG05) & ULPI_CTRL)) return 0;
        delay(10);
    }

    return -1;
}

static int ulpi_request(uint32_t v, const char *what){
    w32(EHCI_BASE + INSNREG05, v);
    if(ulpi_wait_ctrl()){
        puts(what);
        puts(" timeout\n");
        return -1;
    }

    return 0;
}

static int ulpi_write_omap(unsigned port, uint8_t reg, uint8_t val){
    uint32_t req = ULPI_CTRL |
                   (((port + 1u) & 0xfu) << 24) |
                   ULPI_OP_WRITE |
                   ((uint32_t)reg << 16) |
                   val;

    return ulpi_request(req, "ULPI write");
}

static uint32_t ulpi_read_omap(unsigned port, uint8_t reg){
    uint32_t req = ULPI_CTRL |
                   (((port + 1u) & 0xfu) << 24) |
                   ULPI_OP_READ |
                   ((uint32_t)reg << 16);

    if(ulpi_request(req, "ULPI read")) return ULPI_ERROR;
    return r32(EHCI_BASE + INSNREG05) & 0xff;
}

static int ulpi_dump(unsigned port){
    uint32_t id0 = ulpi_read_omap(port, 0x00);
    uint32_t id1 = ulpi_read_omap(port, 0x01);
    uint32_t id2 = ulpi_read_omap(port, 0x02);
    uint32_t id3 = ulpi_read_omap(port, 0x03);
    uint32_t fc = ulpi_read_omap(port, 0x04);
    uint32_t otg = ulpi_read_omap(port, 0x0A);

    if((id0 | id1 | id2 | id3 | fc | otg) & ULPI_ERROR)
        return fail("ULPI read fail\n");

    dumpv("ULPI_ID ", id0 | (id1 << 8) | (id2 << 16) | (id3 << 24));
    dumpv("ULPI_FC ", fc);
    dumpv("ULPI_OTG", otg);
    return 0;
}

static int omap_ehci_soft_phy_reset(unsigned port){
    ulpi_dump(port);

    if(ulpi_write_omap(port, 0x05, 0x20)){
        puts("ULPI reset timeout\n");
        return 0;
    }

    for(int i = 0; i < 1000; i++){
        uint32_t fc = ulpi_read_omap(port, 0x04);
        if(fc != ULPI_ERROR && !(fc & 0x20)) break;
        delay(100000);
    }

    delay(1000000);
    return 0;
}

static int ehci_init(void){
    puts("ehci\n");
    dump("CAP     ", EHCI_BASE);
    dump("HCSPARAM", EHCI_BASE + HCS_PARAMS);

    opbase = EHCI_BASE + (r32(EHCI_BASE) & 0xff);
    ehci_nports = r32(EHCI_BASE + HCS_PARAMS) & 0xf;
    if(ehci_nports == 0 || ehci_nports > 3) ehci_nports = 3;

    w32(opbase + USB_CMD, 2);
    delay(30000000);
    if(wait_clr(opbase + USB_CMD, 2)) return -1;

    w32(opbase + USB_INTR, 0);
    w32(opbase + USB_STS, 0x3f);
    w32(EHCI_BASE + INSNREG04, 0x20);
    if(omap_ehci_soft_phy_reset(0)) return -1;

    w32(opbase + PERIODIC_ADDR, 0);
    w32(opbase + ASYNC_ADDR, 0);

    w32(opbase + USB_CMD, 1);
    delay(50000000);

    w32(opbase + CONFIG_FLAG, 1);
    delay(50000000);

    for(unsigned p = 0; p < ehci_nports; p++){
        uint32_t v = r32(port_addr(p));
        w32(port_addr(p), (v & ~PORT_CHANGES) | PORT_POWER);
    }
    delay(50000000);

    return 0;
}

static void port_clear_changes(unsigned port){
    uint32_t v = r32(port_addr(port));
    v &= ~PORT_RESET;
    v |= PORT_POWER | PORT_CHANGES;
    w32(port_addr(port), v);
    delay(1000000);
}

static int root_port_reset_one(unsigned port){
    for(int i = 0; i < 20; i++){
        uint32_t p = r32(port_addr(port));

        if(!(p & PORT_CONNECT)){
            delay(100000000);
            continue;
        }

        port_clear_changes(port);
        delay(10000000);

        w32(port_addr(port), PORT_POWER | PORT_RESET);
        delay(80000000);

        w32(port_addr(port), PORT_POWER);

        for(int j = 0; j < 100; j++){
            delay(1000000);
            if(!(r32(port_addr(port)) & PORT_RESET)) break;
        }

        delay(30000000);

        if(r32(port_addr(port)) & PORT_ENABLE) return 0;
    }

    return -1;
}

static int root_port_reset(void){
    for(unsigned p = 0; p < ehci_nports; p++){
        if(!root_port_reset_one(p)){
            root_port = p;
            return 0;
        }
    }

    return -1;
}

#define PID_OUT   0
#define PID_IN    1
#define PID_SETUP 2

static uint8_t kb_iface = 0;
static uint8_t kb_mps = 8;
static uint8_t kb_addr = 2;
static uint8_t kb_speed = 0;
static uint8_t kb_hub = 1;
static uint8_t kb_hubport = 0;
static uint8_t kb_ep = 1;
static uint8_t kb_ep_mps = 8;
static uint8_t kb_toggle = 0;
static int kb_port = -1;

static void fill_td(volatile qtd_t *t, uint32_t next, uint32_t pid,
                    volatile void *data, uint32_t len, uint32_t toggle){
    uint32_t a = (uint32_t)data;

    t->next = next;
    t->altnext = 1;
    t->token = (toggle << 31) | (len << 16) | (3 << 10) | (pid << 8) | 0x80;
    t->buf[0] = a;
    t->buf[1] = (a + 0x1000) & ~0xfffu;
    t->buf[2] = (a + 0x2000) & ~0xfffu;
    t->buf[3] = (a + 0x3000) & ~0xfffu;
    t->buf[4] = (a + 0x4000) & ~0xfffu;
}

static int submit_qh(uint8_t addr, uint8_t ep, uint8_t speed, uint16_t mps,
                     uint8_t hubaddr, uint8_t hubport,
                     volatile qtd_t *first, int ctrl){
    memzero(&qh, sizeof(qh));

    qh.horiz = ((uint32_t)&qh) | 2;
    qh.epchar =
        (uint32_t)addr |
        ((uint32_t)ep << 8) |
        ((uint32_t)speed << 12) |
        (1u << 14) |
        (1u << 15) |
        ((uint32_t)mps << 16) |
        (ctrl ? (1u << 27) : 0) |
        (15u << 28);

    qh.epcap =
        ((uint32_t)hubaddr << 16) |
        ((uint32_t)hubport << 23) |
        (1u << 30);

    qh.current = 0;
    qh.overlay.next = (uint32_t)first;
    qh.overlay.altnext = 1;
    qh.overlay.token = 0;

    barrier();

    w32(opbase + ASYNC_ADDR, (uint32_t)&qh);
    barrier();

    w32(opbase + USB_STS, 0x3f);
    w32(opbase + USB_CMD, r32(opbase + USB_CMD) | (1u << 5));

    for(uint32_t i = 0; i < 10000000; i++){
        if(r32(opbase + USB_STS) & (1u << 15)) break;
    }

    delay(1000000);

    for(uint32_t i = 0; i < 50000000; i++){
        barrier();
        uint32_t tok0 = td[0].token;
        uint32_t tok1 = td[1].token;
        uint32_t tok2 = td[2].token;

        if((tok0 | tok1 | tok2) & 0x7e){
            w32(opbase + USB_CMD, r32(opbase + USB_CMD) & ~(1u << 5));

            for(uint32_t j = 0; j < 10000000; j++){
                if(!(r32(opbase + USB_STS) & (1u << 15))) break;
            }

            return -1;
        }

        if(!((tok0 | tok1 | tok2) & 0x80)){
            w32(opbase + USB_CMD, r32(opbase + USB_CMD) & ~(1u << 5));

            for(uint32_t j = 0; j < 10000000; j++){
                if(!(r32(opbase + USB_STS) & (1u << 15))) break;
            }

            return 0;
        }
    }

    w32(opbase + USB_CMD, r32(opbase + USB_CMD) & ~(1u << 5));
    return -1;
}

static int interrupt_in(volatile void *data, uint32_t len){
    memzero(&qh, sizeof(qh));
    memzero(td, sizeof(td));

    fill_td(&td[0], 1, PID_IN, data, len, kb_toggle);
    td[1].next = 1;
    td[1].altnext = 1;
    td[2].next = 1;
    td[2].altnext = 1;

    qh.horiz = 1;
    qh.epchar =
        (uint32_t)kb_addr |
        ((uint32_t)kb_ep << 8) |
        ((uint32_t)kb_speed << 12) |
        (1u << 14) |
        ((uint32_t)kb_ep_mps << 16) |
        (15u << 28);

    qh.epcap =
        0x01u |
        (0x1cu << 8) |
        ((uint32_t)kb_hub << 16) |
        ((uint32_t)kb_hubport << 23) |
        (1u << 30);

    qh.overlay.next = (uint32_t)&td[0];
    qh.overlay.altnext = 1;

    for(unsigned i = 0; i < 1024; i++)
        periodic[i] = ((uint32_t)&qh) | 2;

    barrier();

    w32(opbase + PERIODIC_ADDR, (uint32_t)periodic);
    w32(opbase + USB_STS, 0x3f);
    w32(opbase + USB_CMD, r32(opbase + USB_CMD) | (1u << 4));

    for(uint32_t i = 0; i < 10000000; i++){
        if(r32(opbase + USB_STS) & (1u << 14)) break;
    }

    int ret = 1;

    for(uint32_t i = 0; i < 1000000; i++){
        barrier();
        uint32_t tok = td[0].token;

        if(tok & 0x7e){
            ret = -1;
            break;
        }

        if(!(tok & 0x80)){
            kb_toggle ^= 1;
            ret = 0;
            break;
        }
    }

    w32(opbase + USB_CMD, r32(opbase + USB_CMD) & ~(1u << 4));
    for(uint32_t i = 0; i < 10000000; i++){
        if(!(r32(opbase + USB_STS) & (1u << 14))) break;
    }

    return ret;
}

static int control(uint8_t addr, uint8_t speed, uint8_t mps,
                   uint8_t hubaddr, uint8_t hubport,
                   uint8_t bm, uint8_t req, uint16_t val, uint16_t idx,
                   volatile void *data, uint16_t len){
    memzero(buf, sizeof(buf));
    memzero(td, sizeof(td));

    setup[0] = bm;
    setup[1] = req;
    setup[2] = val;
    setup[3] = val >> 8;
    setup[4] = idx;
    setup[5] = idx >> 8;
    setup[6] = len;
    setup[7] = len >> 8;

    if(len){
        fill_td(&td[0], (uint32_t)&td[1], PID_SETUP, setup, 8, 0);
        fill_td(&td[1], (uint32_t)&td[2], (bm & 0x80) ? PID_IN : PID_OUT, data, len, 1);
        fill_td(&td[2], 1, (bm & 0x80) ? PID_OUT : PID_IN, buf, 0, 1);
    }else{
        fill_td(&td[0], (uint32_t)&td[1], PID_SETUP, setup, 8, 0);
        fill_td(&td[1], 1, PID_IN, buf, 0, 1);
        td[2].next = 1;
        td[2].altnext = 1;
        td[2].token = 0;
    }

    return submit_qh(addr, 0, speed, mps, hubaddr, hubport, &td[0], speed != 2);
}

static int get_desc(uint8_t addr, uint8_t speed, uint8_t mps,
                    uint8_t hub, uint8_t port,
                    uint16_t typeidx, volatile void *dst, uint16_t len){
    return control(addr, speed, mps, hub, port, 0x80, 6, typeidx, 0, dst, len);
}

static int set_addr(uint8_t oldaddr, uint8_t speed, uint8_t mps,
                    uint8_t hub, uint8_t port, uint8_t newaddr){
    int r = control(oldaddr, speed, mps, hub, port, 0x00, 5, newaddr, 0, buf, 0);
    delay(50000000);
    return r;
}

static int set_config(uint8_t addr, uint8_t speed, uint8_t mps,
                      uint8_t hub, uint8_t port, uint8_t cfg){
    return control(addr, speed, mps, hub, port, 0x00, 9, cfg, 0, buf, 0);
}

static int hub_get_port(uint8_t hubaddr, uint8_t port, uint32_t *st){
    int r = control(hubaddr, 2, 64, 0, 0, 0xA3, 0, 0, port, buf, 4);
    *st = buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
    return r;
}

static int hub_set_feature(uint8_t hubaddr, uint8_t port, uint16_t feat){
    return control(hubaddr, 2, 64, 0, 0, 0x23, 3, feat, port, buf, 0);
}

static int hub_clr_feature(uint8_t hubaddr, uint8_t port, uint16_t feat){
    return control(hubaddr, 2, 64, 0, 0, 0x23, 1, feat, port, buf, 0);
}

static void hub_port_log(const char *s, int port, uint32_t st){
    puts(s);
    putc('0' + port);
    puts("=");
    hex32(st);
    puts("\n");
}

static int enum_hub(void){
    puts("hub\n");
    if(get_desc(0, 2, 64, 0, 0, 0x0100, buf, 18)) return fail("hub dev fail\n");

    if(buf[0] != 18 || buf[1] != 1) return fail("bad hub dev desc\n");

    dumpv("hub VIDPID", buf[8] | (buf[9] << 8) | (buf[10] << 16) | (buf[11] << 24));

    if(set_addr(0, 2, 64, 0, 0, 1)) return fail("hub addr fail\n");

    if(get_desc(1, 2, 64, 0, 0, 0x0200, buf, 64)) return fail("hub cfg fail\n");

    if(buf[0] != 9 || buf[1] != 2) return fail("bad hub cfg desc\n");

    dumpv("hub cfg", buf[5]);

    if(set_config(1, 2, 64, 0, 0, buf[5])) return fail("hub set cfg fail\n");

    delay(100000000);
    return 0;
}

static uint8_t tt_hub(uint8_t speed){
    return speed == 2 ? 0 : 1;
}

static uint8_t tt_port(uint8_t speed, int port){
    return speed == 2 ? 0 : (uint8_t)port;
}

static uint8_t hub_port_speed(uint32_t st){
    if(st & (1u << 10)) return 2;
    if(st & (1u << 9)) return 1;
    return 0;
}

static int enum_keyboard(int port, uint8_t speed, uint8_t addr){
    uint8_t hub = tt_hub(speed);
    uint8_t hubport = tt_port(speed, port);
    uint8_t ep0_mps = speed == 2 ? 64 : 8;

    puts("device\n");
    if(get_desc(0, speed, ep0_mps, hub, hubport, 0x0100, buf, 8)) return fail("dev8 fail\n");

    if(buf[0] < 8 || buf[1] != 1) return fail("bad dev8\n");

    kb_mps = buf[7];
    if(kb_mps == 0 || kb_mps > 64) kb_mps = 8;

    dumpv("kbd mps", kb_mps);

    if(set_addr(0, speed, kb_mps, hub, hubport, addr)) return fail("addr fail\n");

    if(get_desc(addr, speed, kb_mps, hub, hubport, 0x0100, buf, 18)) return fail("dev18 fail\n");

    if(buf[0] != 18 || buf[1] != 1) return fail("bad dev18\n");

    dumpv("kbd VIDPID", buf[8] | (buf[9] << 8) | (buf[10] << 16) | (buf[11] << 24));

    if(get_desc(addr, speed, kb_mps, hub, hubport, 0x0200, buf, 255)) return fail("cfg fail\n");

    if(buf[0] != 9 || buf[1] != 2) return fail("bad cfg\n");

    uint8_t cfg = buf[5];
    uint16_t total = buf[2] | (buf[3] << 8);
    if(total > sizeof(buf)) total = sizeof(buf);

    kb_iface = 0xff;
    kb_ep = 0;
    kb_ep_mps = 8;
    uint8_t in_kb_iface = 0;

    for(uint16_t i = 0; i + 2 <= total;){
        uint8_t l = buf[i];
        uint8_t t = buf[i + 1];

        if(!l) break;
        if(i + l > total) break;

        if(t == 4 && l >= 9){
            if(buf[i + 5] == 3 && buf[i + 6] == 1 && buf[i + 7] == 1){
                kb_iface = buf[i + 2];
                in_kb_iface = 1;
            }else{
                in_kb_iface = 0;
            }
        }else if(t == 5 && in_kb_iface && l >= 7){
            uint8_t ea = buf[i + 2];
            uint8_t attr = buf[i + 3] & 3;
            uint16_t emps = buf[i + 4] | (buf[i + 5] << 8);

            if((ea & 0x80) && attr == 3){
                kb_ep = ea & 0xf;
                kb_ep_mps = emps > 8 ? 8 : (uint8_t)emps;
                if(kb_ep_mps == 0) kb_ep_mps = 8;
                break;
            }
        }

        i += l;
    }

    if(kb_iface == 0xff || kb_ep == 0) return fail("no boot keyboard iface\n");

    dumpv("kbd iface", kb_iface);
    dumpv("kbd ep", kb_ep);

    if(set_config(addr, speed, kb_mps, hub, hubport, cfg)) return fail("set cfg fail\n");

    if(control(addr, speed, kb_mps, hub, hubport, 0x21, 11, 0, kb_iface, buf, 0))
        puts("kbd boot protocol fail\n");

    if(control(addr, speed, kb_mps, hub, hubport, 0x21, 10, 0, kb_iface, buf, 0))
        puts("kbd idle fail\n");

    led_report[0] = 1;
    if(control(addr, speed, kb_mps, hub, hubport, 0x21, 9, 0x0200,
               kb_iface, led_report, 1))
        puts("kbd numlock fail\n");

    kb_addr = addr;
    kb_speed = speed;
    kb_hub = hub;
    kb_hubport = hubport;
    kb_port = port;
    kb_toggle = 0;
    return 0;
}

static int find_keyboard_port(void){
    for(int p = 1; p <= 4; p++){
        hub_set_feature(1, p, 8);
        delay(500000000);

        uint32_t st = 0;
        if(hub_get_port(1, p, &st)){
            puts("hub get port fail\n");
            continue;
        }

        hub_port_log("port", p, st);

        if(!(st & 1)) continue;

        hub_set_feature(1, p, 4);
        delay(500000000);

        for(int i = 0; i < 40; i++){
            hub_get_port(1, p, &st);
            if(!(st & (1 << 4))) break;
            delay(50000000);
        }

        hub_clr_feature(1, p, 16);
        hub_clr_feature(1, p, 17);
        hub_clr_feature(1, p, 20);

        hub_get_port(1, p, &st);
        hub_port_log("reset", p, st);

        if(!(st & 2)) continue;

        uint8_t speed = hub_port_speed(st);
        puts("speed=");
        putc('0' + speed);
        puts("\n");

        if(!enum_keyboard(p, speed, 2 + p)) return p;

        puts("skip port ");
        putc('0' + p);
        puts("\n");
    }

    return -1;
}

static const char keymap[128] = {
    [4]='a',[5]='b',[6]='c',[7]='d',[8]='e',[9]='f',[10]='g',
    [11]='h',[12]='i',[13]='j',[14]='k',[15]='l',[16]='m',
    [17]='n',[18]='o',[19]='p',[20]='q',[21]='r',[22]='s',
    [23]='t',[24]='u',[25]='v',[26]='w',[27]='x',[28]='y',[29]='z',
    [30]='1',[31]='2',[32]='3',[33]='4',[34]='5',[35]='6',
    [36]='7',[37]='8',[38]='9',[39]='0',
    [40]='\n',[42]=8,[43]='\t',[44]=' ',[45]='-',[46]='=',
    [47]='[',[48]=']',[49]='\\',[51]=';',[52]='\'',[53]='`',
    [54]=',',[55]='.',[56]='/'
};

static const char keymap_shift[128] = {
    [4]='A',[5]='B',[6]='C',[7]='D',[8]='E',[9]='F',[10]='G',
    [11]='H',[12]='I',[13]='J',[14]='K',[15]='L',[16]='M',
    [17]='N',[18]='O',[19]='P',[20]='Q',[21]='R',[22]='S',
    [23]='T',[24]='U',[25]='V',[26]='W',[27]='X',[28]='Y',[29]='Z',
    [30]='!',[31]='@',[32]='#',[33]='$',[34]='%',[35]='^',
    [36]='&',[37]='*',[38]='(',[39]=')',
    [40]='\n',[42]=8,[43]='\t',[44]=' ',[45]='_',[46]='+',
    [47]='{',[48]='}',[49]='|',[51]=':',[52]='"',[53]='~',
    [54]='<',[55]='>',[56]='?'
};

static uint8_t old_keys[6];

static int old_has(uint8_t k){
    for(int i = 0; i < 6; i++)
        if(old_keys[i] == k) return 1;
    return 0;
}

static void poll_keyboard(void){
    memzero(buf, 8);

    if(interrupt_in(buf, 8))
        return;

    uint8_t mod = buf[0];
    int shift = (mod & 0x22) != 0;

    for(int i = 2; i < 8; i++){
        uint8_t k = buf[i];

        if(k && !old_has(k)){
            char c = shift ? keymap_shift[k] : keymap[k];
            if(c) putc(c);
        }
    }

    for(int i = 0; i < 6; i++)
        old_keys[i] = buf[i + 2];
}

void main(void){
    puts("USB keyboard\n");

    board_usb_init();

    if(ehci_init()){
        puts("FAIL ehci\n");
        for(;;){}
    }

    if(root_port_reset()){
        puts("FAIL root port\n");
        for(unsigned p = 0; p < ehci_nports; p++) dump_port("PORT ", p);
        for(;;){}
    }

    dump_port("ROOTPORT", root_port);
    delay(200000000);

    if(enum_hub()){
        puts("FAIL hub enum\n");
        dump_port("PORT ", root_port);
        for(;;){}
    }

    if(find_keyboard_port() < 0){
        puts("FAIL no keyboard port\n");
        for(;;){}
    }

    puts("kbd port ");
    putc('0' + kb_port);
    puts("\n");

    puts("READY\n");

    while(1){
        poll_keyboard();
        delay(3000000);
    }
}
