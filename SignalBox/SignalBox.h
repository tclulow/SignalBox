/** SignalBox constants.
 *  @file
 *
 *
 *  (c)Copyright Tony Clulow  2021    tony.clulow@pentadtech.com
 *
 *  This work is licensed under the:
 *      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 *      http://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 *  For commercial use, please contact the original copyright holder(s) to agree licensing terms.
 */

#ifndef SignalBox_h
#define SignalBox_h


/** Record state of input switches. Referenced by Configure object.
 */
uint16_t currentSwitchState[INPUT_NODE_MAX];    // Current state of inputs.


/** Is an LCD shield present?
 */
boolean hasLcdShield = LCD_SHIELD;              // An LCD shield is present.


#endif
