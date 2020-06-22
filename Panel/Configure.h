/** Setup hardware.
 */
#ifndef _Configure_h
#define _Configure_h

// Top-level menu states.
#define TOP_SYSTEM   0
#define TOP_INPUT    1
#define TOP_OUTPUT   2
#define TOP_EXPORT   3
#define TOP_IMPORT   4
#define TOP_MAX      5

// Sys menu states.
#define SYS_DEBUG    0
#define SYS_I2C      1
#define SYS_MAX      2


/** Configure the system.
 */
class Configure
{
  private:

  int topMenu = 0;      // Top menu being shown
  int sysMenu = 0;      // System menu being shown.
  int expMenu = 0;      // Report menu being shown.
  int node    = 0;      // The node we're configuring.
  int pin     = 0;      // The pin we're configuring.
  

  /** Display all current data.
   */
  void displayAll()
  {
    lcd.printAt(LCD_COL_START, LCD_ROW_TOP, M_TOP_MENU[topMenu], LCD_LEN_OPTION);
    switch (topMenu)
    {
      case TOP_SYSTEM: displaySystem();
                       break;
      case TOP_INPUT:  pin  &= INPUT_PIN_MASK;
                       node &= INPUT_NODE_MASK;
                       if (!isInputNode(node))
                       {
                         node = nextNode(node, 1, true);
                       }
                       displayNode();
                       break;
      case TOP_OUTPUT: pin  &= OUTPUT_PIN_MASK;
                       node &= OUTPUT_NODE_MASK;
                       if (!isOutputNode(node))
                       {
                         node = nextNode(node, 1, false);
                       }
                       displayNode();
                       break;
      case TOP_EXPORT: 
      case TOP_IMPORT: displaySystem();
                       break;
      default:         systemFail(M_ALL, topMenu);
                       break;
    }

    displayDetail();
  }


  /** Display Sysyem information.
   */
  void displaySystem()
  {
    lcd.clearRow(LCD_COL_MARK, LCD_ROW_TOP);
    lcd.printAt(LCD_COLS - strlen_P(M_VERSION), LCD_ROW_TOP, M_VERSION);
  }


  /** Display the node/pin selection line of the menu.
   */
  void displayNode()
  {
    lcd.clearRow(LCD_COL_MARK, LCD_ROW_TOP);
    lcd.printAt(LCD_COL_NODE,  LCD_ROW_TOP, HEX_CHARS[node]);
    lcd.printAt(LCD_COL_PIN,   LCD_ROW_TOP, HEX_CHARS[pin]);
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
      case TOP_EXPORT: displayDetailExport();
                       break;
      case TOP_IMPORT: displayDetailImport();
                       break;
      default:         systemFail(M_DETAIL, topMenu);
                       break;
    }
  }


  /** Display detail for System menu.
   */
  void displayDetailSystem()
  {
    lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_SYS_TYPES[sysMenu], LCD_LEN_OPTION);
    displaySystemParams();
  }


  /** Display parameters for System menu.
   */
  void displaySystemParams()
  {
    switch (sysMenu)
    {
      case SYS_DEBUG:  displaySystemDebugParams();
                       break;
      case SYS_I2C:    displaySystemI2cParams();
                       break;
      default:         systemFail(M_PARAMS, topMenu);
                       break;
    }
  }


  /** Display System's I2C parameters.
   */
  void displaySystemI2cParams()
  {
    int col = LCD_COL_I2C_PARAM;

    lcd.clearRow(LCD_COL_MARK, LCD_ROW_BOT);
    
    lcd.printAtHex(col, LCD_ROW_BOT, systemData.i2cControllerID, 2);
    col += LCD_COL_OUTPUT_STEP;
    lcd.printAtHex(col, LCD_ROW_BOT, systemData.i2cInputBaseID,  2);
    col += LCD_COL_OUTPUT_STEP;
    lcd.printAtHex(col, LCD_ROW_BOT, systemData.i2cOutputBaseID, 2);
  }



  /** Display an i2c parameter's prompt above it.
   */
  void displayI2cPrompt(int aParam)
  {
    lcd.clearRow(LCD_COL_I2C_PARAM, LCD_ROW_TOP);
    lcd.printAt(LCD_COL_I2C_PARAM + aParam * LCD_COL_I2C_STEP, LCD_ROW_TOP, M_I2C_PROMPTS[aParam], LCD_LEN_OPTION);
  }


  /** Display System's debug parameter.
   */
  void displaySystemDebugParams()
  {
    lcd.clearRow(LCD_COL_MARK, LCD_ROW_BOT);
    lcd.printAt(LCD_COL_DEBUG_PARAM, LCD_ROW_BOT, M_DEBUG_PROMPTS[systemData.debugLevel], LCD_LEN_OPTION);    
  }


  /** Display Export detail menu.
   */
  void displayDetailExport()
  {
    lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_REPORT_TYPES[expMenu], LCD_LEN_OPTION);
  }
  

  /** Display Import detail menu.
   */
  void displayDetailImport()
  {
    lcd.clearRow(LCD_COL_START, LCD_ROW_BOT);
  }
  

  /** Display Input details.
   */
  void displayDetailInput()
  {
    lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_INPUT_TYPES[inputType], LCD_LEN_OPTION);
    displayDetailInputOutput();
  }


  /** Display Input's Output details.
   */
  void displayDetailInputOutput()
  {
    int col = LCD_COL_INPUT_OUTPUT;

    lcd.clearRow(LCD_COL_MARK, LCD_ROW_BOT);
    for (int output = 0; output < INPUT_OUTPUT_MAX; output++, col += LCD_COL_INPUT_STEP)
    {
      displayInputOutput(col, inputData.output[output] & (output == 0 ? INPUT_OUTPUT_MASK : 0xff));
    }
  }
  

  /** Show an output number (or disabled marker).
   */
  void displayInputOutput(int aCol, int aOutput)
  {
    if (aOutput & INPUT_DISABLED_MASK)
    {
      lcd.printAt(aCol, LCD_ROW_BOT, M_DISABLED);
    }
    else
    {
      lcd.setCursor(aCol, LCD_ROW_BOT);
      lcd.print(HEX_CHARS[(aOutput >> OUTPUT_NODE_SHIFT) & OUTPUT_NODE_MASK]);
      lcd.print(HEX_CHARS[(aOutput                     ) & OUTPUT_PIN_MASK]);
    }
  }


  /** Display an Input's output settings, node and pin.
   */
  void displayInputEdit(int aIndex)
  {
    if (   (aIndex > 0)
        && (inputData.output[aIndex] & INPUT_DISABLED_MASK))
    {
      lcd.printAt(LCD_COL_NODE, LCD_ROW_BOT, CHAR_DOT);
      lcd.printAt(LCD_COL_PIN,  LCD_ROW_BOT, CHAR_DOT);
    }
    else
    {
      lcd.printAt(LCD_COL_NODE, LCD_ROW_BOT, HEX_CHARS[(inputData.output[aIndex] >> OUTPUT_NODE_SHIFT) & OUTPUT_NODE_MASK]);
      lcd.printAt(LCD_COL_PIN,  LCD_ROW_BOT, HEX_CHARS[(inputData.output[aIndex]                     ) & OUTPUT_PIN_MASK]);
    }
  }
  

  /** Display Output details.
   */
  void displayDetailOutput()
  {
    lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_OUTPUT_TYPES[outputData.type & OUTPUT_TYPE_MASK], LCD_LEN_OPTION);
    displayOutputParams(outputData.type & OUTPUT_TYPE_MASK);
  }


  /** Display Output's parameters depending on type.
   */
  void displayOutputParams(int aType)
  {
    int col = LCD_COL_OUTPUT_PARAM;
    
    lcd.clearRow(col, LCD_ROW_BOT);

    if (aType != OUTPUT_TYPE_LED)
    {
      lcd.printAtHex(col, LCD_ROW_BOT, outputData.lo,   2);
      col += LCD_COL_OUTPUT_STEP;
      lcd.printAtHex(col, LCD_ROW_BOT, outputData.hi,   2);
      col += LCD_COL_OUTPUT_STEP;
    }
    else
    {
      col = LCD_COL_OUTPUT_PARAM + 2 * LCD_COL_OUTPUT_STEP;
    }

    lcd.printAtHex(col, LCD_ROW_BOT, (outputData.pace >> OUTPUT_PACE_SHIFT) & OUTPUT_PACE_MASK,  1);
    col += LCD_COL_OUTPUT_STEP - 1;
    lcd.printAtHex(col, LCD_ROW_BOT, (outputData.pace                     ) & OUTPUT_DELAY_MASK, 1);
  }


  /** Display Output's angles with suitable prompts above.
   */
  void displayOutputAngles()
  {
    lcd.clearRow(LCD_COL_OUTPUT_PARAM, LCD_ROW_TOP);
    lcd.printAt(LCD_COL_OUTPUT_LO + OUTPUT_ANGLE_SIZE - sizeof(M_LO) + 1, LCD_ROW_TOP, M_LO);
    lcd.printAt(LCD_COL_OUTPUT_HI + OUTPUT_ANGLE_SIZE - sizeof(M_HI) + 1, LCD_ROW_TOP, M_HI);

    lcd.clearRow(LCD_COL_OUTPUT_PARAM, LCD_ROW_BOT);
    lcd.printAtDec(LCD_COL_OUTPUT_LO, LCD_ROW_BOT, outputData.lo, OUTPUT_ANGLE_SIZE);
    lcd.printAtDec(LCD_COL_OUTPUT_HI, LCD_ROW_BOT, outputData.hi, OUTPUT_ANGLE_SIZE);
  }


  /** Display Output's delay and pace parameters with suitable prompts.
   */
  void displayOutputDelayPace()
  {
    lcd.clearRow(LCD_COL_OUTPUT_PARAM, LCD_ROW_TOP);
    lcd.printAt(LCD_COL_OUTPUT_DELAY - 2, LCD_ROW_TOP, M_DELAY);
    lcd.printAt(LCD_COL_OUTPUT_PACE  - 1, LCD_ROW_TOP, M_PACE);

    lcd.clearRow(LCD_COL_OUTPUT_PARAM, LCD_ROW_BOT);
    lcd.printAt(LCD_COL_OUTPUT_PACE,  LCD_ROW_BOT, HEX_CHARS[(outputData.pace >> OUTPUT_PACE_SHIFT) & OUTPUT_PACE_MASK]);
    lcd.printAt(LCD_COL_OUTPUT_DELAY, LCD_ROW_BOT, HEX_CHARS[(outputData.pace                     ) & OUTPUT_DELAY_MASK]);
  }
  

  /** Process IO stage.
   */
  void menuTop()
  {
    boolean finished = false;

    // Initialise state.
    loadInput(node, pin);
    loadOutput(node, pin);
    
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
                            else
                            {
                              int8_t debugLevel = systemData.debugLevel;

                              // Disable debug whilst configuring.
                              systemData.debugLevel = 0;

                              switch (topMenu)
                              {
                                case TOP_INPUT:  menuNode(true);
                                                 break;
                                case TOP_OUTPUT: menuNode(false);
                                                 break;
                                case TOP_EXPORT: menuExport();
                                                 break;
                                case TOP_IMPORT: menuImport();
                                                 break;
                                default:         systemFail(M_CONFIG, topMenu);
                              }

                              // Re-establish debug.
                              systemData.debugLevel = debugLevel;
                            }
                            markField(LCD_COL_START, LCD_ROW_TOP, LCD_COL_MARK, true);
                            break;
      }
    }
    
    lcd.clear();
    waitForButtonRelease();
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
                            displayDetailSystem();
                            markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, true);
                            break;
        case BUTTON_SELECT: if (changed)
                            {
                              if (confirm())
                              {
                                saveSystemData();
                                lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_SAVED);
                                delay(DELAY_READ);
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
                                delay(DELAY_READ);
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
                            if (sysMenu == SYS_DEBUG)
                            {
                              changed = menuSystemDebug();
                            }
                            else if (sysMenu == SYS_I2C)
                            {
                              changed = menuSystemI2c();
                            }
                            else
                            {
                              systemFail(M_SYSTEM, sysMenu);
                            }

                            displaySystem();
                            markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, true);
                            break;
      }
    }

    markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, false);
  }


  /** Process System debug menu.
   *  Reurn true if changes made.
   */
  boolean menuSystemDebug()
  {
    boolean finished = false;
    boolean changed = false;
    int index = 0;

    markField(LCD_COL_DEBUG_PARAM, LCD_ROW_BOT, LCD_COL_DEBUG_LENGTH, true);

    while (!finished)
    {
      switch (waitForButton())
      {
        case BUTTON_NONE:   break;
        case BUTTON_UP:     systemData.debugLevel += 2;
                            if (systemData.debugLevel > DEBUG_MAX)
                            {
                              systemData.debugLevel = 1;
                            }
        case BUTTON_DOWN:   systemData.debugLevel -= 1;
                            if (systemData.debugLevel < 0)
                            {
                              systemData.debugLevel = DEBUG_MAX - 1;
                            }
                            lcd.printAt(LCD_COL_DEBUG_PARAM, LCD_ROW_BOT, M_DEBUG_PROMPTS[systemData.debugLevel], LCD_COL_DEBUG_LENGTH);
                            changed = true;
                            break;
        case BUTTON_SELECT: break;
        case BUTTON_LEFT:   finished = true;
                            break;
        case BUTTON_RIGHT:  break;
      }
    }

    markField(LCD_COL_DEBUG_PARAM, LCD_ROW_BOT, 5, false);
    
    return changed;
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

    return changed;
  }


  /** Process Export menu.
   */
  void menuExport()
  {
    boolean finished = false;
    
    markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, true);

    while (!finished)
    {
      switch (waitForButton())
      {
        case BUTTON_NONE:   break;
        case BUTTON_UP:     expMenu += 2;     // Use +1 to compensate for the -1 that the code below will do.
                            if (expMenu > EXP_MAX)
                            {
                              expMenu = 1;
                            }
        case BUTTON_DOWN:   expMenu -= 1;
                            if (expMenu < 0)
                            {
                              expMenu = EXP_MAX - 1;
                            }
                            displayDetailExport();
                            break;
        case BUTTON_SELECT: break;
        case BUTTON_LEFT:   finished = true;
                            break;
        case BUTTON_RIGHT:  markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, false);
                            importExport.doExport(expMenu);
                            markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, true);
                            break;
      }
    }

    markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, false);
    loadInput(node, pin);
    loadOutput(node, pin);
  }


  /** Process the Import menu (which doesn't exist).
   *  Just run the import.
   */
  void menuImport()
  {
    importExport.doImport();
  }


  /** Process Node menu for Input or Output.
   */
  void menuNode(boolean aIsInput)
  {
    boolean finished = false;

    markField(LCD_COL_NODE, LCD_ROW_TOP, 1, true);
    
    while (!finished)
    {
      int adjust = 0;
      
      switch (waitForButton())
      {
        case BUTTON_NONE:   break;
        case BUTTON_UP:     adjust += 2;     // Use +1 to compensate for the -1 that the code below will do.
        case BUTTON_DOWN:   adjust -= 1;
                            node = nextNode(node, adjust, aIsInput);
                            lcd.printAt(LCD_COL_NODE, LCD_ROW_TOP, HEX_CHARS[node]);
                            displayDetail();
                            break;
        case BUTTON_SELECT: break;
        case BUTTON_LEFT:   finished = true;
                            markField(LCD_COL_NODE, LCD_ROW_TOP, 1, false);
                            break;
        case BUTTON_RIGHT:  markField(LCD_COL_NODE, LCD_ROW_TOP, 1, false);
                            menuPin(aIsInput);
                            markField(LCD_COL_NODE, LCD_ROW_TOP, 1, true);
                            break;
      }
    }

    markField(LCD_COL_NODE, LCD_ROW_TOP, 1, false);
  }


  /** Find the next (live) node in the given direction.
   *  Load the node's data.
   */
  int nextNode(int aStart, int aAdjust, boolean aIsInput)
  {
    int next = aStart & (aIsInput ? INPUT_NODE_MASK : OUTPUT_NODE_MASK);
    
    for (int i = 0; i < (aIsInput ? INPUT_NODE_MAX : OUTPUT_NODE_MAX); i++)
    {
      next = (next + aAdjust) & (aIsInput ? INPUT_NODE_MASK : OUTPUT_NODE_MASK);
      if (   (aIsInput)
          && (isInputNode(next)))
      {
        loadInput(next, pin);
        break;
      }
      else if (   (!aIsInput)
               && (isOutputNode(next)))
      {
        loadOutput(next, pin);
        break;
      }
    }

    return next;
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
        case BUTTON_UP:     pin += 2;     // Use +1 to compensate for the -1 that the code below will do.
                            if (pin > (aIsInput ? INPUT_NODE_SIZE : OUTPUT_NODE_SIZE))
                            {
                              pin = 1;
                            }
        case BUTTON_DOWN:   pin -= 1;
                            if (pin < 0)
                            {
                              pin = aIsInput ? INPUT_NODE_SIZE - 1 : OUTPUT_NODE_SIZE - 1;
                            }
                            lcd.printAt(LCD_COL_PIN, LCD_ROW_TOP, HEX_CHARS[pin]);
                            if (aIsInput)
                            {
                              loadInput(node, pin);
                            }
                            else
                            {
                              loadOutput(node, pin);
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
    
    markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, true);
    
    while (!finished)
    {
      switch (waitForButton())
      {
        case BUTTON_NONE:   break;
        case BUTTON_UP:     inputType += 2;     // Use +1 to compensate for the -1 that the code below will do.
                            if (inputType > INPUT_TYPE_MAX)
                            {
                              inputType = 1;
                            }
        case BUTTON_DOWN:   inputType -= 1;
                            if (inputType < 0)
                            {
                              inputType = INPUT_TYPE_MAX - 1;
                            }
                            lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_INPUT_TYPES[inputType], LCD_LEN_OPTION);
                            changed = true;
                            break;
        case BUTTON_SELECT: if (changed)
                            {
                              if (confirm())
                              {
                                saveInput();
                                lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_SAVED);
                                delay(DELAY_READ);
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
                                loadInput(node, pin);
                                lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_CANCELLED);
                                delay(DELAY_READ);
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
                            changed |= menuInputSelect();
                            displayDetailInputOutput();
                            markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, true);
                            break;
      }
    }

    markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, false);

    // Ensure output node is reset (we may have corrupted it when chaninging an Input's Outputs).
    loadOutput(node, pin);
  }


  /** Select the Input Output to edit.
   */
  boolean menuInputSelect()
  {
    boolean finished = false;
    boolean changed  = false;

    int     index    = 0;

    lcd.clearRow(LCD_COL_INPUT_OUTPUT, LCD_ROW_BOT);
    lcd.printAt(LCD_COL_INPUT_OUTPUT, LCD_ROW_BOT, EDIT_CHARS[index]);
    displayInputEdit(index);
    markField(LCD_COL_INPUT_OUTPUT, LCD_ROW_BOT, 1, true);

    while (!finished)
    {
      switch (waitForButton())
      {
        case BUTTON_NONE:   break;
        case BUTTON_UP:     index += 2;     // Use +1 to compensate for the -1 that the code below will do.
                            if (index > INPUT_OUTPUT_MAX)
                            {
                              index = 1;
                            }
        case BUTTON_DOWN:   index -= 1;
                            if (index < 0)
                            {
                              index = INPUT_OUTPUT_MAX - 1;
                            }
                            lcd.printAt(LCD_COL_INPUT_OUTPUT, LCD_ROW_BOT, EDIT_CHARS[index]);
                            displayInputEdit(index);
                            break;
        case BUTTON_SELECT: if (index == 0)
                            {
                              testInput();
                            }
                            else
                            {
                              changed = true;
                              inputData.output[index] ^= INPUT_DISABLED_MASK;
                              displayInputEdit(index);
                            }
                            break;
        case BUTTON_LEFT:   finished = true;
                            break;
        case BUTTON_RIGHT:  markField(LCD_COL_INPUT_OUTPUT, LCD_ROW_BOT, 1, false);
                            changed |= menuInputOutputNode(index);
                            markField(LCD_COL_INPUT_OUTPUT, LCD_ROW_BOT, 1, true);
                            break;
      }
    }

    markField(LCD_COL_INPUT_OUTPUT, LCD_ROW_BOT, 1, false);
    
    return changed;
  }


  /** Process an Input's Output's node.
   */
  boolean menuInputOutputNode(int aIndex)
  {
    boolean changed  = false;
    boolean finished = false;

    markField(LCD_COL_NODE, LCD_ROW_BOT, 1, true);

    while (!finished)
    {
      switch (waitForButton())
      {
        case BUTTON_NONE:   break;
        case BUTTON_UP:     if (inputData.output[aIndex] & INPUT_DISABLED_MASK)
                            {
                              inputData.output[aIndex] ^= INPUT_DISABLED_MASK;
                            }
                            else
                            {
                              // Increment the node number within the Input's output at this index.
                              int next = (inputData.output[aIndex] >> OUTPUT_NODE_SHIFT) & OUTPUT_NODE_MASK;
                              next = nextNode(next, 1, false);
                              inputData.output[aIndex] = (inputData.output[aIndex] & ~ (OUTPUT_NODE_MASK << OUTPUT_NODE_SHIFT)) | ((next & OUTPUT_NODE_MASK) << OUTPUT_NODE_SHIFT);
                            }
                            
                            displayInputEdit(aIndex);
                            changed = true;
                            break;
        case BUTTON_DOWN:   if (inputData.output[aIndex] & INPUT_DISABLED_MASK)
                            {
                              inputData.output[aIndex] ^= INPUT_DISABLED_MASK;
                            }
                            else
                            {
                              // Decrement the node number within the Input's output at this index.
                              int next = (inputData.output[aIndex] >> OUTPUT_NODE_SHIFT) & OUTPUT_NODE_MASK;
                              next = nextNode(next, -1, false);
                              inputData.output[aIndex] = (inputData.output[aIndex] & ~ (OUTPUT_NODE_MASK << OUTPUT_NODE_SHIFT)) | ((next & OUTPUT_NODE_MASK) << OUTPUT_NODE_SHIFT);
                            }
                            displayInputEdit(aIndex);
                            changed = true;
                            break;
        case BUTTON_SELECT: if (aIndex == 0)
                            {
                              testInput();
                            }
                            else
                            {
                              // Enable/disable this output.
                              changed = true;
                              inputData.output[aIndex] ^= INPUT_DISABLED_MASK;
                              displayInputEdit(aIndex);
                            }
                            break;
        case BUTTON_LEFT:   finished = true;
                            break;
        case BUTTON_RIGHT:  markField(LCD_COL_NODE, LCD_ROW_BOT, 1, false);
                            changed |= menuInputOutputPin(aIndex);
                            markField(LCD_COL_NODE, LCD_ROW_BOT, 1, true);
                            break;
      }
    }

    markField(LCD_COL_NODE, LCD_ROW_BOT, 1, false);

    return changed;
  }


  boolean menuInputOutputPin(int aIndex)
  {
    boolean finished = false;
    boolean changed = false;

    markField(LCD_COL_PIN, LCD_ROW_BOT, 1, true);

    while (!finished)
    {
      switch (waitForButton())
      {
        case BUTTON_NONE:   break;
        case BUTTON_UP:     if (inputData.output[aIndex] & INPUT_DISABLED_MASK)
                            {
                              inputData.output[aIndex] ^= INPUT_DISABLED_MASK;
                            }
                            else
                            {
                              // Increment the pin number within the Input's output at this index.
                              inputData.output[aIndex] = (inputData.output[aIndex] & ~ OUTPUT_PIN_MASK) | ((inputData.output[aIndex] + 1) & OUTPUT_PIN_MASK);
                            }
                            displayInputEdit(aIndex);
                            changed = true;
                            break;
        case BUTTON_DOWN:   if (inputData.output[aIndex] & INPUT_DISABLED_MASK)
                            {
                              inputData.output[aIndex] ^= INPUT_DISABLED_MASK;
                            }
                            else
                            {
                              // Decrement the pin number within the Input's output at this index.
                              inputData.output[aIndex] = (inputData.output[aIndex] & ~ OUTPUT_PIN_MASK) | ((inputData.output[aIndex] - 1) & OUTPUT_PIN_MASK);
                            }
                            displayInputEdit(aIndex);
                            changed = true;
                            break;
        case BUTTON_SELECT: if (aIndex == 0)
                            {
                              testInput();
                            }
                            else
                            {
                              changed = true;
                              inputData.output[aIndex] ^= INPUT_DISABLED_MASK;
                              displayInputEdit(aIndex);
                            }
                            break;
        case BUTTON_LEFT:   finished = true;
                            break;
        case BUTTON_RIGHT:  break;
      }
    }

    markField(LCD_COL_PIN, LCD_ROW_BOT, 1, false);
    
    return finished;
  }


  /** Operate the current input to test if it works.
   */
  void testInput()
  {
    uint8_t currentState = 0;
    
    loadOutput(inputData.output[0] & INPUT_OUTPUT_MASK);
    currentState = outputData.type & OUTPUT_STATE;

    processInputOutputs(currentState ? 0 : OUTPUT_STATE);
    waitForButtonRelease();
    processInputOutputs(currentState);
  }


  /** Process Output menu.
   */
  void menuOutput()
  {
    boolean finished = false;
    boolean changed  = false;
    
    // Retrieve type
    int outputType = outputData.type & OUTPUT_TYPE_MASK;

    // Mark the field.
    markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, true);

    while (!finished)
    {
      switch (waitForButton())
      {
        case BUTTON_NONE:   break;
        case BUTTON_UP:     outputType += 2;     // Use +1 to compensate for the -1 that the code below will do.
                            if (outputType > OUTPUT_TYPE_MAX)
                            {
                              outputType = 1;
                            }
        case BUTTON_DOWN:   outputType -= 1;
                            if (outputType < 0)
                            {
                              outputType = OUTPUT_TYPE_MAX - 1;
                            }
                            lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_OUTPUT_TYPES[outputType], LCD_LEN_OPTION);
                            displayOutputParams(outputType);
                            markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, true);
                            changed = true;
                            break;
        case BUTTON_SELECT: if (changed)
                            {
                              if (confirm())
                              {
                                outputData.type = outputType | (outputData.type & ~OUTPUT_TYPE_MASK);
                                saveOutput();
                                lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_SAVED);
                                delay(DELAY_READ);
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
                                loadOutput(node, pin);
                                lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_CANCELLED);
                                delay(DELAY_READ);
                                displayDetailOutput();
                                finished = true;
                              }
                              else
                              {
                                outputData.type = outputType | (outputData.type & ~OUTPUT_TYPE_MASK);
                                displayDetailOutput();
                                markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, true);
                              }
                            }
                            else
                            {
                              finished = true;
                            }
                            break;
        case BUTTON_RIGHT:  markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, false);
                            if (   (outputType == OUTPUT_TYPE_SERVO)
                                || (outputType == OUTPUT_TYPE_SIGNAL))
                            {
                              changed |= menuOutputLo();
                            }
                            else if (outputType == OUTPUT_TYPE_LED)
                            {
                              changed |= menuOutputPace();
                            }
                            else
                            {
                              systemFail(M_OUTPUT, outputType);
                            }

                            displayNode();
                            displayOutputParams(outputData.type & OUTPUT_TYPE_MASK);
                            markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, true);
                            break;
      }
    }
    
    markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, false);
  }


  /** Process Output's Lo parameter (0-180) menu.
   */
  boolean menuOutputLo()
  {
    boolean finished = false;
    boolean changed  = false;

    displayOutputAngles();
    markField(LCD_COL_OUTPUT_LO, LCD_ROW_BOT, OUTPUT_ANGLE_SIZE, true);
    sendOutputCommand(outputData.lo, outputData.pace & ~ OUTPUT_DELAY_MASK, 0, 0);

    while (!finished)
    {
      int autoRepeat = DELAY_BUTTON_DELAY;
      switch (waitForButton())
      {
        case BUTTON_NONE:   break;
        case BUTTON_UP:     do
                            {
                              outputData.lo += 1;
                              if (outputData.lo > OUTPUT_ANGLE_MAX)
                              {
                                outputData.lo = 0;
                              }
                              lcd.printAtDec(LCD_COL_OUTPUT_LO, LCD_ROW_BOT, outputData.lo, OUTPUT_ANGLE_SIZE);
                              delay(autoRepeat);
                              autoRepeat = DELAY_BUTTON_REPEAT;
                            }
                            while (readButton() != 0);
                            sendOutputCommand(outputData.lo, outputData.pace & ~ OUTPUT_DELAY_MASK, 0, 0);
                            changed = true;
                            break;
        case BUTTON_DOWN:   do
                            {
                              outputData.lo -= 1;
                              if (outputData.lo > OUTPUT_ANGLE_MAX)
                              {
                                outputData.lo = OUTPUT_ANGLE_MAX;
                              }
                              lcd.printAtDec(LCD_COL_OUTPUT_LO, LCD_ROW_BOT, outputData.lo, OUTPUT_ANGLE_SIZE);
                              delay(autoRepeat);
                              autoRepeat = DELAY_BUTTON_REPEAT;
                            }
                            while (readButton() != 0);
                            sendOutputCommand(outputData.lo, outputData.pace & ~ OUTPUT_DELAY_MASK, 0, 0);
                            changed = true;
                            break;
        case BUTTON_SELECT: testOutput(false, true);
                            break;
        case BUTTON_LEFT:   finished = true;
                            break;
        case BUTTON_RIGHT:  markField(LCD_COL_OUTPUT_LO, LCD_ROW_BOT, OUTPUT_ANGLE_SIZE, false);
                            changed |= menuOutputHi();
                            markField(LCD_COL_OUTPUT_LO, LCD_ROW_BOT, OUTPUT_ANGLE_SIZE, true);
                            sendOutputCommand(outputData.lo, outputData.pace & ~ OUTPUT_DELAY_MASK, 0, 0);
                            break;
      }
    }

    markField(LCD_COL_OUTPUT_LO, LCD_ROW_BOT, OUTPUT_ANGLE_SIZE, false);
    sendOutputCommand(outputData.type & OUTPUT_STATE ? outputData.hi : outputData.lo, outputData.pace & ~ OUTPUT_DELAY_MASK, 0, outputData.type & OUTPUT_STATE);
    
    return changed;
  }


  /** Process the Output Hi parameter.
   */
  boolean menuOutputHi()
  {
    boolean finished = false;
    boolean changed  = false;

    markField(LCD_COL_OUTPUT_HI, LCD_ROW_BOT, OUTPUT_ANGLE_SIZE, true);
    sendOutputCommand(outputData.hi, outputData.pace & ~ OUTPUT_DELAY_MASK, 0, OUTPUT_STATE);

    while (!finished)
    {
      int autoRepeat = DELAY_BUTTON_DELAY;
      switch (waitForButton())
      {
        case BUTTON_NONE:   break;
        case BUTTON_UP:     do
                            {
                              outputData.hi += 1;
                              if (outputData.hi > OUTPUT_ANGLE_MAX)
                              {
                                outputData.hi = 0;
                              }
                              lcd.printAtDec(LCD_COL_OUTPUT_HI, LCD_ROW_BOT, outputData.hi, OUTPUT_ANGLE_SIZE);
                              delay(autoRepeat);
                              autoRepeat = DELAY_BUTTON_REPEAT;
                            }
                            while (readButton() != 0);
                            sendOutputCommand(outputData.hi, outputData.pace & ~ OUTPUT_DELAY_MASK, 0, OUTPUT_STATE);
                            changed = true;
                            break;
        case BUTTON_DOWN:   do
                            {
                              outputData.hi -= 1;
                              if (outputData.hi > OUTPUT_ANGLE_MAX)
                              {
                                outputData.hi = OUTPUT_ANGLE_MAX;
                              }
                              lcd.printAtDec(LCD_COL_OUTPUT_HI, LCD_ROW_BOT, outputData.hi, OUTPUT_ANGLE_SIZE);
                              delay(autoRepeat);
                              autoRepeat = DELAY_BUTTON_REPEAT;
                            }
                            while (readButton() != 0);
                            sendOutputCommand(outputData.hi, outputData.pace & ~ OUTPUT_DELAY_MASK, 0, OUTPUT_STATE);
                            changed = true;
                            break;
        case BUTTON_SELECT: testOutput(false, false);
                            break;
        case BUTTON_LEFT:   finished = true;
                            break;
        case BUTTON_RIGHT:  markField(LCD_COL_OUTPUT_HI, LCD_ROW_BOT, OUTPUT_ANGLE_SIZE, false);
                            changed |= menuOutputPace();
                            displayOutputAngles();
                            markField(LCD_COL_OUTPUT_HI, LCD_ROW_BOT, OUTPUT_ANGLE_SIZE, true);
                            break;
      }
    }

    markField(LCD_COL_OUTPUT_HI, LCD_ROW_BOT, OUTPUT_ANGLE_SIZE, false);
        
    return changed;
  }


  /** Process the Output's Pace parameter.
   */
  boolean menuOutputPace()
  {
    boolean finished = false;
    boolean changed  = false;
    int value = (outputData.pace >> OUTPUT_PACE_SHIFT) & OUTPUT_PACE_MASK;

    displayOutputDelayPace();
    markField(LCD_COL_OUTPUT_PACE, LCD_ROW_BOT, 1, true);

    while (!finished)
    {
      switch (waitForButton())
      {
        case BUTTON_NONE:   break;
        case BUTTON_UP:     value = value + 1 & OUTPUT_PACE_MASK;
                            outputData.pace = (outputData.pace & OUTPUT_DELAY_MASK) | ((value & OUTPUT_PACE_MASK) << OUTPUT_PACE_SHIFT);
                            lcd.printAt(LCD_COL_OUTPUT_PACE, LCD_ROW_BOT, HEX_CHARS[value]);
                            changed = true;
                            break;
        case BUTTON_DOWN:   value = value - 1 & OUTPUT_PACE_MASK;
                            outputData.pace = (outputData.pace & OUTPUT_DELAY_MASK) | ((value & OUTPUT_PACE_MASK) << OUTPUT_PACE_SHIFT);
                            lcd.printAt(LCD_COL_OUTPUT_PACE, LCD_ROW_BOT, HEX_CHARS[value]);
                            changed = true;
                            break;
        case BUTTON_SELECT: testOutput(false, false);
                            break;
        case BUTTON_LEFT:   finished = true;
                            break;
        case BUTTON_RIGHT:  markField(LCD_COL_OUTPUT_PACE, LCD_ROW_BOT, 1, false);
                            changed |= menuOutputDelay();
                            markField(LCD_COL_OUTPUT_PACE, LCD_ROW_BOT, 1, true);
                            break;
      }
    }

    markField(LCD_COL_OUTPUT_PACE, LCD_ROW_BOT, 1, false);

    return changed;
  }


  /** Process the Output's Pace parameter.
   */
  boolean menuOutputDelay()
  {
    boolean finished = false;
    boolean changed  = false;
    int value = outputData.pace & OUTPUT_DELAY_MASK;

    markField(LCD_COL_OUTPUT_DELAY, LCD_ROW_BOT, 1, true);

    while (!finished)
    {
      switch (waitForButton())
      {
        case BUTTON_NONE:   break;
        case BUTTON_UP:     value = value + 1 & OUTPUT_DELAY_MASK;
                            outputData.pace = (outputData.pace & ~ OUTPUT_DELAY_MASK) | value & OUTPUT_DELAY_MASK;
                            lcd.printAt(LCD_COL_OUTPUT_DELAY, LCD_ROW_BOT, HEX_CHARS[value]);
                            changed = true;
                            break;
        case BUTTON_DOWN:   value = value - 1 & OUTPUT_DELAY_MASK;
                            outputData.pace = (outputData.pace & ~ OUTPUT_DELAY_MASK) | value & OUTPUT_DELAY_MASK;
                            lcd.printAt(LCD_COL_OUTPUT_DELAY, LCD_ROW_BOT, HEX_CHARS[value]);
                            changed = true;
                            break;
        case BUTTON_SELECT: testOutput(true, false);
                            break;
        case BUTTON_LEFT:   finished = true;
                            break;
        case BUTTON_RIGHT:  break;
      }
    }

    markField(LCD_COL_OUTPUT_DELAY, LCD_ROW_BOT, 1, false);

    return changed;
  }


  /** Test the current Configuration.
   */
  void testOutput(boolean aIncludeDelay, boolean aDirectionHi)
  {
    sendOutputCommand(aDirectionHi ? outputData.hi : outputData.lo, outputData.pace, aIncludeDelay ? outputData.pace & OUTPUT_DELAY_MASK : 0, OUTPUT_STATE);
    waitForButtonRelease();
    sendOutputCommand(aDirectionHi ? outputData.lo : outputData.hi, outputData.pace, aIncludeDelay ? outputData.pace & OUTPUT_DELAY_MASK : 0, 0);
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
    lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_CONFIRM);

    return waitForButton() == BUTTON_SELECT;
  }

  
  /** Output cancellation message.
   */
  boolean cancel()
  {
    lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_CANCEL);

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
    lcd.printAt(LCD_COL_START, LCD_ROW_TOP, M_CONFIG);
    waitForButtonRelease();

    menuTop();
    
    lcd.clear();
  }
};


/** A singleton instance of the class.
 */
Configure configure;

#endif
