#include <stdint.h>
#include <stddef.h>

#include "cpu.h"
#include "mux.h"

#define BIT0	(1<<0)
#define BIT1	(1<<1)
#define BIT2	(1<<2)
#define BIT3	(1<<3)
#define BIT4	(1<<4)
#define BIT5	(1<<5)
#define BIT6	(1<<6)
#define BIT7	(1<<7)
#define BIT8	(1<<8)
#define BIT9	(1<<9)
#define BIT10	(1<<10)
#define BIT11	(1<<11)
#define BIT12	(1<<12)
#define BIT13	(1<<13)
#define BIT14	(1<<14)
#define BIT15	(1<<15)
#define BIT16	(1<<16)
#define BIT17	(1<<17)
#define BIT18	(1<<18)
#define BIT19	(1<<19)
#define BIT20	(1<<20)
#define BIT21	(1<<21)
#define BIT22	(1<<22)
#define BIT23	(1<<23)
#define BIT24	(1<<24)
#define BIT25	(1<<25)
#define BIT26	(1<<26)
#define BIT27	(1<<27)
#define BIT28	(1<<28)
#define BIT29	(1<<29)
#define BIT30	(1<<30)
#define BIT31	(1<<31)

#define LDELAY	12000000

#define __arch_getb(a)		(*(volatile uint8_t *)(a))
#define __arch_getw(a)		(*(volatile uint16_t *)(a))
#define __arch_getl(a)		(*(volatile uint32_t *)(a))

#define __arch_putb(v,a)	(*(volatile uint8_t *)(a) = (v))
#define __arch_putw(v,a)	(*(volatile uint16_t *)(a) = (v))
#define __arch_putl(v,a)	(*(volatile uint32_t *)(a) = (v))

#define __raw_writeb(v,a)	__arch_putb(v,a)
#define __raw_writew(v,a)	__arch_putw(v,a)
#define __raw_writel(v,a)	__arch_putl(v,a)

#define __raw_readb(a)		__arch_getb(a)
#define __raw_readw(a)		__arch_getw(a)
#define __raw_readl(a)		__arch_getl(a)

struct omap4panda_mux {
	uint32_t ads;
	uint32_t value;
};

#define		CP(x)	(CONTROL_PADCONF_##x)
#define		WK(x)	(CONTROL_WKUP_##x)
/*
 * IEN  - Input Enable
 * IDIS - Input Disable
 * PTD  - Pull type Down
 * PTU  - Pull type Up
 * DIS  - Pull type selection is inactive
 * EN   - Pull type selection is active
 * M0   - Mode 0
 * The commented string gives the final mux configuration for that pin
 */

static const struct omap4panda_mux omap4panda_mux[] = {
	{ OMAP44XX_CTRL_BASE + CP(GPMC_AD0),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1  /* sdmmc2_dat0 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_AD1),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1  /* sdmmc2_dat1 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_AD2),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1  /* sdmmc2_dat2 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_AD3),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1  /* sdmmc2_dat3 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_AD4),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1  /* sdmmc2_dat4 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_AD5),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1  /* sdmmc2_dat5 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_AD6),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1  /* sdmmc2_dat6 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_AD7),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1  /* sdmmc2_dat7 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_AD8),
		     PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M3  /* gpio_32 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_AD9),
						PTU | IEN | M3  /* gpio_33 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_AD10),
						PTU | IEN | M3  /* gpio_34 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_AD11),
						PTU | IEN | M3  /* gpio_35 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_AD12),
						PTU | IEN | M3  /* gpio_36 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_AD13),
		      PTD | OFF_EN | OFF_PD | OFF_OUT_PTD | M3  /* gpio_37 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_AD14),
		      PTD | OFF_EN | OFF_PD | OFF_OUT_PTD | M3  /* gpio_38 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_AD15),
		      PTD | OFF_EN | OFF_PD | OFF_OUT_PTD | M3  /* gpio_39 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_A16), M3  /* gpio_40 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_A17), PTD | M3  /* gpio_41 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_A18),
		    PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1  /* kpd_row6 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_A19),
		    PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1  /* kpd_row7 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_A20),
						      IEN | M3  /* gpio_44 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_A21), M3  /* gpio_45 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_A22), M3  /* gpio_46 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_A23),
				OFF_EN | OFF_PD | OFF_IN | M1  /* kpd_col7 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_A24), PTD | M3  /* gpio_48 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_A25), PTD | M3  /* gpio_49 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_NCS0), M3  /* gpio_50 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_NCS1), IEN | M3  /* gpio_51 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_NCS2), IEN | M3  /* gpio_52 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_NCS3), IEN | M3  /* gpio_53 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_NWP), M3  /* gpio_54 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_CLK), PTD | M3  /* gpio_55 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_NADV_ALE), M3  /* gpio_56 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_NOE),
		      PTU | IEN | OFF_EN | OFF_OUT_PTD | M1  /* sdmmc2_clk */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_NWE),
		  PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1  /* sdmmc2_cmd */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_NBE0_CLE), M3  /* gpio_59 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_NBE1), PTD | M3  /* gpio_60 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_WAIT0), PTU | IEN | M3  /* gpio_61 */ },
	{ OMAP44XX_CTRL_BASE + CP(GPMC_WAIT1),
		       PTD | OFF_EN | OFF_PD | OFF_OUT_PTD | M3 /* gpio_62 */ },
	{ OMAP44XX_CTRL_BASE + CP(C2C_DATA11), PTD | M3  /* gpio_100 */ },
	{ OMAP44XX_CTRL_BASE + CP(C2C_DATA12), PTD | IEN | M3  /* gpio_101 */ },
	{ OMAP44XX_CTRL_BASE + CP(C2C_DATA13), PTD | M3  /* gpio_102 */ },
	{ OMAP44XX_CTRL_BASE + CP(C2C_DATA14), M1  /* dsi2_te0 */ },
	{ OMAP44XX_CTRL_BASE + CP(C2C_DATA15), PTD | M3  /* gpio_104 */ },
	{ OMAP44XX_CTRL_BASE + CP(HDMI_HPD), M0  /* hdmi_hpd */ },
	{ OMAP44XX_CTRL_BASE + CP(HDMI_CEC), M0  /* hdmi_cec */ },
	{ OMAP44XX_CTRL_BASE + CP(HDMI_DDC_SCL), PTU | M0  /* hdmi_ddc_scl */ },
	{ OMAP44XX_CTRL_BASE + CP(HDMI_DDC_SDA),
					   PTU | IEN | M0  /* hdmi_ddc_sda */ },
	{ OMAP44XX_CTRL_BASE + CP(CSI21_DX0), IEN | M0  /* csi21_dx0 */ },
	{ OMAP44XX_CTRL_BASE + CP(CSI21_DY0), IEN | M0  /* csi21_dy0 */ },
	{ OMAP44XX_CTRL_BASE + CP(CSI21_DX1), IEN | M0  /* csi21_dx1 */ },
	{ OMAP44XX_CTRL_BASE + CP(CSI21_DY1), IEN | M0  /* csi21_dy1 */ },
	{ OMAP44XX_CTRL_BASE + CP(CSI21_DX2), IEN | M0  /* csi21_dx2 */ },
	{ OMAP44XX_CTRL_BASE + CP(CSI21_DY2), IEN | M0  /* csi21_dy2 */ },
	{ OMAP44XX_CTRL_BASE + CP(CSI21_DX3), PTD | M7  /* csi21_dx3 */ },
	{ OMAP44XX_CTRL_BASE + CP(CSI21_DY3), PTD | M7  /* csi21_dy3 */ },
	{ OMAP44XX_CTRL_BASE + CP(CSI21_DX4),
			 PTD | OFF_EN | OFF_PD | OFF_IN | M7  /* csi21_dx4 */ },
	{ OMAP44XX_CTRL_BASE + CP(CSI21_DY4),
			 PTD | OFF_EN | OFF_PD | OFF_IN | M7  /* csi21_dy4 */ },
	{ OMAP44XX_CTRL_BASE + CP(CSI22_DX0), IEN | M0  /* csi22_dx0 */ },
	{ OMAP44XX_CTRL_BASE + CP(CSI22_DY0), IEN | M0  /* csi22_dy0 */ },
	{ OMAP44XX_CTRL_BASE + CP(CSI22_DX1), IEN | M0  /* csi22_dx1 */ },
	{ OMAP44XX_CTRL_BASE + CP(CSI22_DY1), IEN | M0  /* csi22_dy1 */ },
	{ OMAP44XX_CTRL_BASE + CP(CAM_SHUTTER),
			OFF_EN | OFF_PD | OFF_OUT_PTD | M0  /* cam_shutter */ },
	{ OMAP44XX_CTRL_BASE + CP(CAM_STROBE),
			 OFF_EN | OFF_PD | OFF_OUT_PTD | M0  /* cam_strobe */ },
	{ OMAP44XX_CTRL_BASE + CP(CAM_GLOBALRESET),
		      PTD | OFF_EN | OFF_PD | OFF_OUT_PTD | M3  /* gpio_83 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB1_ULPITLL_CLK),
	   PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M4  /* usbb1_ulpiphy_clk */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB1_ULPITLL_STP),
			   OFF_EN | OFF_OUT_PTD | M4  /* usbb1_ulpiphy_stp */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB1_ULPITLL_DIR),
		 IEN | OFF_EN | OFF_PD | OFF_IN | M4  /* usbb1_ulpiphy_dir */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB1_ULPITLL_NXT),
		 IEN | OFF_EN | OFF_PD | OFF_IN | M4  /* usbb1_ulpiphy_nxt */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB1_ULPITLL_DAT0),
		IEN | OFF_EN | OFF_PD | OFF_IN | M4  /* usbb1_ulpiphy_dat0 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB1_ULPITLL_DAT1),
		IEN | OFF_EN | OFF_PD | OFF_IN | M4  /* usbb1_ulpiphy_dat1 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB1_ULPITLL_DAT2),
		IEN | OFF_EN | OFF_PD | OFF_IN | M4  /* usbb1_ulpiphy_dat2 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB1_ULPITLL_DAT3),
		IEN | OFF_EN | OFF_PD | OFF_IN | M4  /* usbb1_ulpiphy_dat3 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB1_ULPITLL_DAT4),
		IEN | OFF_EN | OFF_PD | OFF_IN | M4  /* usbb1_ulpiphy_dat4 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB1_ULPITLL_DAT5),
		IEN | OFF_EN | OFF_PD | OFF_IN | M4  /* usbb1_ulpiphy_dat5 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB1_ULPITLL_DAT6),
		IEN | OFF_EN | OFF_PD | OFF_IN | M4  /* usbb1_ulpiphy_dat6 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB1_ULPITLL_DAT7),
		IEN | OFF_EN | OFF_PD | OFF_IN | M4  /* usbb1_ulpiphy_dat7 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB1_HSIC_DATA),
		   IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* usbb1_hsic_data */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB1_HSIC_STROBE),
		 IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* usbb1_hsic_strobe */ },
	{ OMAP44XX_CTRL_BASE + CP(USBC1_ICUSB_DP),
					       IEN | M0  /* usbc1_icusb_dp */ },
	{ OMAP44XX_CTRL_BASE + CP(USBC1_ICUSB_DM),
					       IEN | M0  /* usbc1_icusb_dm */ },
	{ OMAP44XX_CTRL_BASE + CP(SDMMC1_CLK),
			    PTU | OFF_EN | OFF_OUT_PTD | M0  /* sdmmc1_clk */ },
	{ OMAP44XX_CTRL_BASE + CP(SDMMC1_CMD),
		  PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* sdmmc1_cmd */ },
	{ OMAP44XX_CTRL_BASE + CP(SDMMC1_DAT0),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* sdmmc1_dat0 */ },
	{ OMAP44XX_CTRL_BASE + CP(SDMMC1_DAT1),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* sdmmc1_dat1 */ },
	{ OMAP44XX_CTRL_BASE + CP(SDMMC1_DAT2),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* sdmmc1_dat2 */ },
	{ OMAP44XX_CTRL_BASE + CP(SDMMC1_DAT3),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* sdmmc1_dat3 */ },
	{ OMAP44XX_CTRL_BASE + CP(SDMMC1_DAT4),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* sdmmc1_dat4 */ },
	{ OMAP44XX_CTRL_BASE + CP(SDMMC1_DAT5),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* sdmmc1_dat5 */ },
	{ OMAP44XX_CTRL_BASE + CP(SDMMC1_DAT6),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* sdmmc1_dat6 */ },
	{ OMAP44XX_CTRL_BASE + CP(SDMMC1_DAT7),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* sdmmc1_dat7 */ },
	{ OMAP44XX_CTRL_BASE + CP(ABE_MCBSP2_CLKX),
		   IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* abe_mcbsp2_clkx */ },
	{ OMAP44XX_CTRL_BASE + CP(ABE_MCBSP2_DR),
			 IEN | OFF_EN | OFF_OUT_PTD | M0  /* abe_mcbsp2_dr */ },
	{ OMAP44XX_CTRL_BASE + CP(ABE_MCBSP2_DX),
			       OFF_EN | OFF_OUT_PTD | M0  /* abe_mcbsp2_dx */ },
	{ OMAP44XX_CTRL_BASE + CP(ABE_MCBSP2_FSX),
		    IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* abe_mcbsp2_fsx */ },
	{ OMAP44XX_CTRL_BASE + CP(ABE_MCBSP1_CLKX),
					   IEN | M1  /* abe_slimbus1_clock */ },
	{ OMAP44XX_CTRL_BASE + CP(ABE_MCBSP1_DR),
					    IEN | M1  /* abe_slimbus1_data */ },
	{ OMAP44XX_CTRL_BASE + CP(ABE_MCBSP1_DX),
			       OFF_EN | OFF_OUT_PTD | M0  /* abe_mcbsp1_dx */ },
	{ OMAP44XX_CTRL_BASE + CP(ABE_MCBSP1_FSX),
		    IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* abe_mcbsp1_fsx */ },
	{ OMAP44XX_CTRL_BASE + CP(ABE_PDM_UL_DATA),
	     PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* abe_pdm_ul_data */ },
	{ OMAP44XX_CTRL_BASE + CP(ABE_PDM_DL_DATA),
	     PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* abe_pdm_dl_data */ },
	{ OMAP44XX_CTRL_BASE + CP(ABE_PDM_FRAME),
	       PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* abe_pdm_frame */ },
	{ OMAP44XX_CTRL_BASE + CP(ABE_PDM_LB_CLK),
	      PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* abe_pdm_lb_clk */ },
	{ OMAP44XX_CTRL_BASE + CP(ABE_CLKS),
		    PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* abe_clks */ },
	{ OMAP44XX_CTRL_BASE + CP(ABE_DMIC_CLK1), M0  /* abe_dmic_clk1 */ },
	{ OMAP44XX_CTRL_BASE + CP(ABE_DMIC_DIN1),
						IEN | M0  /* abe_dmic_din1 */ },
	{ OMAP44XX_CTRL_BASE + CP(ABE_DMIC_DIN2),
						IEN | M0  /* abe_dmic_din2 */ },
	{ OMAP44XX_CTRL_BASE + CP(ABE_DMIC_DIN3),
						IEN | M0  /* abe_dmic_din3 */ },
	{ OMAP44XX_CTRL_BASE + CP(UART2_CTS), PTU | IEN | M0  /* uart2_cts */ },
	{ OMAP44XX_CTRL_BASE + CP(UART2_RTS), M0  /* uart2_rts */ },
	{ OMAP44XX_CTRL_BASE + CP(UART2_RX), PTU | IEN | M0  /* uart2_rx */ },
	{ OMAP44XX_CTRL_BASE + CP(UART2_TX), M0  /* uart2_tx */ },
	{ OMAP44XX_CTRL_BASE + CP(HDQ_SIO), M3  /* gpio_127 */ },
	{ OMAP44XX_CTRL_BASE + CP(I2C1_SCL), PTU | IEN | M0  /* i2c1_scl */ },
	{ OMAP44XX_CTRL_BASE + CP(I2C1_SDA), PTU | IEN | M0  /* i2c1_sda */ },
	{ OMAP44XX_CTRL_BASE + CP(I2C2_SCL), PTU | IEN | M0  /* i2c2_scl */ },
	{ OMAP44XX_CTRL_BASE + CP(I2C2_SDA), PTU | IEN | M0  /* i2c2_sda */ },
	{ OMAP44XX_CTRL_BASE + CP(I2C3_SCL), PTU | IEN | M0  /* i2c3_scl */ },
	{ OMAP44XX_CTRL_BASE + CP(I2C3_SDA), PTU | IEN | M0  /* i2c3_sda */ },
	{ OMAP44XX_CTRL_BASE + CP(I2C4_SCL), PTU | IEN | M0  /* i2c4_scl */ },
	{ OMAP44XX_CTRL_BASE + CP(I2C4_SDA), PTU | IEN | M0  /* i2c4_sda */ },
	{ OMAP44XX_CTRL_BASE + CP(MCSPI1_CLK),
			IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* mcspi1_clk */ },
	{ OMAP44XX_CTRL_BASE + CP(MCSPI1_SOMI),
		       IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* mcspi1_somi */ },
	{ OMAP44XX_CTRL_BASE + CP(MCSPI1_SIMO),
		       IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* mcspi1_simo */ },
	{ OMAP44XX_CTRL_BASE + CP(MCSPI1_CS0),
		  PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* mcspi1_cs0 */ },
	{ OMAP44XX_CTRL_BASE + CP(MCSPI1_CS1),
		  PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M3  /* mcspi1_cs1 */ },
	{ OMAP44XX_CTRL_BASE + CP(MCSPI1_CS2),
			      PTU | OFF_EN | OFF_OUT_PTU | M3  /* gpio_139 */ },
	{ OMAP44XX_CTRL_BASE + CP(MCSPI1_CS3), PTU | IEN | M3  /* gpio_140 */ },
	{ OMAP44XX_CTRL_BASE + CP(UART3_CTS_RCTX),
					       PTU | IEN | M0  /* uart3_tx */ },
	{ OMAP44XX_CTRL_BASE + CP(UART3_RTS_SD), M0  /* uart3_rts_sd */ },
	{ OMAP44XX_CTRL_BASE + CP(UART3_RX_IRRX), IEN | M0  /* uart3_rx */ },
	{ OMAP44XX_CTRL_BASE + CP(UART3_TX_IRTX), M0  /* uart3_tx */ },
	{ OMAP44XX_CTRL_BASE + CP(SDMMC5_CLK),
		      PTU | IEN | OFF_EN | OFF_OUT_PTD | M0  /* sdmmc5_clk */ },
	{ OMAP44XX_CTRL_BASE + CP(SDMMC5_CMD),
		  PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* sdmmc5_cmd */ },
	{ OMAP44XX_CTRL_BASE + CP(SDMMC5_DAT0),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* sdmmc5_dat0 */ },
	{ OMAP44XX_CTRL_BASE + CP(SDMMC5_DAT1),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* sdmmc5_dat1 */ },
	{ OMAP44XX_CTRL_BASE + CP(SDMMC5_DAT2),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* sdmmc5_dat2 */ },
	{ OMAP44XX_CTRL_BASE + CP(SDMMC5_DAT3),
		 PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* sdmmc5_dat3 */ },
	{ OMAP44XX_CTRL_BASE + CP(MCSPI4_CLK),
			IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* mcspi4_clk */ },
	{ OMAP44XX_CTRL_BASE + CP(MCSPI4_SIMO),
		       IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* mcspi4_simo */ },
	{ OMAP44XX_CTRL_BASE + CP(MCSPI4_SOMI),
		       IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* mcspi4_somi */ },
	{ OMAP44XX_CTRL_BASE + CP(MCSPI4_CS0),
		  PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* mcspi4_cs0 */ },
	{ OMAP44XX_CTRL_BASE + CP(UART4_RX), IEN | M0  /* uart4_rx */ },
	{ OMAP44XX_CTRL_BASE + CP(UART4_TX), M0  /* uart4_tx */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB2_ULPITLL_CLK),
						     IEN | M3  /* gpio_157 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB2_ULPITLL_STP),
						IEN | M5  /* dispc2_data23 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB2_ULPITLL_DIR),
						IEN | M5  /* dispc2_data22 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB2_ULPITLL_NXT),
						IEN | M5  /* dispc2_data21 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB2_ULPITLL_DAT0),
						IEN | M5  /* dispc2_data20 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB2_ULPITLL_DAT1),
						IEN | M5  /* dispc2_data19 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB2_ULPITLL_DAT2),
						IEN | M5  /* dispc2_data18 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB2_ULPITLL_DAT3),
						IEN | M5  /* dispc2_data15 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB2_ULPITLL_DAT4),
						IEN | M5  /* dispc2_data14 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB2_ULPITLL_DAT5),
						IEN | M5  /* dispc2_data13 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB2_ULPITLL_DAT6),
						IEN | M5  /* dispc2_data12 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB2_ULPITLL_DAT7),
						IEN | M5  /* dispc2_data11 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB2_HSIC_DATA),
			      PTD | OFF_EN | OFF_OUT_PTU | M3  /* gpio_169 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBB2_HSIC_STROBE),
			      PTD | OFF_EN | OFF_OUT_PTU | M3  /* gpio_170 */ },
	{ OMAP44XX_CTRL_BASE + CP(UNIPRO_TX0), PTD | IEN | M3  /* gpio_171 */ },
	{ OMAP44XX_CTRL_BASE + CP(UNIPRO_TY0),
				OFF_EN | OFF_PD | OFF_IN | M1  /* kpd_col1 */ },
	{ OMAP44XX_CTRL_BASE + CP(UNIPRO_TX1),
				OFF_EN | OFF_PD | OFF_IN | M1  /* kpd_col2 */ },
	{ OMAP44XX_CTRL_BASE + CP(UNIPRO_TY1),
				OFF_EN | OFF_PD | OFF_IN | M1  /* kpd_col3 */ },
	{ OMAP44XX_CTRL_BASE + CP(UNIPRO_TX2), PTU | IEN | M3  /* gpio_0 */ },
	{ OMAP44XX_CTRL_BASE + CP(UNIPRO_TY2), PTU | IEN | M3  /* gpio_1 */ },
	{ OMAP44XX_CTRL_BASE + CP(UNIPRO_RX0),
		    PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1  /* kpd_row0 */ },
	{ OMAP44XX_CTRL_BASE + CP(UNIPRO_RY0),
		    PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1  /* kpd_row1 */ },
	{ OMAP44XX_CTRL_BASE + CP(UNIPRO_RX1),
		    PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1  /* kpd_row2 */ },
	{ OMAP44XX_CTRL_BASE + CP(UNIPRO_RY1),
		    PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1  /* kpd_row3 */ },
	{ OMAP44XX_CTRL_BASE + CP(UNIPRO_RX2),
		    PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1  /* kpd_row4 */ },
	{ OMAP44XX_CTRL_BASE + CP(UNIPRO_RY2),
		    PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1  /* kpd_row5 */ },
	{ OMAP44XX_CTRL_BASE + CP(USBA0_OTG_CE),
		 PTD | OFF_EN | OFF_PD | OFF_OUT_PTD | M0  /* usba0_otg_ce */ },
	{ OMAP44XX_CTRL_BASE + CP(USBA0_OTG_DP),
		      IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* usba0_otg_dp */ },
	{ OMAP44XX_CTRL_BASE + CP(USBA0_OTG_DM),
		      IEN | OFF_EN | OFF_PD | OFF_IN | M0  /* usba0_otg_dm */ },
	{ OMAP44XX_CTRL_BASE + CP(FREF_CLK1_OUT), M0  /* fref_clk1_out */ },
	{ OMAP44XX_CTRL_BASE + CP(FREF_CLK2_OUT),
					       PTD | IEN | M3  /* gpio_182 */ },
	{ OMAP44XX_CTRL_BASE + CP(SYS_NIRQ1), PTU | IEN | M0  /* sys_nirq1 */ },
	{ OMAP44XX_CTRL_BASE + CP(SYS_NIRQ2), M7  /* sys_nirq2 */ },
	{ OMAP44XX_CTRL_BASE + CP(SYS_BOOT0), PTU | IEN | M3  /* gpio_184 */ },
	{ OMAP44XX_CTRL_BASE + CP(SYS_BOOT1), M3  /* gpio_185 */ },
	{ OMAP44XX_CTRL_BASE + CP(SYS_BOOT2), PTD | IEN | M3  /* gpio_186 */ },
	{ OMAP44XX_CTRL_BASE + CP(SYS_BOOT3), M3  /* gpio_187 */ },
	{ OMAP44XX_CTRL_BASE + CP(SYS_BOOT4), M3  /* gpio_188 */ },
	{ OMAP44XX_CTRL_BASE + CP(SYS_BOOT5), PTD | IEN | M3  /* gpio_189 */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU0), IEN | M0  /* dpm_emu0 */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU1), IEN | M0  /* dpm_emu1 */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU2), IEN | M0  /* dpm_emu2 */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU3), IEN | M5  /* dispc2_data10 */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU4), IEN | M5  /* dispc2_data9 */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU5), IEN | M5  /* dispc2_data16 */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU6), IEN | M5  /* dispc2_data17 */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU7), IEN | M5  /* dispc2_hsync */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU8), IEN | M5  /* dispc2_pclk */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU9), IEN | M5  /* dispc2_vsync */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU10), IEN | M5  /* dispc2_de */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU11), IEN | M5  /* dispc2_data8 */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU12), IEN | M5  /* dispc2_data7 */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU13), IEN | M5  /* dispc2_data6 */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU14), IEN | M5  /* dispc2_data5 */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU15), IEN | M5  /* dispc2_data4 */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU16), M3  /* gpio_27 */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU17), IEN | M5  /* dispc2_data2 */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU18), IEN | M5  /* dispc2_data1 */ },
	{ OMAP44XX_CTRL_BASE + CP(DPM_EMU19), IEN | M5  /* dispc2_data0 */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD0_SIM_IO), IEN | M0  /* sim_io */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD1_SIM_CLK), M0  /* sim_clk */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD0_SIM_RESET), M0  /* sim_reset */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD1_SIM_CD),
						 PTU | IEN | M0  /* sim_cd */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD0_SIM_PWRCTRL),
							M0  /* sim_pwrctrl */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD1_SR_SCL),
						 PTU | IEN | M0  /* sr_scl */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD0_SR_SDA),
						 PTU | IEN | M0  /* sr_sda */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD1_FREF_XTAL_IN), M0  /* # */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD0_FREF_SLICER_IN),
						     M0  /* fref_slicer_in */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD1_FREF_CLK_IOREQ),
						     M0  /* fref_clk_ioreq */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD0_FREF_CLK0_OUT),
						    M2  /* sys_drm_msecure */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD1_FREF_CLK3_REQ),
						      PTU | IEN | M0  /* # */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD0_FREF_CLK3_OUT),
						      M0  /* fref_clk3_out */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD1_FREF_CLK4_REQ),
						      PTU | IEN | M0  /* # */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD0_FREF_CLK4_OUT), M0  /* # */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD1_SYS_32K), IEN | M0  /* sys_32k */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD0_SYS_NRESPWRON),
						      M0  /* sys_nrespwron */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD1_SYS_NRESWARM),
						       M0  /* sys_nreswarm */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD0_SYS_PWR_REQ),
						  PTU | M0  /* sys_pwr_req */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD1_SYS_PWRON_RESET),
							  M3  /* gpio_wk29 */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD0_SYS_BOOT6),
						     IEN | M3  /* gpio_wk9 */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD1_SYS_BOOT7),
						    IEN | M3  /* gpio_wk10 */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD1_FREF_CLK3_REQ),
							   M3 /* gpio_wk30 */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD1_FREF_CLK4_REQ), M3 /* gpio_wk7 */ },
	{ OMAP44XX_WKUP_CTRL_BASE + WK(PAD0_FREF_CLK4_OUT), M3 /* gpio_wk8 */ },
};



/*****************************************************************
 * sr32 - clear & set a value in a bit range for a 32 bit address
 *****************************************************************/
void sr32(uint32_t addr, uint32_t start_bit, uint32_t num_bits, uint32_t value)
{
	uint32_t tmp, msk = 0;
	msk = 1 << num_bits;
	--msk;
	tmp = __raw_readl(addr) & ~(msk << start_bit);
	tmp |=  value << start_bit;
	__raw_writel(tmp, addr);
}

/*********************************************************************
 * wait_on_value() - common routine to allow waiting for changes in
 *   volatile regs.
 *********************************************************************/
uint32_t wait_on_value(uint32_t read_bit_mask, uint32_t match_value, uint32_t read_addr, uint32_t bound)
{
	uint32_t i = 0, val;
	do {
		++i;
		val = __raw_readl(read_addr) & read_bit_mask;
		if (val == match_value)
			return 1;
		if (i == bound)
			return 0;
	} while (1);
}

uint32_t cortex_a9_rev()
{
	uint32_t i;

	asm ("mrc p15, 0, %0, c0, c0, 0" : "=r" (i));

	return i;
}

uint32_t omap_revision()
{
	switch (cortex_a9_rev()) {

	case 0x410FC091:
		return OMAP4430_ES1_0;

	case 0x411FC092:
		switch ((__raw_readl(OMAP44XX_CTRL_ID_CODE) >> 28) & 0xF) {
		case 0:
		case 1:
		case 2:
			return OMAP4430_ES2_0;
		case 3:
			return OMAP4430_ES2_1;
		default:
			return OMAP4430_ES2_2;
		}
	}

	return OMAP4430_SILICON_ID_INVALID;
}
int main()
{
// get rev
	uint32_t rev = omap_revision();

	uint32_t clk_index;

	/* Get the sysclk speed from cm_sys_clksel
	 * Set the CM_SYS_CLKSEL in case ROM code has not set
	 */
	__raw_writel(0x7, CM_SYS_CLKSEL);
	clk_index = __raw_readl(CM_SYS_CLKSEL);
	if (!clk_index) {
//		return; /* Sys clk uninitialized */
		goto set_muxconf;
	}

	/* L4PER clocks */
/*
	sr32(CM_L4PER_CLKSTCTRL, 0, 32, 0x2);
	sr32(CM_L4PER_DMTIMER10_CLKCTRL, 0, 32, 0x2);
	wait_on_value(BIT17|BIT16, 0, CM_L4PER_DMTIMER10_CLKCTRL, LDELAY);
	sr32(CM_L4PER_DMTIMER11_CLKCTRL, 0, 32, 0x2);
	wait_on_value(BIT17|BIT16, 0, CM_L4PER_DMTIMER11_CLKCTRL, LDELAY);
	sr32(CM_L4PER_DMTIMER2_CLKCTRL, 0, 32, 0x2);
	wait_on_value(BIT17|BIT16, 0, CM_L4PER_DMTIMER2_CLKCTRL, LDELAY);
	sr32(CM_L4PER_DMTIMER3_CLKCTRL, 0, 32, 0x2);
	wait_on_value(BIT17|BIT16, 0, CM_L4PER_DMTIMER3_CLKCTRL, LDELAY);
	sr32(CM_L4PER_DMTIMER4_CLKCTRL, 0, 32, 0x2);
	wait_on_value(BIT17|BIT16, 0, CM_L4PER_DMTIMER4_CLKCTRL, LDELAY);
	sr32(CM_L4PER_DMTIMER9_CLKCTRL, 0, 32, 0x2);
	wait_on_value(BIT17|BIT16, 0, CM_L4PER_DMTIMER9_CLKCTRL, LDELAY);
*/

	/* GPIO clocks */
/*
	sr32(CM_L4PER_GPIO2_CLKCTRL, 0, 32, 0x1);
	wait_on_value(BIT17|BIT16, 0, CM_L4PER_GPIO2_CLKCTRL, LDELAY);
	sr32(CM_L4PER_GPIO3_CLKCTRL, 0, 32, 0x1);
	wait_on_value(BIT17|BIT16, 0, CM_L4PER_GPIO3_CLKCTRL, LDELAY);
	sr32(CM_L4PER_GPIO4_CLKCTRL, 0, 32, 0x1);
	wait_on_value(BIT17|BIT16, 0, CM_L4PER_GPIO4_CLKCTRL, LDELAY);
	sr32(CM_L4PER_GPIO5_CLKCTRL, 0, 32, 0x1);
	wait_on_value(BIT17|BIT16, 0, CM_L4PER_GPIO5_CLKCTRL, LDELAY);
	sr32(CM_L4PER_GPIO6_CLKCTRL, 0, 32, 0x1);
	wait_on_value(BIT17|BIT16, 0, CM_L4PER_GPIO6_CLKCTRL, LDELAY);

	sr32(CM_L4PER_HDQ1W_CLKCTRL, 0, 32, 0x2);
*/
	/* WKUP clocks */
	sr32(CM_WKUP_GPIO1_CLKCTRL, 0, 32, 0x1);
	wait_on_value(BIT17|BIT16, 0, CM_WKUP_GPIO1_CLKCTRL, LDELAY);
	sr32(CM_WKUP_TIMER1_CLKCTRL, 0, 32, 0x01000002);
	wait_on_value(BIT17|BIT16, 0, CM_WKUP_TIMER1_CLKCTRL, LDELAY);

	sr32(CM_WKUP_KEYBOARD_CLKCTRL, 0, 32, 0x2);
	wait_on_value(BIT17|BIT16, 0, CM_WKUP_KEYBOARD_CLKCTRL, LDELAY);

	sr32(CM_SDMA_CLKSTCTRL, 0, 32, 0x0);
	sr32(CM_MEMIF_CLKSTCTRL, 0, 32, 0x3);
	sr32(CM_MEMIF_EMIF_1_CLKCTRL, 0, 32, 0x1);
	wait_on_value(BIT17|BIT16, 0, CM_MEMIF_EMIF_1_CLKCTRL, LDELAY);
	sr32(CM_MEMIF_EMIF_2_CLKCTRL, 0, 32, 0x1);
	wait_on_value(BIT17|BIT16, 0, CM_MEMIF_EMIF_2_CLKCTRL, LDELAY);
	sr32(CM_D2D_CLKSTCTRL, 0, 32, 0x3);
	sr32(CM_L3_2_GPMC_CLKCTRL, 0, 32, 0x1);
	wait_on_value(BIT17|BIT16, 0, CM_L3_2_GPMC_CLKCTRL, LDELAY);
	sr32(CM_L3INSTR_L3_3_CLKCTRL, 0, 32, 0x1);
	wait_on_value(BIT17|BIT16, 0, CM_L3INSTR_L3_3_CLKCTRL, LDELAY);
	sr32(CM_L3INSTR_L3_INSTR_CLKCTRL, 0, 32, 0x1);
	wait_on_value(BIT17|BIT16, 0, CM_L3INSTR_L3_INSTR_CLKCTRL, LDELAY);
	sr32(CM_L3INSTR_OCP_WP1_CLKCTRL, 0, 32, 0x1);
	wait_on_value(BIT17|BIT16, 0, CM_L3INSTR_OCP_WP1_CLKCTRL, LDELAY);

set_muxconf:
	for (size_t n = 0; n < sizeof omap4panda_mux / sizeof omap4panda_mux[0]; n++)
		__raw_writew(omap4panda_mux[n].value, omap4panda_mux[n].ads);

	asm volatile ("nop\n\tnop\n\tnop\n\tb .\n\tnop\n\tnop\n\tnop\n\t");

// light up both leds
	uint32_t v = __raw_readl(OMAP44XX_GPIO_BASE1 + __GPIO_OE);

	/* set both LED gpio to output */
	__raw_writel((v & ~(0x03 << 7)), OMAP44XX_GPIO_BASE1 + __GPIO_OE);

	v = __raw_readl(OMAP44XX_GPIO_BASE1 + __GPIO_DATAOUT);
	__raw_writel((v | (0x03 << 7)), OMAP44XX_GPIO_BASE1 + __GPIO_DATAOUT);

	(void) rev;

	return 0;
}
