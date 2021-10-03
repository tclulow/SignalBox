/** Buttons
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

#ifndef Buttons_h
#define Buttons_h


// The analog button
#define BUTTON_ANALOG 0


// Button values.
#define BUTTON_NONE   0
#define BUTTON_SELECT 1
#define BUTTON_LEFT   2
#define BUTTON_DOWN   3
#define BUTTON_UP     4
#define BUTTON_RIGHT  5

#define BUTTON_LOW    1             // Button range, 1 to 5
#define BUTTON_HIGH   5

#define BUTTON_THRESHHOLD   1000    // Analog limit above which no button is considered to be pressed.


/** Class for handling controll buttons.
 *  Both on the LCD shield and as Uno pins.
 */
class Buttons
{
    private:
    
    uint8_t lastButton = 0xff;      // Keep track of last button pressed for reporting in debug messages.
    

    /** Initialise the alternate button pins.
     */
    void initButtonPins()
    {
        for (uint8_t button = BUTTON_LOW; button <= BUTTON_HIGH; button++)
        {
            pinMode(BUTTON_PINS[button], INPUT_PULLUP);
        }
    }
    
    
    public:

    /** Constructor.
     */
    Buttons()
    {
        initButtonPins();           // Initialise alternate button pins.
    }

        
    /** Is button calibration required?
     */
    boolean calibrationRequired()
    {
        return systemData.buttons[BUTTON_NONE] == 0;
    }
    
    
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
        disp.printProgStrAt(LCD_COL_START, LCD_ROW_DET, M_PRESS);   // Announce we're ready to start
        systemData.buttons[BUTTON_HIGH] = 0;                        // Marker for last button.
    
        // Request values for all buttons in turn.
        for (int button = 0; button < BUTTON_HIGH; button++)
        {
            disp.clearRow(LCD_COL_CALIBRATE, LCD_ROW_DET);
            disp.printProgStrAt(LCD_COL_CALIBRATE, LCD_ROW_DET, M_BUTTONS[button + 1], LCD_LEN_OPTION);
    
            // Wait for a button to be pressed
            while ((value = analogRead(A0)) > BUTTON_THRESHHOLD)
            {
                delay(DELAY_BUTTON_WAIT);
            }
    
            if (isDebug(DEBUG_BRIEF))
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
    
            // Report the button's value (4 digits)
            disp.setCursor(LCD_COLS - 4, LCD_ROW_DET);
            disp.printDec(value, 4, CHAR_SPACE);
    
            // Wait for button to be released.
            while (analogRead(A0) < BUTTON_THRESHHOLD)
            {
                delay(DELAY_BUTTON_WAIT);
            }
    
            // Check for buttons out-of-sequence
            if (previous < value)
            {
                disp.printProgStrAt(LCD_COL_START, LCD_ROW_DET, M_SEQUENCE, LCD_COLS);
                delay(DELAY_READ);
    
                // Force start again.
                button = -1;
                previous = BUTTON_THRESHHOLD;
                disp.clearRow(LCD_COL_START, LCD_ROW_DET);
                disp.printProgStrAt(LCD_COL_START, LCD_ROW_DET, M_PRESS, LCD_COLS);
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
    
    
    /** Read the input button pressed.
     *  Return one of the button constants.
     *  Use the attached LCD shield, or the dedicated pins.
     */
    uint8_t readButton()
    {
        uint8_t button = BUTTON_NONE;
        int     value  = analogRead(BUTTON_ANALOG);
    
    //    static int previous = 0;
    //    if (value != previous)
    //    {
    //        previous = value;
    //        Serial.print(millis());
    //        Serial.print(" ");
    //        Serial.println(value);
    //    }
    
        if (hasLcdShield)
        {
            // See if BUTTON_ANALOG is pressed.
            // Analog cutoff is in the previous cell in the systemData.button array, so index from BUTTON_NONE.
            for (button = BUTTON_NONE; button < BUTTON_HIGH; button++)
            {
                if (value >= systemData.buttons[button])
                {
                    break;
                }
            }
        }
    
        // If no BUTTON_ANALOG pressed.
        if (button == BUTTON_NONE)
        {
            // Scan alternate buttons.
            for (button = BUTTON_HIGH; button >= BUTTON_LOW; button--)
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
        while (readButton() != BUTTON_NONE);
    
        delay(DELAY_BUTTON_WAIT);
    }
    
    
    /** Wait for a button to be pressed.
     *  Don't wait for it to be released.
     *  Return the button pressed.
     */
    uint8_t waitForButtonPress()
    {
        uint8_t button = BUTTON_NONE;
    
        waitForButtonRelease();
        while ((button = readButton()) == BUTTON_NONE)
        {
            delay(DELAY_BUTTON_WAIT);
        }
    
        return button;
    }
    
    
    /** Wait for a button to be pressed.
     *  Wait for it to be released.
     *  Abandon wait after DELAY_READ msecs.
     *  Return the button clicked.
     */
    uint8_t waitForButtonClick()
    {
        long delayTo = millis() + DELAY_READ;
        uint8_t button = BUTTON_NONE;
    
        waitForButtonRelease();
    
        while (   ((button = readButton()) == BUTTON_NONE)
               && (millis() < delayTo))
        {
            delay(DELAY_BUTTON_WAIT);
        }
    
        waitForButtonRelease();
    
        return button;
    }
};


// Singleton instance of the class.
Buttons buttons;


#endif
