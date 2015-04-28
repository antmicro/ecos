/*****************************************************************************/
/**
*
* @file     xc7z020_xslcr.h
*
* @brief    Private timer header
*
******************************************************************************/

#ifndef XC7Z020_PTIMER_H /* prevent circular inclusions */
#define XC7Z020_PTIMER_H /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif


/************************** Constant Definitions *****************************/

/* SLCR registers */
#define XSLCR_LOCK_OFFSET               0x00000004       /**< SLCR Write Protection Lock */
#define XSLCR_UNLOCK_OFFSET             0x00000008       /**< SLCR Write Protection Unlock */
#define XSLCRIO_PLL_CTRL_OFFSET         0x00000108       /**< IO PLL Configuration */
#define XSLCRAPER_CLK_CTRL_OFFSET       0x0000012C       /**< AMBA Peripheral Clock Control */
#define XSLCRGEM0_RCLK_CTRL_OFFSET      0x00000138       /**< GigE 0 Rx Clock Control */
#define XSLCRGEM1_RCLK_CTRL_OFFSET      0x0000013C       /**< GigE 1 Rx Clock Control */
#define XSLCRGEM0_CLK_CTRL_OFFSET       0x00000140       /**< GigE 0 Ref Clock Control */
#define XSLCRGEM1_CLK_CTRL_OFFSET       0x00000144       /**< GigE 1 Ref Clock Control */
#define XSLCRUART_CLK_CTRL_OFFSET       0x00000154       /**< UART Ref Clock Control */
#define XSLCRUART_RST_CTRL_OFFSET       0x00000228       /**< UART Software Reset Control */

/* SLCR registers lock key */
#define XSLCR_LOCK_KEY                  0x0000767B

/* SLCR registers unlock key */
#define XSLCR_UNLOCK_KEY                0x0000DF0D

/* AMBA Peripheral Clock Control register bits */
#define XSLCRAPER_CLK_CTRL_SMC_EN       0x01000000
#define XSLCRAPER_CLK_CTRL_QSPI_EN      0x00800000
#define XSLCRAPER_CLK_CTRL_GPIO_EN      0x00400000
#define XSLCRAPER_CLK_CTRL_UART1_EN     0x00200000
#define XSLCRAPER_CLK_CTRL_UART0_EN     0x00100000
#define XSLCRAPER_CLK_CTRL_I2C1_EN      0x00080000
#define XSLCRAPER_CLK_CTRL_I2C0_EN      0x00040000
#define XSLCRAPER_CLK_CTRL_CAN1_EN      0x00020000
#define XSLCRAPER_CLK_CTRL_CAN0_EN      0x00010000
#define XSLCRAPER_CLK_CTRL_SPI1_EN      0x00008000
#define XSLCRAPER_CLK_CTRL_SPI0_EN      0x00004000
#define XSLCRAPER_CLK_CTRL_SDI1_EN      0x00000800
#define XSLCRAPER_CLK_CTRL_SDI0_EN      0x00000400
#define XSLCRAPER_CLK_CTRL_GEM1_EN      0x00000080
#define XSLCRAPER_CLK_CTRL_GEM0_EN      0x00000040
#define XSLCRAPER_CLK_CTRL_USB1_EN      0x00000008
#define XSLCRAPER_CLK_CTRL_USB0_EN      0x00000004
#define XSLCRAPER_CLK_CTRL_DMA_EN       0x00000001

/* GigE N Rx Clock Control register bits */
#define XSLCRGEM_RCLK_CTRL_SRCSEL_MASK  0x00000010
#define XSLCRGEM_RCLK_CTRL_SRCSEL_EMIO  0x00000010
#define XSLCRGEM_RCLK_CTRL_SRCSEL_MIO   0x00000000
#define XSLCRGEM_RCLK_CTRL_CLKACT_EN    0x00000001

/* GigE N Ref Clock Control register bits */
#define XSLCRGEM_CLK_CTRL_DIVISOR1_MASK     0x03F00000
#define XSLCRGEM_CLK_CTRL_DIVISOR1_BITPOS   20
#define XSLCRGEM_CLK_CTRL_DIVISOR_MASK      0x00003F00
#define XSLCRGEM_CLK_CTRL_DIVISOR_BITPOS    8
#define XSLCRGEM_CLK_CTRL_SRCSEL_MASK       0x00000070
#define XSLCRGEM_CLK_CTRL_SRCSEL_IOPLL      0x00000010
#define XSLCRGEM_CLK_CTRL_SRCSEL_ARMPLL     0x00000020
#define XSLCRGEM_CLK_CTRL_SRCSEL_DDRPLL     0x00000030
#define XSLCRGEM_CLK_CTRL_SRCSEL_EMIO       0x00000040
#define XSLCRGEM_CLK_CTRL_CLKACT_EN         0x00000001

/* UART Ref Clock Control register bits */
#define XSLCRUART_CLK_CTRL_CLKACT1_EN   0x00000002
#define XSLCRUART_CLK_CTRL_CLKACT0_EN   0x00000001
/* UART Ref Clock Control register fields */
#define XSLCRUART_CLK_CTRL_SRCSEL_MASK          0x00000030
#define XSLCRUART_CLK_CTRL_SRCSEL_IO_PLL_EN     0x00000010
#define XSLCRUART_CLK_CTRL_SRCSEL_ARM_PLL_EN    0x00000020
#define XSLCRUART_CLK_CTRL_SRCSEL_DDR_PLL_EN    0x00000030
#define XSLCRUART_CLK_CTRL_DIVISOR_MASK         0x00003f00
#define XSLCRUART_CLK_CTRL_DIVISOR_BITPOS       8

/* UART Software Reset Control register bits */
#define XSLCRUART_RST_CTRL_UART1_REF_RST    0x00000008
#define XSLCRUART_RST_CTRL_UART0_REF_RST    0x00000004
#define XSLCRUART_RST_CTRL_UART1_CPU1X_RST  0x00000002
#define XSLCRUART_RST_CTRL_UART0_CPU1X_RST  0x00000001

/* @} */

 
#ifdef __cplusplus
}
#endif

#endif            /* end of protection macro */

