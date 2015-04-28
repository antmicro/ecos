/*****************************************************************************/
/**
*
* @file     xc7z020_xcwdtimer.h
*
* @brief    Watchdog timer header
*
******************************************************************************/

#ifndef XC7Z020_XCWDTIMER_H /* prevent circular inclusions */
#define XC7Z020_XCWDTIMER_H /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/************************** Constant Definitions *****************************/

/** @name Register Map
 * Offsets of registers from the start of the device
 * @{
 */

#define XSCUWDTIMER_LOAD_OFFSET                 0x00        /**< Watchdog Timer Load Register */
#define XSCUWDTIMER_COUNTER_OFFSET              0x04        /**< Watchdog Timer Counter Register */
#define XSCUWDTIMER_CONTROL_OFFSET              0x08        /**< Watchdog Timer Control Register */
#define XSCUWDTIMER_ISR_OFFSET                  0x0C        /**< Watchdog Timer Interrupt Status Register */
#define XSCUWDTIMER_RESET_OFFSET                0x10        /**< Watchdog Timer Reset Register */
#define XSCUWDTIMER_DISABLE_OFFSET              0x14        /**< Watchdog Timer Disable Register */
/* @} */

/** @name Watchdog Timer Control register
 * This register bits control the prescaler, WD mode, Intr enable,
 * auto-reload and timer enable.
 * @{
 */

#define XSCUWDTIMER_CONTROL_PRESCALER_MASK        0x0000FF00 /**< Prescaler */
#define XSCUWDTIMER_CONTROL_PRESCALER_SHIFT       8
#define XSCUWDTIMER_CONTROL_WD_MODE_MASK          0x00000008 /**< Watchdog mode enable */
#define XSCUWDTIMER_CONTROL_IRQ_ENABLE_MASK       0x00000004 /**< Intr enable */
#define XSCUWDTIMER_CONTROL_AUTO_RELOAD_MASK      0x00000002 /**< Auto-reload */
#define XSCUWDTIMER_CONTROL_ENABLE_MASK           0x00000001 /**< WD enable */
/* @} */

/** @name Interrupt Status register
 * This register bit indicates the WD counter register has reached zero.
 * @{
 */

#define XSCUWDTIMER_ISR_EVENT_FLAG_MASK           0x00000001 /**< Event flag */
/*@}*/

/** @name Reset Status register
 * This register bit indicates the WD reset register has 1.
 * @{
 */

#define XSCUWDTIMER_RESET_FLAG_MASK               0x00000001 /**< Reset flag */
/*@}*/

/** @name Disable sequence 1
 * This seq need to send first.
 * @{
 */

#define XSCUWDTIMER_WD_DISABLE_SEQ1               0x12345678
/*@}*/

/** @name Disable sequence 2
 * This seq need to send first.
 * @{
 */

#define XSCUWDTIMER_WD_DISABLE_SEQ2               0x87654321
/*@}*/

#ifdef __cplusplus
}
#endif

#endif            /* end of protection macro */

