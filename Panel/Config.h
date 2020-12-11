/** Config data.
 */
#ifndef _Config_h
#define _Config_h


#define STEP_SCAN             50    // Steps in msecs between scans of the input switches.
#define STEP_HEARTBEAT       200    // Steps in msecs between changes of the heartbeat indicator.

#define DELAY_READ          2000    // Delay in msecs when the LCD may need to be read by an operator.
#define DELAY_BUTTON_WAIT     10    // Delay when waiting for button state to change.
#define DELAY_BUTTON_DELAY   250    // Delay before auto-repeating button.
#define DELAY_BUTTON_REPEAT  100    // Auto-repeat button when held continuously.


#endif
