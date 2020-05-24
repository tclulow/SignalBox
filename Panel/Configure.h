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
  

  /** Print -/+ characters around a variable.
   */
  void printMinusPlus(int aCol, int aRow, int aLen, boolean aShow)
  {
    lcd.printAt(aCol - 1,    aRow, aShow ? CHAR_MINUS : CHAR_SPACE);
    lcd.printAt(aCol + aLen, aRow, aShow ? CHAR_PLUS  : CHAR_SPACE);
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
      switch (waitForButton())
      {
        case BUTTON_NONE:   break;
        case BUTTON_UP:
        case BUTTON_DOWN:   isInput = !isInput;
                            lcd.printAt(LCD_COL_CONFIG, LCD_ROW_TOP, (isInput ? M_INPUT : M_OUTPUT));
                            break;
        case BUTTON_SELECT: 
        case BUTTON_LEFT:   stage -= 1;
                            break;
        case BUTTON_RIGHT:  stageModule();
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
    printMinusPlus(LCD_COL_MODULE, LCD_ROW_TOP, 1, true);

    while (stage == STAGE_MODULE)
    {
      switch (waitForButton())
      {
        case BUTTON_NONE:   break;
        case BUTTON_UP:     module = (module + 1) & (isInput ? INPUT_MODULE_MASK : OUTPUT_MODULE_MASK);
                            lcd.printAt(LCD_COL_MODULE, LCD_ROW_TOP, HEX_CHARS[module]);
                            break;
        case BUTTON_DOWN:   module = (module - 1) & (isInput ? INPUT_MODULE_MASK : OUTPUT_MODULE_MASK);
                            lcd.printAt(LCD_COL_MODULE, LCD_ROW_TOP, HEX_CHARS[module]);
                            break;
        case BUTTON_SELECT: 
        case BUTTON_LEFT:   stage -= 1;
                            break;
        case BUTTON_RIGHT:  printMinusPlus(LCD_COL_MODULE, LCD_ROW_TOP, 1, false);
                            lcd.clearRow(LCD_COL_MODULE - 1, LCD_ROW_BOT);
                            stagePin();
                            lcd.printAt(LCD_COL_MODULE - 1, LCD_ROW_BOT, M_MOD);
                            printMinusPlus(LCD_COL_MODULE, LCD_ROW_TOP, 1, true);
                            break;
      }
    }

    printMinusPlus(LCD_COL_MODULE, LCD_ROW_TOP, 1, false);
  }

  
  /** Process Pin stage.
   */
  void stagePin()
  {
    stage = STAGE_PIN;
    pin = pin & (isInput ? INPUT_INPUT_MASK : OUTPUT_OUTPUT_MASK);
    lcd.printAt(LCD_COL_PIN, LCD_ROW_TOP, HEX_CHARS[pin]);
    lcd.printAt(LCD_COL_PIN - 1, LCD_ROW_BOT, M_PIN);
    printMinusPlus(LCD_COL_PIN, LCD_ROW_TOP, 1, true);
    
    while (stage == STAGE_PIN)
    {
      switch (waitForButton())
      {
        case BUTTON_NONE:   break;
        case BUTTON_UP:     pin = (pin + 1) & (isInput ? INPUT_INPUT_MASK : OUTPUT_OUTPUT_MASK);
                            lcd.printAt(LCD_COL_PIN, LCD_ROW_TOP, HEX_CHARS[pin]);
                            break;
        case BUTTON_DOWN:   pin = (pin - 1) & (isInput ? INPUT_INPUT_MASK : OUTPUT_OUTPUT_MASK);
                            lcd.printAt(LCD_COL_PIN, LCD_ROW_TOP, HEX_CHARS[pin]);
                            break;
        case BUTTON_SELECT:
        case BUTTON_LEFT:   stage -= 1; 
                            break;
        case BUTTON_RIGHT:  printMinusPlus(LCD_COL_PIN, LCD_ROW_TOP, 1, false);
                            lcd.clearRow(LCD_COL_PIN - 1, LCD_ROW_BOT);
                            if (isInput)
                            {
                              stageInput();
                            }
                            else
                            {
                              stageOutput();
                            }
                            printMinusPlus(LCD_COL_PIN, LCD_ROW_TOP, 1, true);
                            lcd.printAt(LCD_COL_PIN - 1, LCD_ROW_BOT, M_PIN);
                            break;
      }
    }

    printMinusPlus(LCD_COL_PIN, LCD_ROW_TOP, 1, false);
  }

  
  /** Process Input stage.
   */
  void stageInput()
  {
    boolean changed = false;
    stage = STAGE_ADJUST;
    loadInput(module, pin);

    // Retrieve Toggle/Button flag and clear from data.
    uint8_t isButton = inputData.output[0] & INPUT_PUSH_TO_MAKE;
    inputData.output[0] &= INPUT_OUTPUT_MASK;

    // Output appropriate prompt.
    lcd.clearRow(LCD_COL_CONFIG, LCD_ROW_BOT);
    lcd.printAt(LCD_COL_CONFIG, LCD_ROW_BOT, (isButton ? M_BUTTON : M_TOGGLE));
    
    while (stage == STAGE_ADJUST)
    {
      switch (waitForButton())
      {
        case BUTTON_NONE:   break;
        case BUTTON_UP:
        case BUTTON_DOWN:   isButton ^= INPUT_PUSH_TO_MAKE;
                            lcd.printAt(LCD_COL_CONFIG, LCD_ROW_BOT, (isButton ? M_BUTTON : M_TOGGLE));
                            changed = true;
                            break;
        case BUTTON_SELECT: if (changed)
                            {
                              if (confirm())
                              {
                                inputData.output[0] |= isButton;
                                saveInput();
                                stage -= 1;
                              }
                              else
                              {
                                lcd.clearRow(LCD_COL_CONFIG, LCD_ROW_BOT);
                                lcd.printAt(LCD_COL_CONFIG, LCD_ROW_BOT, (isButton ? M_BUTTON : M_TOGGLE));
                              }
                            }
                            else
                            {
                              stage -= 1;
                            }
                            break;
        case BUTTON_LEFT:   if (changed)
                            {
                              if (cancel())
                              {
                                stage -= 1;
                              }
                              else
                              {
                                lcd.clearRow(LCD_COL_CONFIG, LCD_ROW_BOT);
                                lcd.printAt(LCD_COL_CONFIG, LCD_ROW_BOT, (isButton ? M_BUTTON : M_TOGGLE));
                              }
                            }
                            else
                            {
                              stage -= 1;
                            }
                            break;
        case BUTTON_RIGHT:  changed |= stageInputOutput();
                            break;
      }
    }

    lcd.clearRow(LCD_COL_CONFIG, LCD_ROW_BOT);
  }


  /** Process an Input's output definitions.
   */
  boolean stageInputOutput()
  {
    boolean changed = false;
    int     offset  = LCD_COL_INPUT_OUTPUT;
    int     index   = 0;
    uint8_t output   = inputData.output[index];

    while (index >= 0)
    {
      lcd.printAtHex(offset, LCD_ROW_BOT, output, 2);
      printMinusPlus(offset, LCD_ROW_BOT, 2, true);

      switch (waitForButton())
      {
        case BUTTON_NONE:   break;
        case BUTTON_UP:     changed = true;
                            output = (output + 1) & INPUT_OUTPUT_MASK;
                            lcd.printAtHex(offset, LCD_ROW_BOT, output, 2);
                            break;
        case BUTTON_DOWN:   changed = true;
                            output = (output - 1) & INPUT_OUTPUT_MASK;
                            lcd.printAtHex(offset, LCD_ROW_BOT, output, 2);
                            break;
        case BUTTON_SELECT: break;    // TODO disable/enable if index > 0
        case BUTTON_LEFT:   inputData.output[index] = output;
                            printMinusPlus(offset, LCD_ROW_BOT, 2, false);
                            index -= 1;
                            if (index >= 0)
                            {
                              output = inputData.output[index];
                              offset -= 3;
                              printMinusPlus(offset, LCD_ROW_BOT, 2, true);
                            }
                            break;
        case BUTTON_RIGHT:  inputData.output[index] = output;
                            index += 1;
                            if (index >= INPUT_OUTPUT_MAX)
                            {
                              index -= 1;
                            }
                            else
                            {
                              printMinusPlus(offset, LCD_ROW_BOT, 2, false);
                              output = inputData.output[index];
                              offset += 3;
                              lcd.printAtHex(offset, LCD_ROW_BOT, output, 2);
                              printMinusPlus(offset, LCD_ROW_BOT, 2, true);
                            }
                            break;
      }
    }

    return changed;
  }
  
  /** Process Output stage.
   */
  void stageOutput()
  {
    stage = STAGE_ADJUST;
    
    while (stage == STAGE_ADJUST)
    {
      switch (waitForButton())
      {
        case BUTTON_NONE:   break;
        case BUTTON_UP:     break;
        case BUTTON_DOWN:   break;
        case BUTTON_SELECT: 
        case BUTTON_LEFT:   stage -= 1;
                            break;
        case BUTTON_RIGHT:break;
      }
    }
  }


  /** Output confirmation message.
   */
  boolean confirm()
  {
    int offset = lcd.printAt(LCD_COL_CONFIG, LCD_ROW_BOT, M_CONFIRM);
    lcd.clearRow(offset, LCD_ROW_BOT);

    return waitForButton() == BUTTON_SELECT;
  }

  
  /** Output cancellation message.
   */
  boolean cancel()
  {
    int offset = lcd.printAt(LCD_COL_CONFIG, LCD_ROW_BOT, M_CANCEL);
    lcd.clearRow(offset, LCD_ROW_BOT);

    return waitForButton() == BUTTON_SELECT;
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
