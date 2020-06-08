/** Buttons
 */
#ifndef _Buttons_h
#define _Buttons_h


// The analog button
#define BUTTON_ANALOG 0


// Button values.
#define BUTTON_NONE   0
#define BUTTON_SELECT 1
#define BUTTON_LEFT   2
#define BUTTON_DOWN   3
#define BUTTON_UP     4
#define BUTTON_RIGHT  5
#define BUTTON_LIMIT  5


/** Calibrate the analog buttons.
 *  Each marker is half-way between the values the buttons return.
 */
void calibrateButtons();

/** Read the input button pressed.
 *  Return one of the constants above.
 */
int readButton();

/** Wait for button to be released.
 */
void waitForButtonRelease();

/** Wait for a button to be pressed.
 *  Return that button after waiting for the release.
 */
int waitForButton();


#endif
