/*****************************************************************************/
/**
*
* @file     xc7z020_xptimer.h
*
* @brief    Private timer header
*
******************************************************************************/

#ifndef XC7Z020_XPTIMER_H /* prevent circular inclusions */
#define XC7Z020_XPTIMER_H /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/************************** Constant Definitions *****************************/

/** @name Register Map
 * Offsets of registers from the start of the device
 * @{
 */

#define XSCUTIMER_LOAD_OFFSET                   0x00       /**< Timer Load Register */
#define XSCUTIMER_COUNTER_OFFSET                0x04       /**< Timer Counter Register */
#define XSCUTIMER_CONTROL_OFFSET                0x08       /**< Timer Control Register */
#define XSCUTIMER_ISR_OFFSET                    0x0C       /**< Timer Interrupt Status Register */
/* @} */

/** @name Timer Control register
 * This register bits control the prescaler, Intr enable,
 * auto-reload and timer enable.
 * @{
 */

#define XSCUTIMER_CONTROL_PRESCALER_MASK        0x0000FF00 /**< Prescaler */
#define XSCUTIMER_CONTROL_PRESCALER_SHIFT       8
#define XSCUTIMER_CONTROL_IRQ_ENABLE_MASK       0x00000004 /**< Intr enable */
#define XSCUTIMER_CONTROL_AUTO_RELOAD_MASK      0x00000002 /**< Auto-reload */
#define XSCUTIMER_CONTROL_ENABLE_MASK           0x00000001 /**< Timer enable */
/* @} */

/** @name Interrupt Status register
 * This register indicates the Timer counter register has reached zero.
 * @{
 */

#define XSCUTIMER_ISR_EVENT_FLAG_MASK           0x00000001 /**< Event flag */
/*@}*/
 
#ifdef __cplusplus
}
#endif

#endif            /* end of protection macro */

