/** Setup hardware.
 */
#ifndef _Configure_h
#define _Configure_h


// Stages of configuration.
#define STAGE_IO        1
#define STAGE_MODULE    2
#define STAGE_PIN       3
#define STAGE_ADJUST    4


/** Configure the system.
 */
class Configure
{
  private:
  
  int     button  = 0;      // Last button pressed.
  boolean isInput = true;   // Configuring inputs or outputs?
  int     module  = 0;      // The module we're configuring.
  int     pin     = 0;      // The pin we're configuring.
  int     stage   = 0;      // Stage we're at in configuration (see definitions above).
  

  /** Wait for button to be released.
   */
  void waitForButtonRelease()
  {
    while (readButton())
    {
      delay(100);
    }
  }


  /** Print -/+ characters around a variable.
   */
  void printMinusPlus(int aCol, int aRow, boolean aShow)
  {
    lcd.printAt(aCol - 1, aRow, aShow ? CHAR_MINUS : CHAR_SPACE);
    lcd.printAt(aCol + 1, aRow, aShow ? CHAR_PLUS  : CHAR_SPACE);
  }


  /** Process IO stage.
   */
  void stageIO()
  {
    stage = STAGE_IO;
    lcd.clear();
    lcd.printAt(LCD_COL_CONFIG, LCD_ROW_TOP, (isInput ? M_INPUT : M_OUTPUT));

    while (stage == STAGE_IO)
    {
      button = readButton();
      waitForButtonRelease();

      switch (button)
      {
        case BUTTON_NONE:   break;
        case BUTTON_UP:
        case BUTTON_DOWN:   isInput = !isInput;
                            lcd.printAt(LCD_COL_CONFIG, LCD_ROW_TOP, (isInput ? M_INPUT : M_OUTPUT));
                            break;
        case BUTTON_LEFT:   stage -= 1;
                            break;
        case BUTTON_RIGHT:
        case BUTTON_SELECT: stageModule();
                            break;
      }
    }
  }

  
  /** Process Module stage.
   */
  void stageModule()
  {
    stage = STAGE_MODULE;
    module = module & (isInput ? INPUT_MODULE_MASK : OUTPUT_MODULE_MASK);
    lcd.printAt(LCD_COL_MODULE, LCD_ROW_TOP, HEX_CHARS[module]);
    lcd.printAt(LCD_COL_MODULE - 1, LCD_ROW_BOT, M_MOD);
    printMinusPlus(LCD_COL_MODULE, LCD_ROW_TOP, true);

    while (stage == STAGE_MODULE)
    {
      button = readButton();
      waitForButtonRelease();

      switch (button)
      {
        case BUTTON_NONE:   break;
        case BUTTON_UP:     module = (module + 1) & (isInput ? INPUT_MODULE_MASK : OUTPUT_MODULE_MASK);
                            lcd.printAt(LCD_COL_MODULE, LCD_ROW_TOP, HEX_CHARS[module]);
                            break;
        case BUTTON_DOWN:   module = (module - 1) & (isInput ? INPUT_MODULE_MASK : OUTPUT_MODULE_MASK);
                            lcd.printAt(LCD_COL_MODULE, LCD_ROW_TOP, HEX_CHARS[module]);
                            break;
        case BUTTON_LEFT:   stage -= 1;
                            break;
        case BUTTON_RIGHT:
        case BUTTON_SELECT: printMinusPlus(LCD_COL_MODULE, LCD_ROW_TOP, false);
                            stagePin();
                            printMinusPlus(LCD_COL_MODULE, LCD_ROW_TOP, true);
                            break;
      }
    }

    printMinusPlus(LCD_COL_MODULE, LCD_ROW_TOP, false);
  }

  
  /** Process Pin stage.
   */
  void stagePin()
  {
    stage = STAGE_PIN;
    pin = pin & (isInput ? INPUT_INPUT_MASK : OUTPUT_OUTPUT_MASK);
    lcd.printAt(LCD_COL_PIN, LCD_ROW_TOP, HEX_CHARS[pin]);
    lcd.printAt(LCD_COL_PIN - 1, LCD_ROW_BOT, M_PIN);
    printMinusPlus(LCD_COL_PIN, LCD_ROW_TOP, true);
    
    while (stage == STAGE_PIN)
    {
      button = readButton();
      waitForButtonRelease();

      switch (button)
      {
        case BUTTON_NONE:   break;
        case BUTTON_UP:     pin = (pin + 1) & (isInput ? INPUT_INPUT_MASK : OUTPUT_OUTPUT_MASK);
                            lcd.printAt(LCD_COL_PIN, LCD_ROW_TOP, HEX_CHARS[pin]);
                            break;
        case BUTTON_DOWN:   pin = (pin - 1) & (isInput ? INPUT_INPUT_MASK : OUTPUT_OUTPUT_MASK);
                            lcd.printAt(LCD_COL_PIN, LCD_ROW_TOP, HEX_CHARS[pin]);
                            break;
        case BUTTON_LEFT:   stage -= 1;
                            break;
        case BUTTON_RIGHT:
        case BUTTON_SELECT: printMinusPlus(LCD_COL_PIN, LCD_ROW_TOP, false);
                            stageAdjust();
                            printMinusPlus(LCD_COL_PIN, LCD_ROW_TOP, true);
                            lcd.printAt(LCD_COL_MODULE - 1, LCD_ROW_BOT, M_MOD);
                            lcd.printAt(LCD_COL_PIN - 1, LCD_ROW_BOT, M_PIN);
                            break;
      }
    }

    printMinusPlus(LCD_COL_PIN, LCD_ROW_TOP, false);
  }

  
  /** Process Adjust stage.
   */
  void stageAdjust()
  {
    stage = STAGE_ADJUST;
    
    while (stage == STAGE_ADJUST)
    {
      button = readButton();
      waitForButtonRelease();

      switch (button)
      {
        case BUTTON_NONE:   break;
        case BUTTON_UP:     break;
        case BUTTON_DOWN:   break;
        case BUTTON_LEFT:   stage -= 1;
                            break;
        case BUTTON_RIGHT:
        case BUTTON_SELECT: break;
      }
    }
  }

  
  public:
  
  /** A Configure object.
   */
  Configure()
  {
  }


  /** Run configuration.
   */
  void run()
  {
    lcd.clear();
    lcd.printAt(0, 0, M_CONFIG);
    waitForButtonRelease();

    stageIO();

    lcd.clear();
  }
};


/** A singleton instance of the class.
 */
Configure configure;

#endif
