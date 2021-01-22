/** Buttons
 */


/** Calibrate the analog buttons.
 *  Each marker is half-way between the values the buttons return.
 */
void calibrateButtons()
{
    int previous = 1024;
    int value    = 0;

    // Announce calibration
    lcd.clear();
    lcd.printAt(LCD_COL_START, LCD_ROW_TOP, M_CALIBRATE);

    // Wait for no button being pressed
    while (analogRead(A0) < BUTTON_THRESHHOLD);

    // Now start calibration
    lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_PRESS);   // Announce we're ready to start
    systemData.buttons[BUTTON_LIMIT] = 0;               // Marker for last button.
    
    // Request values for all buttons in turn.
    for (int button = 0; button < BUTTON_LIMIT; button++)
    {
        lcd.clearRow(LCD_COL_CALIBRATE, LCD_ROW_BOT);
        lcd.printAt(LCD_COL_CALIBRATE, LCD_ROW_BOT, M_BUTTONS[button + 1], LCD_LEN_OPTION);

        // Record average between this button and the previous.
        while ((value = analogRead(A0)) > BUTTON_THRESHHOLD);
        delay(DELAY_BUTTON_WAIT);
        systemData.buttons[button] = (previous + value) / 2;
        previous = value;

        if (isDebug(DEBUG_DETAIL))
        {
            Serial.print(millis());
            Serial.print(CHAR_TAB);
            Serial.print(PGMT(M_BUTTONS[button + 1]));
            Serial.print(PGMT(M_DEBUG_VALUE));
            Serial.print(value, HEX);
            Serial.print(PGMT(M_DEBUG_TARGET));
            Serial.print(systemData.buttons[button], HEX);
            Serial.println();
        }

        // Wait for button to be released.
        while (analogRead(A0) < BUTTON_THRESHHOLD);
        delay(DELAY_BUTTON_WAIT);
    }

    lcd.clear();
}


int lastButton = -1;
/** Read the input button pressed.
 *  Return one of the constants above.
 */
int readButton()
{
    int button = 0;
    int value  = analogRead(BUTTON_ANALOG);

//    static int previous = 0;
//    if (value != previous)
//    {
//        previous = value;
//        Serial.print(millis());
//        Serial.print(" ");
//        Serial.println(value);
//    }

    for (button = 0; button < BUTTON_LIMIT; button++)
    {
        if (value >= systemData.buttons[button])
        {
            break;
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
int waitForButton()
{
    int button;
    
    waitForButtonRelease();
    
    do
    {
        delay(DELAY_BUTTON_WAIT);
    }
    while ((button = readButton()) == 0);
    
    delay(DELAY_BUTTON_WAIT);

    return button;
}
