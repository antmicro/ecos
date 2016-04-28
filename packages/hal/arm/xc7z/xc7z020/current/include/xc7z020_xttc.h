/*****************************************************************************/
/**
*
* @file     xc7z020_xttc.h
*
* @brief    Triple timer counter header
*
******************************************************************************/

#ifndef XC7Z020_XTTC_H /* prevent circular inclusions */
#define XC7Z020_XTTC_H /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/************************** Constant Definitions *****************************/

/** @name Counter offset
 * Offset for selecting proper counter
 */

#define XTTC_COUNTER_1                          0x00
#define XTTC_COUNTER_2                          0x04
#define XTTC_COUNTER_3                          0x08

/** @name Register Map
 * Offsets of registers from the start of the device
 * @{
 */

#define XTTC_CLOCK_CONTROL_OFFSET               0x00
#define XTTC_COUNTER_CONTROL_OFFSET             0x0C
#define XTTC_COUNTER_VALUE_OFFSET               0x18
#define XTTC_INTERVAL_COUNTER_OFFSET            0x24
#define XTTC_INTERRUPT_OFFSET                   0x54
#define XTTC_INTERRUPT_EN_OFFSET                0x60

/* @} */

/** @name Clock Control register
 *  @{
 */

#define XTTC_CLKCONTROL_PRESCALER_EN_SHIFT      0x00 /* Prescaler enable shift */
#define XTTC_CLKCONTROL_PRESCALER_VAL_SHIFT     0x01 /* Prescaler value shift */
#define XTTC_CLKCONTROL_PRESCALER_MASK          0x0000000F /* Prescaler value mask */
#define XTTC_CLKCONTROL_PRE_EN_CLR_MASK         0xFFFFFFFE /* Clear prescaler enable bit */

/*  @} */

/** @name Counter Control register
 *  @{
 */

#define XTTC_CNTCONTROL_DIS_SHIFT               0x00 /* Counter disable shift */
#define XTTC_CNTCONTROL_INT_SHIFT               0x01 /* Counter interrupt enable shift */
#define XTTC_CNTCONTROL_DECR_SHIFT              0x02 /* Counter decrement enable shift */
#define XTTC_CNTCONTROL_RST_SHIFT               0x04 /* Counter reset shift */

#define XTTC_CNTCONTROL_DIS_CLR_MASK            0xFFFFFFFE
#define XTTC_CNTCONTROL_DIS_SET_MASK            0x00000001
/*  @} */

/** @name Counter Value register
 *  Stores counter register
 *  @{
 */

#define XTTC_COUNTER_VALUE_MASK                 0x0000FFFF /* Counter Value Mask */
/*  @} */


/** @name Interval register
 *  If interval is enabled, this is the maximum value
 *  that the counter will count up to or down from.
 *  @{
 */

#define XTTC_INTERVAL_VALUE_MASK                0x0000FFFF /* Interval value mask */
/*  @} */

/** @name Interrupt register
 *  @{
 */
#define XTTC_INT_INTERVAL_SHIFT                 0x00
#define XTTC_INT_INTERVAL_MASK                  0x00000001 /* Interval interrupt mask */
/*  @} */

/** @name Interrupt enable register
 *  @{
 */

#define XTTC_INTEN_INTERVAL_SHIFT               0x00

/*  @} */

#ifdef __cplusplus
}
#endif

#endif            /* end of protection macro */

