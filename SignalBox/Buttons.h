/** Buttons
 *
 *
 *  (c)Copyright Tony Clulow  2021    tony.clulow@pentadtech.com
 *
 *  This work is licensed under the:
 *      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 *      http://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 *   For commercial use, please contact the original copyright holder(s) to agree licensing terms
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

#define BUTTON_THRESHHOLD   1000    // Analog limit above which no button is considered to be pressed.

// Alternate pins that can be used to control the menus.
const uint8_t BUTTON_PINS[] = { 0, A1, A2, 2, 3, A3 };


/** Initialise the alternate button pins.
 */
void initButtonPins()
{
    for (uint8_t button = 1; button <= BUTTON_LIMIT; button++)
    {
        pinMode(BUTTON_PINS[button], INPUT_PULLUP);
    }
}


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
