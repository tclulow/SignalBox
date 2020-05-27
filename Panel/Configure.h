/** Setup hardware.
 */
#ifndef _Configure_h
#define _Configure_h

// Top-level menu states.
#define TOP_SYSTEM 0
#define TOP_INPUT  1
#define TOP_OUTPUT 2
#define TOP_MAX    3

#define SYS_I2C    0
#define SYS_BUTTON 1
#define SYS_MAX    2


/** Configure the system.
 */
class Configure
{
  private:

  int     topMenu = 0;      // Top menu being shown
  int     sysMenu = 0;      // System menu being shown.
  int     module  = 0;      // The module we're configuring.
  int     pin     = 0;      // The pin we're configuring.
  

  /** Display all current data.
   */
  void displayAll()
  {
    lcd.printAt(LCD_COL_START, LCD_ROW_TOP, M_TOP_MENU[topMenu]);
    switch (topMenu)
    {
      case TOP_SYSTEM: displaySystem();
                       break;
      case TOP_INPUT:  
      case TOP_OUTPUT: displayModule();
                       break;
    }

    displayDetail();
  }


  /** Display Sysyem information.
   */
  void displaySystem()
  {
    lcd.clearRow(LCD_COL_MARK, LCD_ROW_TOP);
  }


  void displayDetailSystem()
  {
    lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_SYS_TYPES[sysMenu]);
    switch (sysMenu)
    {
      case SYS_I2C:    displaySystemI2cParams();
                       break;
      case SYS_BUTTON: lcd.printAt(LCD_COL_MARK, LCD_ROW_BOT, M_TODO);
                       break;
    }
  }


  /** Display System's parameters depending on mode.
   */
  void displaySystemI2cParams()
  {
    int offset = LCD_COL_I2C_PARAM;

    lcd.clearRow(LCD_COL_MARK, LCD_ROW_BOT);
    
    lcd.printAtHex(offset, LCD_ROW_BOT, systemData.i2cControllerID, 2);
    offset += LCD_COL_OUTPUT_STEP;
    lcd.printAtHex(offset, LCD_ROW_BOT, systemData.i2cInputBaseID,  2);
    offset += LCD_COL_OUTPUT_STEP;
    lcd.printAtHex(offset, LCD_ROW_BOT, systemData.i2cOutputBaseID, 2);
  }



  /** Display an i2c parameter's prompt above it.
   */
  void displayI2cPrompt(int aParam)
  {
    lcd.clearRow(LCD_COL_I2C_PARAM, LCD_ROW_TOP);
    lcd.printAt(LCD_COL_I2C_PARAM + aParam * LCD_COL_I2C_STEP, LCD_ROW_TOP, M_I2C_PROMPTS[aParam]);
  }
  

  /** Display the module/pin selection line of the menu.
   */
  void displayModule()
  {
    lcd.clearRow(LCD_COL_MARK, LCD_ROW_TOP);
    lcd.printAt(LCD_COL_MODULE, LCD_ROW_TOP, HEX_CHARS[module]);
    lcd.printAt(LCD_COL_PIN   , LCD_ROW_TOP, HEX_CHARS[pin]);
  }


  /** Display the detail line of the menu.
   */
  void displayDetail()
  {
    lcd.clearRow(LCD_COL_START, LCD_ROW_BOT);
    switch (topMenu)
    {
      case TOP_SYSTEM: displayDetailSystem();
                       break;
      case TOP_INPUT:  displayDetailInput();
                       break;
      case TOP_OUTPUT: displayDetailOutput();
                       break;
    }
  }


  /** Display Input details.
   */
  void displayDetailInput()
  {
    int output = 0;
    int offset = LCD_COL_INPUT_OUTPUT;

    lcd.printAt(LCD_COL_START, LCD_ROW_BOT, (inputData.output[output] & INPUT_BUTTON_MASK ? M_BUTTON : M_TOGGLE));
    lcd.clearRow(LCD_COL_MARK, LCD_ROW_BOT);
    
    offset = LCD_COL_INPUT_OUTPUT;
    for (int output = 0; output < INPUT_OUTPUT_MAX; output++, offset += LCD_COL_INPUT_STEP)
    {
      displayInputOutput(offset, inputData.output[output] & (output == 0 ? INPUT_OUTPUT_MASK : 0xff));
    }
  }
  

  /** Show an output number (or disabled marker).
   */
  void displayInputOutput(int aOffset, int aOutput)
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
  

  /** Display Output details.
   */
  void displayDetailOutput()
  {
    lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_OUTPUT_TYPES[outputData.mode & OUTPUT_MODE_MASK]);
    displayOutputParams(outputData.mode & OUTPUT_MODE_MASK);
  }


  /** Display Output's parameters depending on mode.
   */
  void displayOutputParams(int aMode)
  {
    int offset = LCD_COL_OUTPUT_PARAM;

    lcd.clearRow(LCD_COL_MARK, LCD_ROW_BOT);
    
    if (aMode == OUTPUT_MODE_NONE)
    {
      lcd.clearRow(offset, LCD_ROW_BOT);
    }
    else
    {
      lcd.printAtHex(offset, LCD_ROW_BOT, outputData.lo,   2);
      offset += LCD_COL_OUTPUT_STEP;
      lcd.printAtHex(offset, LCD_ROW_BOT, outputData.hi,   2);
      offset += LCD_COL_OUTPUT_STEP;
      lcd.printAtHex(offset, LCD_ROW_BOT, outputData.pace, 2);
    }
  }


  /** Display an Output parameter's prompt above it.
   */
  void displayOutputPrompt(int aParam)
  {
    lcd.clearRow(LCD_COL_OUTPUT_PARAM, LCD_ROW_TOP);
    lcd.printAt(LCD_COL_OUTPUT_PARAM + aParam * LCD_COL_OUTPUT_STEP, LCD_ROW_TOP, M_OUTPUT_PROMPTS[aParam]);
  }
  

  /** Process IO stage.
   */
  void menuTop()
  {
    boolean finished = false;

    // Initialise state.
    loadInput(module, pin);
    loadOutput(module, pin);
    
    lcd.clear();
    displayAll();
    markField(LCD_COL_START, LCD_ROW_TOP, LCD_COL_MARK, true);

    while (!finished)
    {
      switch (waitForButton())
      {
        case BUTTON_NONE:   break;
        case BUTTON_UP:     topMenu += 2;     // Use +1 to compensate for the -1 that the code below will do.
                            if (topMenu > TOP_MAX)
                            {
                              topMenu = 1;
                            }
        case BUTTON_DOWN:   topMenu -= 1;
                            if (topMenu < 0)
                            {
                              topMenu = TOP_MAX - 1;
                            }
                            // Ensure module and pin are in-range
                            module = module & (topMenu == TOP_INPUT ? INPUT_MODULE_MASK : OUTPUT_MODULE_MASK);
                            pin    = pin    & (topMenu == TOP_INPUT ? INPUT_INPUT_MASK  : OUTPUT_OUTPUT_MASK);

                            displayAll();
                            markField(LCD_COL_START, LCD_ROW_TOP, LCD_COL_MARK, true);
                            break;
        case BUTTON_SELECT: break;
        case BUTTON_LEFT:   finished = true;
                            break;
        case BUTTON_RIGHT:  markField(LCD_COL_START, LCD_ROW_TOP, LCD_COL_MARK, false);
                            if (topMenu == TOP_SYSTEM)
                            {
                              menuSystem();
                            }
                            else if (   (topMenu == TOP_INPUT)
                                     || (topMenu == TOP_OUTPUT))
                            {
                              menuModule(topMenu == TOP_INPUT);
                            }
                            markField(LCD_COL_START, LCD_ROW_TOP, LCD_COL_MARK, true);
                            break;
      }
    }
  }


  /** Process System menu.
   */
  void menuSystem()
  {
    boolean finished = false;
    boolean changed = false;

    markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, true);

    while (!finished)
    {
      switch (waitForButton())
      {
        case BUTTON_NONE:   break;
        case BUTTON_UP:     sysMenu += 2;     // Use +1 to compensate for the -1 that the code below will do.
                            if (sysMenu > SYS_MAX)
                            {
                              sysMenu = 1;
                            }
        case BUTTON_DOWN:   sysMenu -= 1;
                            if (sysMenu < 0)
                            {
                              sysMenu = SYS_MAX - 1;
                            }
                            lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_SYS_TYPES[sysMenu]);
                            break;
        case BUTTON_SELECT: if (changed)
                            {
                              if (confirm())
                              {
                                saveSystemData();
                                lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_SAVED);
                                delay(DELAY);
                                displayDetailSystem();
                                finished = true;
                              }
                              else
                              {
                                displayDetailSystem();
                                markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, true);
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
                                loadSystemData();
                                lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_CANCELLED);
                                delay(DELAY);
                                displayDetailSystem();
                                finished = true;
                              }
                              else
                              {
                                displayDetailSystem();
                                markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, true);
                              }
                            }
                            else
                            {
                              finished = true;
                            }
                            break;
        case BUTTON_RIGHT:  markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, false);
                            if (sysMenu == SYS_I2C)
                            {
                              changed = menuSystemI2c();
                            }
                            {
                              // menuSystemButton();
                            }
                            markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, true);
                            break;
      }
    }

    markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, false);
  }


  /** Process System i2c menu.
   *  Reurn true if changes made.
   */
  boolean menuSystemI2c()
  {
    boolean changed = false;
    int index = 0;

    int params[] = { systemData.i2cControllerID, systemData.i2cInputBaseID, systemData.i2cOutputBaseID };
    displayI2cPrompt(index);
    markField(LCD_COL_I2C_PARAM, LCD_ROW_BOT, 2, true);

    while (index >= 0)
    {
      switch (waitForButton())
      {
        case BUTTON_NONE:   break;
        case BUTTON_UP:     params[index] += 1;
                            lcd.printAtHex(LCD_COL_I2C_PARAM + index * LCD_COL_I2C_STEP, LCD_ROW_BOT, params[index], 2);
                            changed = true;
                            break;
        case BUTTON_DOWN:   params[index] -= 1;
                            lcd.printAtHex(LCD_COL_I2C_PARAM + index * LCD_COL_I2C_STEP, LCD_ROW_BOT, params[index], 2);
                            changed = true;
                            break;
        case BUTTON_SELECT: break;
        case BUTTON_LEFT:   markField(LCD_COL_I2C_PARAM + index * LCD_COL_I2C_STEP, LCD_ROW_BOT, 2, false);
                            index -= 1;
                            if (index >= 0)
                            {
                              displayI2cPrompt(index);
                              markField(LCD_COL_I2C_PARAM + index * LCD_COL_I2C_STEP, LCD_ROW_BOT, 2, true);
                            }
                            break;
        case BUTTON_RIGHT:  if (index < 2)
                            {
                              markField(LCD_COL_I2C_PARAM + index * LCD_COL_I2C_STEP, LCD_ROW_BOT, 2, false);
                              index += 1;
                              displayI2cPrompt(index);
                              markField(LCD_COL_I2C_PARAM + index * LCD_COL_I2C_STEP, LCD_ROW_BOT, 2, true);
                            }
                            break;
      }
    }

    // Update outputData if changes have been made.
    if (changed)
    {
      systemData.i2cControllerID = params[0];
      systemData.i2cInputBaseID  = params[1];
      systemData.i2cOutputBaseID = params[2];
    }

    lcd.clearRow(LCD_COL_I2C_PARAM, LCD_ROW_TOP);
    return changed;
  }


  /** Process Module menu.
   *  For both input and output.
   */
  void menuModule(boolean aIsInput)
  {
    boolean finished = false;

    markField(LCD_COL_MODULE, LCD_ROW_TOP, 1, true);

    while (!finished)
    {
      switch (waitForButton())
      {
        case BUTTON_NONE:   break;
        case BUTTON_UP:     module += 2;
                            if (module > (aIsInput ? INPUT_MODULE_MAX : OUTPUT_MODULE_MAX))
                            {
                              module = 1;
                            }
        case BUTTON_DOWN:   module -= 1;
                            if (module < 0)
                            {
                              module = aIsInput ? INPUT_MODULE_MAX - 1 : OUTPUT_MODULE_MAX - 1;
                            }
                            lcd.printAt(LCD_COL_MODULE, LCD_ROW_TOP, HEX_CHARS[module]);
                            if (aIsInput)
                            {
                              loadInput(module, pin);
                            }
                            else
                            {
                              loadOutput(module, pin);
                            }
                            displayDetail();
                            break;
        case BUTTON_SELECT: break;
        case BUTTON_LEFT:   finished = true;
                            markField(LCD_COL_MODULE, LCD_ROW_TOP, 1, false);
                            break;
        case BUTTON_RIGHT:  markField(LCD_COL_MODULE, LCD_ROW_TOP, 1, false);
                            menuPin(aIsInput);
                            markField(LCD_COL_MODULE, LCD_ROW_TOP, 1, true);
                            break;
      }
    }

    markField(LCD_COL_MODULE, LCD_ROW_TOP, 1, false);
  }

  
  /** Process Pin menu.
   */
  void menuPin(boolean aIsInput)
  {
    boolean finished = false;
    
    markField(LCD_COL_PIN, LCD_ROW_TOP, 1, true);
    
    while (!finished)
    {
      switch (waitForButton())
      {
        case BUTTON_NONE:   break;
        case BUTTON_UP:     pin += 2;
                            if (pin > (aIsInput ? INPUT_MODULE_SIZE : OUTPUT_MODULE_SIZE))
                            {
                              pin = 1;
                            }
        case BUTTON_DOWN:   pin -= 1;
                            if (pin < 0)
                            {
                              pin = aIsInput ? INPUT_MODULE_SIZE - 1 : OUTPUT_MODULE_SIZE - 1;
                            }
                            lcd.printAt(LCD_COL_PIN, LCD_ROW_TOP, HEX_CHARS[pin]);
                            if (aIsInput)
                            {
                              loadInput(module, pin);
                            }
                            else
                            {
                              loadOutput(module, pin);
                            }
                            displayDetail();
                            break;
        case BUTTON_SELECT: break;
        case BUTTON_LEFT:   finished = true;
                            break;
        case BUTTON_RIGHT:  markField(LCD_COL_PIN, LCD_ROW_TOP, 1, false);
                            if (aIsInput)
                            {
                              menuInput();
                            }
                            else
                            {
                              menuOutput();
                            }
                            markField(LCD_COL_PIN, LCD_ROW_TOP, 1, true);
                            break;
      }
    }

    markField(LCD_COL_PIN, LCD_ROW_TOP, 1, false);
  }

  
  /** Process Input menu.
   */
  void menuInput()
  {
    boolean finished = false;
    boolean changed  = false;
    
    // Retrieve Toggle/Button flag and clear from data.
    int isButton = inputData.output[0] & INPUT_BUTTON_MASK;
    inputData.output[0] &= INPUT_OUTPUT_MASK;

    markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, true);
    
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
                                displayDetailInput();
                                finished = true;
                              }
                              else
                              {
                                displayDetailInput();
                                markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, true);
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
                                displayDetailInput();
                                finished = true;
                              }
                              else
                              {
                                displayDetailInput();
                                markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, true);
                              }
                            }
                            else
                            {
                              finished = true;
                            }
                            break;
        case BUTTON_RIGHT:  markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, false);
                            changed |= menuInputOutput();
                            markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, true);
                            break;
      }
    }
  }


  /** Process an Input's output definitions.
   */
  boolean menuInputOutput()
  {
    boolean changed = false;
    int     offset  = LCD_COL_INPUT_OUTPUT;
    int     index   = 0;
    int     output  = inputData.output[index];

    markField(offset, LCD_ROW_BOT, 2, true);

    while (index >= 0)
    {
      switch (waitForButton())
      {
        case BUTTON_NONE:   break;
        case BUTTON_UP:     if (output & INPUT_DISABLED_MASK)
                            {
                              output ^= INPUT_DISABLED_MASK;
                            }
                            else
                            {
                              output = (output + 1) & INPUT_OUTPUT_MASK;
                            }
                            displayInputOutput(offset, output);
                            changed = true;
                            break;
        case BUTTON_DOWN:   if (output & INPUT_DISABLED_MASK)
                            {
                              output ^= INPUT_DISABLED_MASK;
                            }
                            else
                            {
                              output = (output - 1) & INPUT_OUTPUT_MASK;
                            }
                            displayInputOutput(offset, output);
                            changed = true;
                            break;
        case BUTTON_SELECT: if (index > 0)
                            {
                              changed = true;
                              output ^= INPUT_DISABLED_MASK;
                              displayInputOutput(offset, output);
                            }
                            break;
        case BUTTON_LEFT:   inputData.output[index] = output;
                            markField(offset, LCD_ROW_BOT, 2, false);
                            index -= 1;
                            if (index >= 0)
                            {
                              output = inputData.output[index];
                              offset -= LCD_COL_INPUT_STEP;
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
                              offset += LCD_COL_INPUT_STEP;
                              displayInputOutput(offset, output);
                              markField(offset, LCD_ROW_BOT, 2, true);
                            }
                            break;
      }
    }

    return changed;
  }


  /** Process Output menu.
   */
  void menuOutput()
  {
    boolean finished = false;
    boolean changed  = false;
    
    // Retrieve type
    int outputMode = outputData.mode & OUTPUT_MODE_MASK;

    // Mark the field.
    markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, true);

    while (!finished)
    {
      switch (waitForButton())
      {
        case BUTTON_NONE:   break;
        case BUTTON_UP:     outputMode += 2;
                            if (outputMode > OUTPUT_MODE_MAX)
                            {
                              outputMode = 1;
                            }
        case BUTTON_DOWN:   outputMode -= 1;
                            if (outputMode < 0)
                            {
                              outputMode = OUTPUT_MODE_MAX - 1;
                            }
                            lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_OUTPUT_TYPES[outputMode]);
                            displayOutputParams(outputMode);
                            markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, true);
                            changed = true;
                            break;
        case BUTTON_SELECT: if (changed)
                            {
                              if (confirm())
                              {
                                outputData.mode = outputMode | (outputData.mode & ~OUTPUT_MODE_MASK);
                                saveOutput();
                                lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_SAVED);
                                delay(DELAY);
                                displayDetailOutput();
                                finished = true;
                              }
                              else
                              {
                                displayDetailOutput();
                                markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, true);
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
                                displayDetailOutput();
                                finished = true;
                              }
                              else
                              {
                                outputData.mode = outputMode | (outputData.mode & ~OUTPUT_MODE_MASK);
                                displayDetailOutput();
                                markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, true);
                              }
                            }
                            else
                            {
                              finished = true;
                            }
                            break;
        case BUTTON_RIGHT:  if (outputMode != OUTPUT_MODE_NONE)
                            {
                              markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, false);
                              changed |= menuOutputParms();
                              markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, true);
                            }
                            break;
      }
    }
  }


  /** Process Output's parameters menu.
   */
  boolean menuOutputParms()
  {
    boolean changed  = false;
    int     index    = 0;
    int     params[] = { outputData.lo, outputData.hi, outputData.pace }; 

    displayOutputPrompt(index);
    markField(LCD_COL_OUTPUT_PARAM, LCD_ROW_BOT, 2, true);

    while (index >= 0)
    {
      switch (waitForButton())
      {
        case BUTTON_NONE:   break;
        case BUTTON_UP:     params[index] += 1;
                            lcd.printAtHex(LCD_COL_OUTPUT_PARAM + index * LCD_COL_OUTPUT_STEP, LCD_ROW_BOT, params[index], 2);
                            changed = true;
                            break;
        case BUTTON_DOWN:   params[index] -= 1;
                            lcd.printAtHex(LCD_COL_OUTPUT_PARAM + index * LCD_COL_OUTPUT_STEP, LCD_ROW_BOT, params[index], 2);
                            changed = true;
                            break;
        case BUTTON_SELECT: break;
        case BUTTON_LEFT:   markField(LCD_COL_OUTPUT_PARAM + index * LCD_COL_OUTPUT_STEP, LCD_ROW_BOT, 2, false);
                            index -= 1;
                            if (index >= 0)
                            {
                              displayOutputPrompt(index);
                              markField(LCD_COL_OUTPUT_PARAM + index * LCD_COL_OUTPUT_STEP, LCD_ROW_BOT, 2, true);
                            }
                            break;
        case BUTTON_RIGHT:  if (index < 2)
                            {
                              markField(LCD_COL_OUTPUT_PARAM + index * LCD_COL_OUTPUT_STEP, LCD_ROW_BOT, 2, false);
                              index += 1;
                              displayOutputPrompt(index);
                              markField(LCD_COL_OUTPUT_PARAM + index * LCD_COL_OUTPUT_STEP, LCD_ROW_BOT, 2, true);
                            }
                            break;
      }
    }

    // Update outputData if changes have been made.
    if (changed)
    {
      outputData.lo   = params[0];
      outputData.hi   = params[1];
      outputData.pace = params[2];
    }

    displayModule();
    
    return changed;
  }


  /** Mark a variable with field markers.
   */
  void markField(int aCol, int aRow, int aLen, boolean aShow)
  {
    if (aCol > 0)
    {
      lcd.printAt(aCol - 1,    aRow, aShow ? CHAR_RIGHT : CHAR_SPACE);
    }
    lcd.printAt(aCol + aLen, aRow, aShow ? CHAR_LEFT  : CHAR_SPACE);
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

    menuTop();

    lcd.clear();
  }
};


/** A singleton instance of the class.
 */
Configure configure;

#endif
