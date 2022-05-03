/** Buttons
 *  @file
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


// Button values.
const uint8_t BUTTON_NONE       = 0;        // Button enums.
const uint8_t BUTTON_SELECT     = 1;
const uint8_t BUTTON_LEFT       = 2;
const uint8_t BUTTON_DOWN       = 3;
const uint8_t BUTTON_UP         = 4;
const uint8_t BUTTON_RIGHT      = 5;

const uint8_t BUTTON_LOW        = 1;        // Button range, 1 to 5
const uint8_t BUTTON_HIGH       = 5;  

const uint8_t BUTTON_ANALOG     =   A0;     // The analog button
const int     BUTTON_THRESHHOLD = 1000;     // Analog limit above which no button is considered to be pressed.
const uint8_t BUTTON_MARGIN     =   50;     // Analog buttons must be this far from each other to avoid possible confusion.


/** Class for handling control buttons.
 *  Both on the LCD shield and as Uno pins.
 */
class Buttons
{
    private:

    uint8_t lastButton = 0xff;      // Keep track of last button pressed for reporting in debug messages.
    int *   buttonsPtr;             // Array of button definitions (in the systemMgr object).


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
        buttonsPtr = systemMgr.getButtons();    // Pointer to the button settings as held by the SystemMgr.
        initButtonPins();                       // Initialise alternate button pins.
    }


    /** Is button calibration required?
     */
    bool calibrationRequired()
    {
        return buttonsPtr[BUTTON_NONE] == 0;
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
        buttonsPtr[BUTTON_HIGH] = 0;                                    // Marker for last button.

        // Request values for all buttons in turn.
        for (int button = 0; button < BUTTON_HIGH; button++)
        {
            // Announce the button to be calibrated.
            disp.printProgStrAt(LCD_COL_START, LCD_ROW_DET, M_PRESS, LCD_COLS);
            disp.printProgStrAt(LCD_COL_CALIBRATE, LCD_ROW_DET, M_BUTTONS[button + 1], LCD_LEN_OPTION);

            // Wait for a button to be pressed
            while ((value = analogRead(A0)) > BUTTON_THRESHHOLD)
            {
                delay(DELAY_BUTTON_WAIT);
            }

//            if (isDebug(DEBUG_BRIEF))
//            {
//                Serial.print(millis());
//                Serial.print(CHAR_TAB);
//                Serial.print(PGMT(M_BUTTONS[button + 1]));
//                Serial.print(PGMT(M_DEBUG_VALUE));
//                Serial.print(value, HEX);
//                Serial.print(PGMT(M_DEBUG_TARGET));
//                Serial.print(previous, HEX);
//                Serial.println();
//            }

            // Report the button's value (4 digits)
            disp.setCursor(LCD_COLS - 4, LCD_ROW_DET);
            disp.printDec(value, 4, CHAR_SPACE);

            // Wait for button to be released.
            while (analogRead(A0) < BUTTON_THRESHHOLD)
            {
                delay(DELAY_BUTTON_WAIT);
            }

            // Check for buttons out-of-sequence
            if (value > (previous - BUTTON_MARGIN))
            {
                disp.printProgStrAt(LCD_COL_START, LCD_ROW_DET, M_SEQUENCE, LCD_COLS);
                delay(DELAY_READ);

                // Force start again.
                button = -1;
                previous = BUTTON_THRESHHOLD;
            }
            else
            {
                // Record button barrier (half way between this button and the previous one).
                buttonsPtr[button] = (previous + value) / 2;
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
        int     analog = analogRead(BUTTON_ANALOG);

//        static int previous = 0;
//        if (value != previous)
//        {
//            previous = value;
//            Serial.print(millis());
//            Serial.print(" ");
//            Serial.println(value);
//        }

        if (disp.hasShield())
        {
            // See if BUTTON_ANALOG is pressed.
            // Analog cutoff is in the previous cell in the systemData.button array, so index from BUTTON_NONE.
            for (button = BUTTON_NONE; button < BUTTON_HIGH; button++)
            {
                if (analog >= buttonsPtr[button])
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
            Serial.print(analog, HEX);
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
        unsigned long delayTo = millis() + DELAY_READ;
        uint8_t       button  = BUTTON_NONE;

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
