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
#define SYS_REPORT   0
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
            default:         systemFail(M_ALL, topMenu, 0);
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
            default:         systemFail(M_DETAIL, topMenu, 0);
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
            case SYS_REPORT: displaySystemReportParams();
                             break;
            case SYS_I2C:    displaySystemI2cParams();
                             break;
            default:         systemFail(M_PARAMS, topMenu, 0);
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


    /** Display System's report parameter.
     */
    void displaySystemReportParams()
    {
        lcd.clearRow(LCD_COL_MARK, LCD_ROW_BOT);
        lcd.printAt(LCD_COL_REPORT_PARAM, LCD_ROW_BOT, M_REPORT_PROMPTS[systemData.reportLevel], LCD_LEN_OPTION);    
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
        for (int index = 0; index < INPUT_OUTPUT_MAX; index++, col += LCD_COL_INPUT_STEP)
        {
            if (inputDef.isDisabled(index))
            {
                lcd.printAt(col, LCD_ROW_BOT, M_DISABLED);    
            }
            else
            {
                lcd.setCursor(col, LCD_ROW_BOT);
                lcd.print(HEX_CHARS[inputDef.getOutputNode(index)]);
                lcd.print(HEX_CHARS[inputDef.getOutputPin(index)]);
            }
        }
    }
    

    /** Display an Input's output settings, node and pin.
     */
    void displayInputEdit(int aIndex)
    {
        if (inputDef.isDisabled(aIndex))
        {
            lcd.printAt(LCD_COL_NODE, LCD_ROW_BOT, CHAR_DOT);
            lcd.printAt(LCD_COL_PIN,  LCD_ROW_BOT, CHAR_DOT);
        }
        else
        {
            lcd.printAt(LCD_COL_NODE, LCD_ROW_BOT, HEX_CHARS[inputDef.getOutputNode(aIndex)]);
            lcd.printAt(LCD_COL_PIN,  LCD_ROW_BOT, HEX_CHARS[inputDef.getOutputPin(aIndex)]);
        }
    }
    

    /** Display Output details.
     */
    void displayDetailOutput()
    {
        lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_OUTPUT_TYPES[outputDef.getType()], LCD_LEN_OPTION);
        displayOutputParams(outputDef.getType());
    }


    /** Display Output's parameters depending on type.
     */
    void displayOutputParams(int aType)
    {
        int col = LCD_COL_OUTPUT_PARAM;
        
        lcd.clearRow(col, LCD_ROW_BOT);
        lcd.printAtHex(col, LCD_ROW_BOT, outputDef.getLo(),    2);
        col += LCD_COL_OUTPUT_STEP;
        lcd.printAtHex(col, LCD_ROW_BOT, outputDef.getHi(),    2);
        col += LCD_COL_OUTPUT_STEP;
        lcd.printAtHex(col, LCD_ROW_BOT, outputDef.getPace(),  1);
        col += LCD_COL_OUTPUT_STEP - 1;
        lcd.printAtHex(col, LCD_ROW_BOT, outputDef.getDelay(), 1);
    }


    /** Display Output's angles with suitable prompts above.
     */
    void displayOutputAngles()
    {
        lcd.clearRow(LCD_COL_OUTPUT_PARAM, LCD_ROW_TOP);
        lcd.printAt(LCD_COL_OUTPUT_LO + OUTPUT_HI_LO_SIZE - sizeof(M_LO) + 1, LCD_ROW_TOP, M_LO);
        lcd.printAt(LCD_COL_OUTPUT_HI + OUTPUT_HI_LO_SIZE - sizeof(M_HI) + 1, LCD_ROW_TOP, M_HI);

        lcd.clearRow(LCD_COL_OUTPUT_PARAM, LCD_ROW_BOT);
        lcd.printAtDec(LCD_COL_OUTPUT_LO, LCD_ROW_BOT, outputDef.getLo(), OUTPUT_HI_LO_SIZE);
        lcd.printAtDec(LCD_COL_OUTPUT_HI, LCD_ROW_BOT, outputDef.getHi(), OUTPUT_HI_LO_SIZE);
    }


    /** Display Output's delay and pace parameters with suitable prompts.
     */
    void displayOutputDelayPace()
    {
        lcd.clearRow(LCD_COL_OUTPUT_PARAM, LCD_ROW_TOP);
        lcd.printAt(LCD_COL_OUTPUT_DELAY - 2, LCD_ROW_TOP, M_DELAY);
        lcd.printAt(LCD_COL_OUTPUT_PACE  - 1, LCD_ROW_TOP, M_PACE);

        lcd.clearRow(LCD_COL_OUTPUT_PARAM, LCD_ROW_BOT);
        lcd.printAt(LCD_COL_OUTPUT_PACE,  LCD_ROW_BOT, HEX_CHARS[outputDef.getPace()]);
        lcd.printAt(LCD_COL_OUTPUT_DELAY, LCD_ROW_BOT, HEX_CHARS[outputDef.getDelay()]);
    }
    

    /** Process IO stage.
     */
    void menuTop()
    {
        boolean finished = false;

        // Initialise state.
        loadInput(node, pin);
        readOutput(node, pin);
        
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
                                        int8_t reportLevel = systemData.reportLevel;

                                        // Disable reporting whilst configuring.
                                        systemData.reportLevel = 0;

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
                                            default:         systemFail(M_CONFIG, topMenu, 0);
                                        }

                                        // Re-establish reporting.
                                        systemData.reportLevel = reportLevel;
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
                                    if (sysMenu == SYS_REPORT)
                                    {
                                        changed = menuSystemReport();
                                    }
                                    else if (sysMenu == SYS_I2C)
                                    {
                                        changed = menuSystemI2c();
                                    }
                                    else
                                    {
                                        systemFail(M_SYSTEM, sysMenu, 0);
                                    }

                                    displaySystem();
                                    markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, true);
                                    break;
            }
        }

        markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, false);
    }


    /** Process System report menu.
     *  Reurn true if changes made.
     */
    boolean menuSystemReport()
    {
        boolean finished = false;
        boolean changed = false;
        int index = 0;

        markField(LCD_COL_REPORT_PARAM, LCD_ROW_BOT, LCD_COL_REPORT_LENGTH, true);

        while (!finished)
        {
            switch (waitForButton())
            {
                case BUTTON_NONE:   break;
                case BUTTON_UP:     systemData.reportLevel += 2;     // Allow for decrement in BUTTON_DOWN code below.
                                    if (systemData.reportLevel > REPORT_MAX)
                                    {
                                        systemData.reportLevel = 1;
                                    }
                case BUTTON_DOWN:   systemData.reportLevel -= 1;
                                    if (systemData.reportLevel < 0)
                                    {
                                        systemData.reportLevel = REPORT_MAX - 1;
                                    }
                                    lcd.printAt(LCD_COL_REPORT_PARAM, LCD_ROW_BOT, M_REPORT_PROMPTS[systemData.reportLevel % REPORT_MAX], LCD_COL_REPORT_LENGTH);
                                    changed = true;
                                    break;
                case BUTTON_SELECT: break;
                case BUTTON_LEFT:   finished = true;
                                    break;
                case BUTTON_RIGHT:  break;
            }
        }

        markField(LCD_COL_REPORT_PARAM, LCD_ROW_BOT, 5, false);
        
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

        // Update systemData if changes have been made.
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
        readOutput(node, pin);
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
    uint8_t nextNode(uint8_t aStart, int aAdjust, boolean aIsInput)
    {
        uint8_t next = aStart & (aIsInput ? INPUT_NODE_MASK : OUTPUT_NODE_MASK);
        
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
                readOutput(next, pin);
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
                                    if (pin > (aIsInput ? INPUT_NODE_SIZE : OUTPUT_PIN_MAX))
                                    {
                                        pin = 1;
                                    }
                case BUTTON_DOWN:   pin -= 1;
                                    if (pin < 0)
                                    {
                                        pin = aIsInput ? INPUT_NODE_SIZE - 1 : OUTPUT_PIN_MAX - 1;
                                    }
                                    lcd.printAt(LCD_COL_PIN, LCD_ROW_TOP, HEX_CHARS[pin]);
                                    if (aIsInput)
                                    {
                                        loadInput(node, pin);
                                    }
                                    else
                                    {
                                        readOutput(node, pin);
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
        readOutput(node, pin);
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
                                        inputDef.setDisabled(index, !inputDef.isDisabled(index));
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
                case BUTTON_UP:     if (inputDef.isDisabled(aIndex))
                                    {
                                        inputDef.setDisabled(aIndex, false);
                                    }
                                    else
                                    {
                                        // Increment the node number (to the next available) within the Input's output at this index.
                                        inputDef.setOutputNode(aIndex, nextNode(inputDef.getOutputNode(aIndex), 1, false));
                                    }
                                    
                                    displayInputEdit(aIndex);
                                    changed = true;
                                    break;
                case BUTTON_DOWN:   if (inputDef.isDisabled(aIndex))
                                    {
                                        inputDef.setDisabled(aIndex, false);
                                    }
                                    else
                                    {
                                        // Decrement the node number (to the next available) within the Input's output at this index.
                                        inputDef.setOutputNode(aIndex, nextNode(inputDef.getOutputNode(aIndex), -1, false));
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
                                        inputDef.setDisabled(aIndex, !inputDef.isDisabled(aIndex));
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
                case BUTTON_UP:     if (inputDef.isDisabled(aIndex))
                                    {
                                        inputDef.setDisabled(aIndex, false);
                                    }
                                    else
                                    {
                                        // Increment the pin number within the Input's output at this index.
                                        inputDef.setOutputPin(aIndex, inputDef.getOutputPin(aIndex) + 1);
                                    }
                                    displayInputEdit(aIndex);
                                    changed = true;
                                    break;
                case BUTTON_DOWN:   if (inputDef.isDisabled(aIndex))
                                    {
                                        inputDef.setDisabled(aIndex, false);
                                    }
                                    else
                                    {
                                        // Decrement the pin number within the Input's output at this index.
                                        inputDef.setOutputPin(aIndex, inputDef.getOutputPin(aIndex) - 1);
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
                                        inputDef.setDisabled(aIndex, !inputDef.isDisabled(aIndex));
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
        boolean currentState = false;
        
        readOutput(inputDef.getOutput(0));
        currentState = outputDef.getState();

        processInputOutputs(!currentState);
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
        int outputType = outputDef.getType();

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
                                    outputDef.setType(outputType);
                                    
                                    lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_OUTPUT_TYPES[outputType], LCD_LEN_OPTION);
                                    displayOutputParams(outputType);
                                    markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, true);
                                    changed = true;
                                    break;
                case BUTTON_SELECT: if (changed)
                                    {
                                        if (confirm())
                                        {
                                            writeOutput();
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
                                            readOutput(node, pin);
                                            sendOutputCommand(outputDef.getState() ? outputDef.getHi() : outputDef.getLo(), outputDef.getPace(), 0, outputDef.getState());
                                            lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_CANCELLED);
                                            delay(DELAY_READ);
                                            displayDetailOutput();
                                            finished = true;
                                        }
                                        else
                                        {
                                            outputDef.setType(outputType);
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
                                    switch (outputType)
                                    {
                                        case OUTPUT_TYPE_SERVO:
                                        case OUTPUT_TYPE_SIGNAL: changed |= menuOutputLo(OUTPUT_SERVO_MAX);
                                                                 break;
                                        case OUTPUT_TYPE_LED:
                                        case OUTPUT_TYPE_FLASH:
                                        case OUTPUT_TYPE_BLINK:  changed |= menuOutputLo(OUTPUT_LED_MAX);
                                                                 // sendOutputCommand(outputDef.getHi(),                                                  outputDef.getPace(), 0, OUTPUT_STATE_MASK);
                                                                 // changed |= menuOutputPace();
                                                                 // sendOutputCommand(outputDef.type & OUTPUT_STATE_MASK ? outputDef.getHi() : outputDef.getLo(), outputDef.getPace(), 0, outputDef.type & OUTPUT_STATE_MASK);
                                                                 break;
                                        default:                 systemFail(M_OUTPUT, outputType, 0);
                                    }

                                    displayNode();
                                    displayOutputParams(outputDef.getType());
                                    markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, true);
                                    break;
            }
        }
        
        markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, false);
    }


    /** Process Output's Lo parameter (0-180) menu.
     */
    boolean menuOutputLo(int aLimit)
    {
        boolean finished = false;
        boolean changed  = false;

        displayOutputAngles();
        markField(LCD_COL_OUTPUT_LO, LCD_ROW_BOT, OUTPUT_HI_LO_SIZE, true);
        sendOutputCommand(outputDef.getLo(), outputDef.getPace(), 0, 0);

        while (!finished)
        {
            int autoRepeat = DELAY_BUTTON_DELAY;
            switch (waitForButton())
            {
                case BUTTON_NONE:   break;
                case BUTTON_UP:     do
                                    {
                                        outputDef.setLo(outputDef.getLo() + 1);
                                        if (outputDef.getLo() > aLimit)
                                        {
                                            outputDef.setLo(0);
                                        }
                                        lcd.printAtDec(LCD_COL_OUTPUT_LO, LCD_ROW_BOT, outputDef.getLo(), OUTPUT_HI_LO_SIZE);
                                        delay(autoRepeat);
                                        autoRepeat = DELAY_BUTTON_REPEAT;
                                    }
                                    while (readButton() != 0);
                                    sendOutputCommand(outputDef.getLo(), outputDef.getPace(), 0, 0);
                                    changed = true;
                                    break;
                case BUTTON_DOWN:   do
                                    {
                                        outputDef.setLo(outputDef.getLo() - 1);
                                        if (outputDef.getLo() > aLimit)
                                        {
                                            outputDef.setLo(aLimit);
                                        }
                                        lcd.printAtDec(LCD_COL_OUTPUT_LO, LCD_ROW_BOT, outputDef.getLo(), OUTPUT_HI_LO_SIZE);
                                        delay(autoRepeat);
                                        autoRepeat = DELAY_BUTTON_REPEAT;
                                    }
                                    while (readButton() != 0);
                                    sendOutputCommand(outputDef.getLo(), outputDef.getPace(), 0, 0);
                                    changed = true;
                                    break;
                case BUTTON_SELECT: testOutput(false, true);
                                    break;
                case BUTTON_LEFT:   finished = true;
                                    break;
                case BUTTON_RIGHT:  markField(LCD_COL_OUTPUT_LO, LCD_ROW_BOT, OUTPUT_HI_LO_SIZE, false);
                                    changed |= menuOutputHi(aLimit);
                                    markField(LCD_COL_OUTPUT_LO, LCD_ROW_BOT, OUTPUT_HI_LO_SIZE, true);
                                    sendOutputCommand(outputDef.getLo(), outputDef.getPace(), 0, 0);
                                    break;
            }
        }

        markField(LCD_COL_OUTPUT_LO, LCD_ROW_BOT, OUTPUT_HI_LO_SIZE, false);
        sendOutputCommand(outputDef.getType() ? outputDef.getHi() : outputDef.getLo(), outputDef.getPace(), 0, outputDef.getType());
        
        return changed;
    }


    /** Process the Output Hi parameter.
     */
    boolean menuOutputHi(int aLimit)
    {
        boolean finished = false;
        boolean changed  = false;

        markField(LCD_COL_OUTPUT_HI, LCD_ROW_BOT, OUTPUT_HI_LO_SIZE, true);
        sendOutputCommand(outputDef.getHi(), outputDef.getPace(), 0, OUTPUT_STATE_MASK);

        while (!finished)
        {
            int autoRepeat = DELAY_BUTTON_DELAY;
            switch (waitForButton())
            {
                case BUTTON_NONE:   break;
                case BUTTON_UP:     do
                                    {
                                        outputDef.setHi(outputDef.getHi() + 1);
                                        if (outputDef.getHi() > aLimit)
                                        {
                                            outputDef.setHi(0);
                                        }
                                        lcd.printAtDec(LCD_COL_OUTPUT_HI, LCD_ROW_BOT, outputDef.getHi(), OUTPUT_HI_LO_SIZE);
                                        delay(autoRepeat);
                                        autoRepeat = DELAY_BUTTON_REPEAT;
                                    }
                                    while (readButton() != 0);
                                    sendOutputCommand(outputDef.getHi(), outputDef.getPace(), 0, OUTPUT_STATE_MASK);
                                    changed = true;
                                    break;
                case BUTTON_DOWN:   do
                                    {
                                        outputDef.setHi(outputDef.getHi() - 1);
                                        if (outputDef.getHi() > aLimit)
                                        {
                                            outputDef.setHi(aLimit);
                                        }
                                        lcd.printAtDec(LCD_COL_OUTPUT_HI, LCD_ROW_BOT, outputDef.getHi(), OUTPUT_HI_LO_SIZE);
                                        delay(autoRepeat);
                                        autoRepeat = DELAY_BUTTON_REPEAT;
                                    }
                                    while (readButton() != 0);
                                    sendOutputCommand(outputDef.getHi(), outputDef.getPace(), 0, OUTPUT_STATE_MASK);
                                    changed = true;
                                    break;
                case BUTTON_SELECT: testOutput(false, false);
                                    break;
                case BUTTON_LEFT:   finished = true;
                                    break;
                case BUTTON_RIGHT:  markField(LCD_COL_OUTPUT_HI, LCD_ROW_BOT, OUTPUT_HI_LO_SIZE, false);
                                    changed |= menuOutputPace();
                                    displayOutputAngles();
                                    markField(LCD_COL_OUTPUT_HI, LCD_ROW_BOT, OUTPUT_HI_LO_SIZE, true);
                                    break;
            }
        }

        markField(LCD_COL_OUTPUT_HI, LCD_ROW_BOT, OUTPUT_HI_LO_SIZE, false);
                
        return changed;
    }


    /** Process the Output's Pace parameter.
     */
    boolean menuOutputPace()
    {
        boolean finished = false;
        boolean changed  = false;
        uint8_t value    = outputDef.getPace();

        displayOutputDelayPace();
        markField(LCD_COL_OUTPUT_PACE, LCD_ROW_BOT, 1, true);

        while (!finished)
        {
            switch (waitForButton())
            {
                case BUTTON_NONE:   break;
                case BUTTON_UP:     value = (value + 1) & OUTPUT_PACE_MASK;
                                    outputDef.setPace(value);
                                    lcd.printAt(LCD_COL_OUTPUT_PACE, LCD_ROW_BOT, HEX_CHARS[value]);
                                    changed = true;
                                    break;
                case BUTTON_DOWN:   value = (value - 1) & OUTPUT_PACE_MASK;
                                    outputDef.setPace(value);
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
        uint8_t value    = outputDef.getDelay();

        markField(LCD_COL_OUTPUT_DELAY, LCD_ROW_BOT, 1, true);

        while (!finished)
        {
            switch (waitForButton())
            {
                case BUTTON_NONE:   break;
                case BUTTON_UP:     value = (value + 1) & OUTPUT_DELAY_MASK;
                                    outputDef.setDelay(value);
                                    lcd.printAt(LCD_COL_OUTPUT_DELAY, LCD_ROW_BOT, HEX_CHARS[value]);
                                    changed = true;
                                    break;
                case BUTTON_DOWN:   value = (value - 1) & OUTPUT_DELAY_MASK;
                                    outputDef.setDelay(value);
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
        sendOutputCommand(aDirectionHi ? outputDef.getHi() : outputDef.getLo(), outputDef.getPace(), aIncludeDelay ? outputDef.getDelay() : 0, aDirectionHi ? OUTPUT_STATE_MASK : 0);
        waitForButtonRelease();
        sendOutputCommand(aDirectionHi ? outputDef.getLo() : outputDef.getHi(), outputDef.getPace(), aIncludeDelay ? outputDef.getDelay() : 0, aDirectionHi ? 0 : OUTPUT_STATE_MASK);
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


    /** Send a command to an output node.
     *  Return error code if any.
     */
    int sendOutputCommand(uint8_t aValue, uint8_t aPace, uint8_t aDelay, uint8_t aState)
    {
        #if DEBUG
            Serial.print(millis());
            Serial.print("\tOutput ");
            Serial.print(PGMT(M_OUTPUT_TYPES[outputDef.getType()]));
            Serial.print(CHAR_SPACE);
            Serial.print(HEX_CHARS[outputNode]);
            Serial.print(HEX_CHARS[outputPin]);
            Serial.print(CHAR_SPACE);
            Serial.print(PGMT(aState ? M_HI : M_LO));
            Serial.print(", value=");
            Serial.print(aValue, HEX);
            Serial.print(", pace=");
            Serial.print(aPace, HEX);
            Serial.println();
        #endif
    
//        // TODO - send output command
//        Wire.beginTransmission(systemData.i2cOutputBaseID + outputNode);
//        Wire.write(COMMS_CMD_SET + outputPin);
//        Wire.write(outputDef.getType());
//        Wire.write(aValue);
//        Wire.write(aPace);
//        Wire.write(aState);
//        if (aDelay)
//        {
//            Wire.write(aDelay);
//        }
//        return Wire.endTransmission();
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
