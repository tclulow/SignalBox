/** Buttons
 *
 *
 *  (c)Copyright Tony Clulow  2021    tony.clulow@pentadtech.com
 *
 *  This work is licensed under the:
 *      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 *      http://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 *  For commercial use, please contact the original copyright holder(s) to agree licensing terms
 */


#include "All.h"


/** Calibrate the analog buttons.
 *  Each marker is half-way between the values the buttons return.
 */
void calibrateButtons()
{
    int previous = BUTTON_THRESHHOLD;
    int value    = 0;

    // Announce calibration
    disp.clear();
    disp.printProgStrAt(LCD_COL_START, LCD_ROW_TOP, M_CALIBRATE);

    // Wait for no button being pressed
    while (analogRead(A0) < BUTTON_THRESHHOLD);

    // Now start calibration
    disp.printProgStrAt(LCD_COL_START, LCD_ROW_BOT, M_PRESS);    // Announce we're ready to start
    systemData.buttons[BUTTON_LIMIT] = 0;                       // Marker for last button.
    
    // Request values for all buttons in turn.
    for (int button = 0; button < BUTTON_LIMIT; button++)
    {
        disp.clearRow(LCD_COL_CALIBRATE, LCD_ROW_BOT);
        disp.printProgStrAt(LCD_COL_CALIBRATE, LCD_ROW_BOT, M_BUTTONS[button + 1], LCD_LEN_OPTION);

        // Wait for a button to be pressed
        while ((value = analogRead(A0)) > BUTTON_THRESHHOLD);
        delay(DELAY_BUTTON_WAIT);

        if (isDebug(DEBUG_ERRORS))
        {
            Serial.print(millis());
            Serial.print(CHAR_TAB);
            Serial.print(PGMT(M_BUTTONS[button + 1]));
            Serial.print(PGMT(M_DEBUG_VALUE));
            Serial.print(value, HEX);
            Serial.print(PGMT(M_DEBUG_TARGET));
            Serial.print(previous, HEX);
            Serial.println();
        }

        // Report the button's value
        disp.setCursor(LCD_COLS - 4, LCD_ROW_BOT);
        disp.printDec(value, 4, CHAR_SPACE);

        // Wait for button to be released.
        while (analogRead(A0) < BUTTON_THRESHHOLD);
        delay(DELAY_BUTTON_WAIT);

        // Check for buttons out-of-sequence
        if (previous < value)
        {
            disp.printProgStrAt(LCD_COL_START, LCD_ROW_BOT, M_SEQUENCE, LCD_COLS);
            delay(DELAY_READ);

            // Force start again.
            button = -1;
            previous = BUTTON_THRESHHOLD;
            disp.clearRow(LCD_COL_START, LCD_ROW_BOT);
            disp.printProgStrAt(LCD_COL_START, LCD_ROW_BOT, M_PRESS, LCD_COLS);
        }
        else
        {
            // Record button barrier (half way between this button and the previous one).
            systemData.buttons[button] = (previous + value) / 2;
            previous = value;
        }
    }

    disp.clear();
}


uint8_t lastButton = 0xff;
/** Read the input button pressed.
 *  Return one of the button constants.
 */
uint8_t readButton()
{
    uint8_t button = 0;
    int value  = analogRead(BUTTON_ANALOG);

//    static int previous = 0;
//    if (value != previous)
//    {
//        previous = value;
//        Serial.print(millis());
//        Serial.print(" ");
//        Serial.println(value);
//    }

    // See if BUTTON_ANALOG is pressed.
    for (button = 0; button < BUTTON_LIMIT; button++)
    {
        if (value >= systemData.buttons[button])
        {
            break;
        }
    }

    // If no BUTTON_ANALOG pressed.
    if (button == BUTTON_NONE)
    {
        // Scan alternate buttons.
        for (button = BUTTON_LIMIT; button > BUTTON_NONE; button--)
        {
            if (!digitalRead(BUTTON_PINS[button]))
            {
                break;
            }
        }
    }

    if (   (button != lastButton)
        && (isDebug(DEBUG_FULL)))
    {
        Serial.print(millis());
        Serial.print(CHAR_TAB);
        Serial.print(PGMT(M_DEBUG_BUTTON));
        Serial.print(CHAR_SPACE);
        Serial.print(PGMT(M_BUTTONS[button]));
        Serial.print(PGMT(M_DEBUG_VALUE));
        Serial.print(value, HEX);
        Serial.println();
    }
    lastButton = button;
    
    return button;
}


/** Wait for button to be released.
 *  With delays to suppress contact-bounce.
 */
void waitForButtonRelease()
{
    do
    {
        delay(DELAY_BUTTON_WAIT);
    }
    while (readButton());

    delay(DELAY_BUTTON_WAIT);
}


/** Wait for a button to be pressed.
 *  First wait for all buttons to be released.
 *  Return the button pressed.
 */
uint8_t waitForButton()
{
    waitForButtonRelease();
    
    return waitForButton(0);
}


/** Wait for a button to be pressed.
 *  Return the button pressed.
 *  Abandon wait after aMaxWait msecs (unless aMaxWait is 0).
 */
uint8_t waitForButton(int aMaxWait)
{
    long delayTo = millis() + aMaxWait;
    uint8_t button;
    
    do
    {
        delay(DELAY_BUTTON_WAIT);
    }
    while (   ((button = readButton()) == BUTTON_NONE)
           && (   (aMaxWait == 0)
               || (millis() < delayTo)));
    
    delay(DELAY_BUTTON_WAIT);

    return button;
}
