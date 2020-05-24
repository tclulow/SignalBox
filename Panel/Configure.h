/** Setup hardware.
 */
#ifndef _Configure_h
#define _Configure_h


/** Configure the system.
 */
class Configure
{
  private:
  
  int     button  = 0;      // Last button pressed.
  boolean isInput = true;   // Configuring inputs or outputs?
  int     module  = 0;      // The module we're configuring.
  int     pin     = 0;      // The pin we're configuring.
  

  /** Print -/+ characters around a variable.
   */
  void markField(int aCol, int aRow, int aLen, boolean aShow)
  {
    if (aCol > 0)
    {
      lcd.printAt(aCol - 1,    aRow, aShow ? CHAR_RIGHT : CHAR_SPACE);
    }
    lcd.printAt(aCol + aLen, aRow, aShow ? CHAR_LEFT  : CHAR_SPACE);
  }


  /** Process IO stage.
   */
  void stageIO()
  {
    boolean finished = false;
    
    lcd.clear();
    int offset = lcd.printAt(LCD_COL_START, LCD_ROW_TOP, (isInput ? M_INPUT : M_OUTPUT));
    markField(LCD_COL_START, LCD_ROW_TOP, offset, true);

    while (!finished)
    {
      switch (waitForButton())
      {
        case BUTTON_NONE:   break;
        case BUTTON_UP:
        case BUTTON_DOWN:   isInput = !isInput;
                            offset = lcd.printAt(LCD_COL_START, LCD_ROW_TOP, (isInput ? M_INPUT : M_OUTPUT));
                            break;
        case BUTTON_SELECT: break;
        case BUTTON_LEFT:   finished = true;
                            break;
        case BUTTON_RIGHT:  markField(LCD_COL_START, LCD_ROW_TOP, offset, false);
                            stageModule();
                            markField(LCD_COL_START, LCD_ROW_TOP, offset, true);
                            break;
      }
    }
  }

  
  /** Process Module stage.
   */
  void stageModule()
  {
    boolean finished = false;
    
    module = module & (isInput ? INPUT_MODULE_MASK : OUTPUT_MODULE_MASK);
    lcd.printAt(LCD_COL_MODULE, LCD_ROW_TOP, HEX_CHARS[module]);
    lcd.printAt(LCD_COL_MODULE - 1, LCD_ROW_BOT, M_MOD);
    markField(LCD_COL_MODULE, LCD_ROW_TOP, 1, true);

    while (!finished)
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
        case BUTTON_SELECT: break;
        case BUTTON_LEFT:   finished = true;
                            lcd.clearRow(LCD_COL_MODULE - 1, LCD_ROW_BOT);
                            break;
        case BUTTON_RIGHT:  markField(LCD_COL_MODULE, LCD_ROW_TOP, 1, false);
                            lcd.clearRow(LCD_COL_MODULE - 1, LCD_ROW_BOT);
                            stagePin();
                            lcd.printAt(LCD_COL_MODULE - 1, LCD_ROW_BOT, M_MOD);
                            markField(LCD_COL_MODULE, LCD_ROW_TOP, 1, true);
                            break;
      }
    }

    markField(LCD_COL_MODULE, LCD_ROW_TOP, 1, false);
  }

  
  /** Process Pin stage.
   */
  void stagePin()
  {
    boolean finished = false;
    
    pin = pin & (isInput ? INPUT_INPUT_MASK : OUTPUT_OUTPUT_MASK);
    lcd.printAt(LCD_COL_PIN, LCD_ROW_TOP, HEX_CHARS[pin]);
    lcd.printAt(LCD_COL_PIN - 1, LCD_ROW_BOT, M_PIN);
    markField(LCD_COL_PIN, LCD_ROW_TOP, 1, true);
    
    while (!finished)
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
        case BUTTON_SELECT: break;
        case BUTTON_LEFT:   finished = true;
                            lcd.clearRow(LCD_COL_PIN - 1, LCD_ROW_BOT);
                            break;
        case BUTTON_RIGHT:  markField(LCD_COL_PIN, LCD_ROW_TOP, 1, false);
                            lcd.clearRow(LCD_COL_PIN - 1, LCD_ROW_BOT);
                            if (isInput)
                            {
                              stageInput();
                            }
                            else
                            {
                              stageOutput();
                            }
                            markField(LCD_COL_PIN, LCD_ROW_TOP, 1, true);
                            lcd.printAt(LCD_COL_PIN - 1, LCD_ROW_BOT, M_PIN);
                            break;
      }
    }

    markField(LCD_COL_PIN, LCD_ROW_TOP, 1, false);
  }

  
  /** Process Input stage.
   */
  void stageInput()
  {
    boolean finished = false;
    boolean changed  = false;
    
    loadInput(module, pin);

    // Retrieve Toggle/Button flag and clear from data.
    uint8_t isButton = inputData.output[0] & INPUT_BUTTON_MASK;
    inputData.output[0] &= INPUT_OUTPUT_MASK;

    // Output appropriate prompt.
    lcd.clearRow(LCD_COL_START, LCD_ROW_BOT);
    int offset = lcd.printAt(LCD_COL_START, LCD_ROW_BOT, (isButton ? M_BUTTON : M_TOGGLE));
    markField(LCD_COL_START, LCD_ROW_BOT, offset, true);
    printInputOutputs();
    
    while (!finished)
    {
      switch (waitForButton())
      {
        case BUTTON_NONE:   break;
        case BUTTON_UP:
        case BUTTON_DOWN:   isButton ^= INPUT_BUTTON_MASK;
                            lcd.printAt(LCD_COL_START, LCD_ROW_BOT, (isButton ? M_BUTTON : M_TOGGLE));
                            changed = true;
                            break;
        case BUTTON_SELECT: if (changed)
                            {
                              if (confirm())
                              {
                                inputData.output[0] |= isButton;
                                saveInput();
                                lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_SAVED);
                                delay(DELAY);
                                finished = true;
                              }
                              else
                              {
                                lcd.clearRow(LCD_COL_START, LCD_ROW_BOT);
                                lcd.printAt(LCD_COL_START, LCD_ROW_BOT, (isButton ? M_BUTTON : M_TOGGLE));
                                markField(LCD_COL_START, LCD_ROW_BOT, offset, true);
                                printInputOutputs();
                              }
                            }
                            else
                            {
                              finished = true;
                            }
                            break;
        case BUTTON_LEFT:   if (changed)
                            {
                              if (cancel())
                              {
                                lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_CANCELLED);
                                delay(DELAY);
                                finished = true;
                              }
                              else
                              {
                                lcd.clearRow(LCD_COL_START, LCD_ROW_BOT);
                                lcd.printAt(LCD_COL_START, LCD_ROW_BOT, (isButton ? M_BUTTON : M_TOGGLE));
                                markField(LCD_COL_START, LCD_ROW_BOT, offset, true);
                                printInputOutputs();
                              }
                            }
                            else
                            {
                              finished = true;
                            }
                            break;
        case BUTTON_RIGHT:  markField(LCD_COL_START, LCD_ROW_BOT, offset, false);
                            changed |= stageInputOutput();
                            markField(LCD_COL_START, LCD_ROW_BOT, offset, true);
                            printInputOutputs();
                            break;
      }
    }

    lcd.clearRow(LCD_COL_START, LCD_ROW_BOT);
  }


  /** Print the Input's outputs.
   */
  void printInputOutputs()
  {
    int offset = LCD_COL_INPUT_OUTPUT;
    for (int output = 0; output < INPUT_OUTPUT_MAX; output++, offset += 3)
    {
      printOutput(offset, inputData.output[output]);
    }
  }
  

  /** Process an Input's output definitions.
   */
  boolean stageInputOutput()
  {
    boolean changed = false;
    int     offset  = LCD_COL_INPUT_OUTPUT;
    int     index   = 0;
    uint8_t output  = inputData.output[index];

    markField(offset, LCD_ROW_BOT, 2, true);

    while (index >= 0)
    {
      switch (waitForButton())
      {
        case BUTTON_NONE:   break;
        case BUTTON_UP:     changed = true;
                            if (output & INPUT_DISABLED_MASK)
                            {
                              output &= INPUT_OUTPUT_MASK;
                            }
                            else
                            {
                              output = (output + 1) & INPUT_OUTPUT_MASK;
                            }
                            printOutput(offset, output);
                            break;
        case BUTTON_DOWN:   changed = true;
                            if (output & INPUT_DISABLED_MASK)
                            {
                              output &= INPUT_OUTPUT_MASK;
                            }
                            else
                            {
                              output = (output - 1) & INPUT_OUTPUT_MASK;
                            }
                            printOutput(offset, output);
                            break;
        case BUTTON_SELECT: if (index > 0)
                            {
                              changed = true;
                              output ^= INPUT_DISABLED_MASK;
                              printOutput(offset, output);
                            }
                            break;
        case BUTTON_LEFT:   inputData.output[index] = output;
                            markField(offset, LCD_ROW_BOT, 2, false);
                            index -= 1;
                            if (index >= 0)
                            {
                              output = inputData.output[index];
                              offset -= 3;
                              markField(offset, LCD_ROW_BOT, 2, true);
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
                              markField(offset, LCD_ROW_BOT, 2, false);
                              output = inputData.output[index];
                              offset += 3;
                              printOutput(offset, output);
                              markField(offset, LCD_ROW_BOT, 2, true);
                            }
                            break;
      }
    }

    return changed;
  }


  void printOutput(int aOffset, int aOutput)
  {
    if (aOutput & INPUT_DISABLED_MASK)
    {
      lcd.printAt(aOffset, LCD_ROW_BOT, M_DISABLED);
    }
    else
    {
      lcd.printAtHex(aOffset, LCD_ROW_BOT, aOutput, 2);
    }
  }
  
  
  /** Process Output stage.
   */
  void stageOutput()
  {
    boolean finished = false;
    
    while (!finished)
    {
      switch (waitForButton())
      {
        case BUTTON_NONE:   break;
        case BUTTON_UP:     break;
        case BUTTON_DOWN:   break;
        case BUTTON_SELECT: break;
        case BUTTON_LEFT:   finished = true;
                            break;
        case BUTTON_RIGHT:break;
      }
    }
  }


  /** Output confirmation message.
   */
  boolean confirm()
  {
    int offset = lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_CONFIRM);
    lcd.clearRow(offset, LCD_ROW_BOT);

    return waitForButton() == BUTTON_SELECT;
  }

  
  /** Output cancellation message.
   */
  boolean cancel()
  {
    int offset = lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_CANCEL);
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
