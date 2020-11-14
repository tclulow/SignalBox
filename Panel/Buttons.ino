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
  while (analogRead(0) < BUTTON_THRESHHOLD);

  // Now start calibration
  lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_PRESS);   // Announce we're ready to start
  systemData.buttons[BUTTON_LIMIT] = 0;               // Marker for last button.
  
  // Request values for all buttons in turn.
  for (int button = 0; button < BUTTON_LIMIT; button++)
  {
    lcd.clearRow(LCD_COL_CALIBRATE, LCD_ROW_BOT);
    lcd.printAt(LCD_COL_CALIBRATE, LCD_ROW_BOT, M_BUTTONS[button], LCD_LEN_OPTION);

    // Record average between this button and the previous.
    while ((value = analogRead(0)) > BUTTON_THRESHHOLD);
    systemData.buttons[button] = (previous + value) / 2;
    previous = value;

    // Wait for button to be released.
    while (analogRead(0) < BUTTON_THRESHHOLD);
  }

  lcd.clear();
}


/** Read the input button pressed.
 *  Return one of the constants above.
 */
int readButton()
{
  int value = analogRead(BUTTON_ANALOG);

//  #if DEBUG
//  static int previous = 0;
//  if (value != previous)
//  {
//    previous = value;
//    Serial.print(millis());
//    Serial.print(" ");
//    Serial.println(value);
//  }
//  #endif

  for (int button = 0; button < BUTTON_LIMIT; button++)
  {
    if (value >= systemData.buttons[button])
    {
      return button;
    }
  }

  // Shouldn't get here, but return end case.
  return BUTTON_RIGHT;
}


/** Wait for button to be released.
 */
void waitForButtonRelease()
{
  while (readButton())
  {
    delay(DELAY_BUTTON_WAIT);
  }
}


/** Wait for a button to be pressed.
 *  First wair for all buttons to be released.
 *  Return the button pressed.
 */
int waitForButton()
{
  int button;
  
  waitForButtonRelease();

  while ((button = readButton()) == 0)
  {
    delay(DELAY_BUTTON_WAIT);
  }

  return button;
}
