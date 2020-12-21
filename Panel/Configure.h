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
#define SYS_NODES    2
#define SYS_DEBUG    3
#define SYS_MAX      4


/** Configure the system.
 */
class Configure
{
    private:

    uint8_t topMenu = 0;    // Top menu being shown
    uint8_t sysMenu = 0;    // System menu being shown.
    uint8_t expMenu = 0;    // Export menu being shown.
    uint8_t outNode = 0;    // The output node we last configured.
    uint8_t outPin  = 0;    // The output pin we last configured.
    uint8_t inpNode = 0;    // The input node we last configured.
    uint8_t inpPin  = 0;    // The input pin we last configured.
    uint8_t node    = 0;    // The node we're configuring.
    uint8_t pin     = 0;    // The pin we're configuring.
    

    /** Display all current data.
     */
    void displayAll()
    {
        lcd.printAt(LCD_COL_START, LCD_ROW_TOP, M_TOP_MENU[topMenu], LCD_LEN_OPTION);
        switch (topMenu)
        {
            case TOP_SYSTEM: displaySystem();
                             break;
            case TOP_INPUT:  node = inpNode;
                             pin  = inpPin;
                             displayNode();
                             break;
            case TOP_OUTPUT: node = outNode;
                             pin  = outPin;
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


    /** Display move node ID.
     */
    void displayNewNode(uint8_t aNode)
    {
        lcd.clearRow(LCD_COL_START, LCD_ROW_BOT);
        lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_NEW_NODE_NO);
        lcd.printAt(LCD_COL_NODE,  LCD_ROW_BOT, HEX_CHARS[aNode]);
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
            case SYS_NODES:  displaySystemNodesParams();
                             break;
            case SYS_DEBUG:  displaySystemDebugParams();
                             break;
            default:         systemFail(M_PARAMS, sysMenu, 0);
                             break;
        }
    }


    /** Display System's report parameter.
     */
    void displaySystemReportParams()
    {
        lcd.clearRow(LCD_COL_MARK, LCD_ROW_BOT);
        lcd.printAt(LCD_COL_REPORT_PARAM, LCD_ROW_BOT, M_REPORT_PROMPTS[systemData.reportLevel], LCD_LEN_OPTION);    
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


    /** Display System's report parameter.
     */
    void displaySystemNodesParams()
    {
        lcd.clearRow(LCD_COL_MARK, LCD_ROW_BOT);
    }


    /** Display System's report parameter.
     */
    void displaySystemDebugParams()
    {
        lcd.clearRow(LCD_COL_MARK, LCD_ROW_BOT);
        lcd.printAt(LCD_COL_DEBUG_PARAM, LCD_ROW_BOT, M_DEBUG_PROMPTS[systemData.debugLevel], LCD_LEN_OPTION);    
    }


    /** Display an i2c parameter's prompt above it.
     */
    void displayI2cPrompt(int aParam)
    {
        lcd.clearRow(LCD_COL_I2C_PARAM, LCD_ROW_TOP);
        lcd.printAt(LCD_COL_I2C_PARAM + aParam * LCD_COL_I2C_STEP, LCD_ROW_TOP, M_I2C_PROMPTS[aParam], LCD_LEN_OPTION);
    }


    /** Display Export detail menu.
     */
    void displayDetailExport()
    {
        lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_EXPORT_TYPES[expMenu], LCD_LEN_OPTION);
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
        if (aType != OUTPUT_TYPE_NONE)
        {    
            lcd.printAtHex(col, LCD_ROW_BOT, outputDef.getLo(),    2);
            col += LCD_COL_OUTPUT_STEP;
            lcd.printAtHex(col, LCD_ROW_BOT, outputDef.getHi(),    2);
            col += LCD_COL_OUTPUT_STEP;
            lcd.printAtHex(col, LCD_ROW_BOT, outputDef.getPace(),  1);
            col += LCD_COL_OUTPUT_STEP - 1;
            lcd.printAtHex(col, LCD_ROW_BOT, outputDef.getDelay(), 1);
        }
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
        if (!isInputNode(inpNode))
        {
            inpNode = nextNode(inpNode, 1, true, true);
        }
        if (!isOutputNode(outNode))
        {
            outNode = nextNode(outNode, 1, false, true);
        }
        loadInput(inpNode, inpPin);
        readOutput(outNode, outPin);
        
        lcd.clear();
        displayAll();
        markField(LCD_COL_START, LCD_ROW_TOP, LCD_COL_MARK, true);

        while (!finished)
        {
            switch (waitForButton())
            {
                case BUTTON_NONE:   break;
                case BUTTON_UP:     topMenu += 2;           // Use +1 to compensate for the -1 that the code below will do.
                case BUTTON_DOWN:   topMenu -= 1;
                                    topMenu += TOP_MAX;     // Ensure in-range.
                                    topMenu %= TOP_MAX;
                                    displayAll();
                                    markField(LCD_COL_START, LCD_ROW_TOP, LCD_COL_MARK, true);
                                    break;
                case BUTTON_SELECT: break;
                case BUTTON_LEFT:   finished = true;
                                    break;
                case BUTTON_RIGHT:  markField(LCD_COL_START, LCD_ROW_TOP, LCD_COL_MARK, false);
                                    switch (topMenu)
                                    {
                                        case TOP_SYSTEM: menuSystem();
                                                         break;
                                        case TOP_INPUT:  menuNode(true);
                                                         inpNode = node;
                                                         inpPin = pin;
                                                         break;
                                        case TOP_OUTPUT: menuNode(false);
                                                         outNode = node;
                                                         outPin = pin;
                                                         break;
                                        case TOP_EXPORT: menuExport();
                                                         break;
                                        case TOP_IMPORT: menuImport();
                                                         break;
                                        default:         systemFail(M_CONFIG, topMenu, 0);
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
        boolean finished   = false;
        boolean changed    = false;
        uint8_t debugLevel = getDebug();

        markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, true);

        while (!finished)
        {
            switch (waitForButton())
            {
                case BUTTON_NONE:   break;
                case BUTTON_UP:     sysMenu += 2;     // Use +1 to compensate for the -1 that the code below will do.
                case BUTTON_DOWN:   sysMenu -= 1;
                                    sysMenu += SYS_MAX;     // Ensure in-range.
                                    sysMenu %= SYS_MAX;
                                    displayDetailSystem();
                                    markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, true);
                                    break;
                case BUTTON_SELECT: if (changed)
                                    {
                                        if (confirm())
                                        {
                                            saveSystemData();
                                            if (debugLevel != getDebug())
                                            {
                                                sendDebugLevel();
                                            }
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
                                            saveSystemData();
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
                                    switch (sysMenu)
                                    {
                                        case SYS_REPORT: changed = menuSystemReport();
                                                         break;
                                        case SYS_I2C:    changed = menuSystemI2c();
                                                         break;
                                        case SYS_NODES:  mapHardware();
                                                         waitForButtonRelease();
                                                         displayAll();
                                                         break;
                                        case SYS_DEBUG:  changed = menuSystemDebug();
                                                         break;
                                        default:         systemFail(M_SYSTEM, sysMenu, 0);
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
                case BUTTON_UP:     systemData.reportLevel += 2;              // Allow for decrement in BUTTON_DOWN code below.
                case BUTTON_DOWN:   systemData.reportLevel -= 1;
                                    systemData.reportLevel += REPORT_MAX;     // Ensure in-range.
                                    systemData.reportLevel %= REPORT_MAX;
                                    lcd.printAt(LCD_COL_REPORT_PARAM, LCD_ROW_BOT, M_REPORT_PROMPTS[systemData.reportLevel], LCD_COL_REPORT_LENGTH);
                                    changed = true;
                                    break;
                case BUTTON_SELECT: break;
                case BUTTON_LEFT:   finished = true;
                                    break;
                case BUTTON_RIGHT:  break;
            }
        }

        markField(LCD_COL_REPORT_PARAM, LCD_ROW_BOT, LCD_COL_REPORT_LENGTH, false);
        
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


    /** Process System redebug menu.
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
                case BUTTON_UP:     systemData.debugLevel += 2;             // Allow for decrement in BUTTON_DOWN code below.
                case BUTTON_DOWN:   systemData.debugLevel -= 1;
                                    systemData.debugLevel += DEBUG_MAX;     // Ensure in-range.
                                    systemData.debugLevel %= DEBUG_MAX;
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
                case BUTTON_UP:     expMenu += 2;           // Use +1 to compensate for the -1 that the code below will do.
                case BUTTON_DOWN:   expMenu -= 1;
                                    expMenu += EXP_MAX;     // Ensure in-range.
                                    expMenu %= EXP_MAX;
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
        displayAll();
    }


    /** Process Node menu for Input or Output.
     */
    void menuNode(boolean aIsInput)
    {
        boolean finished    = false;
        int8_t  reportLevel = systemData.reportLevel;    // Record reportLevel so we can turn it back on again.

        systemData.reportLevel = 0;
        markField(LCD_COL_NODE, LCD_ROW_TOP, 1, true);
        
        while (!finished)
        {
            int adjust = 0;
            
            switch (waitForButton())
            {
                case BUTTON_NONE:   break;
                case BUTTON_UP:     adjust += 2;     // Use +1 to compensate for the -1 that the code below will do.
                case BUTTON_DOWN:   adjust -= 1;
                                    node = nextNode(node, adjust, aIsInput, true);
                                    lcd.printAt(LCD_COL_NODE, LCD_ROW_TOP, HEX_CHARS[node]);
                                    displayDetail();
                                    break;
                case BUTTON_SELECT: if (!aIsInput)
                                    {
                                        markField(LCD_COL_NODE, LCD_ROW_TOP, 1, false);
                                        menuNewNode();
                                        markField(LCD_COL_NODE, LCD_ROW_TOP, 1, true);
                                    }
                                    break;
                case BUTTON_LEFT:   finished = true;
                                    markField(LCD_COL_NODE, LCD_ROW_TOP, 1, false);
                                    break;
                case BUTTON_RIGHT:  markField(LCD_COL_NODE, LCD_ROW_TOP, 1, false);
                                    menuPin(aIsInput);
                                    markField(LCD_COL_NODE, LCD_ROW_TOP, 1, true);
                                    break;
            }
        }

        systemData.reportLevel = reportLevel;
        markField(LCD_COL_NODE, LCD_ROW_TOP, 1, false);
    }


    /** Find the next (inUse) node in the given direction.
     *  Or not inUse if flag indicates such.
     *  Load the node's data (if it's in use).
     */
    uint8_t nextNode(uint8_t aStart, int aAdjust, boolean aIsInput, boolean aInUse)
    {
        uint8_t next = aStart & (aIsInput ? INPUT_NODE_MASK : OUTPUT_NODE_MASK);
        
        for (int i = 0; i < (aIsInput ? INPUT_NODE_MAX : OUTPUT_NODE_MAX); i++)
        {
            next = (next + aAdjust) & (aIsInput ? INPUT_NODE_MASK : OUTPUT_NODE_MASK);
            
            if (   (aIsInput)
                && (aInUse == isInputNode(next)))
            {
                if (aInUse)
                {
                    loadInput(next, pin);
                }
                break;
            }
            else if (   (!aIsInput)
                     && (aInUse == isOutputNode(next)))
            {
                if (aInUse)
                {
                    readOutput(next, pin);
                }
                break;
            }
        }

        return next;
    }


    /** Move a node to a new number.
     */
    void menuNewNode()
    {
        boolean finished = false;
        boolean changed  = false;
        boolean jumpers  = false;
        uint8_t newNode  = node;
        
        displayNewNode(newNode);
        markField(LCD_COL_NODE, LCD_ROW_BOT, 1, true);
        
        while (!finished)
        {
            switch (waitForButton())
            {
                case BUTTON_NONE:   break;
                case BUTTON_UP:     if (jumpers)
                                    {
                                        jumpers = false;
                                    }
                                    else
                                    {
                                        newNode = nextNode(newNode,  1, false, false);
                                    }
                                    lcd.printAt(LCD_COL_NODE, LCD_ROW_BOT, HEX_CHARS[newNode]);
                                    changed = true;
                                    break;
                case BUTTON_DOWN:   if (jumpers)
                                    {
                                        jumpers = false;
                                    }
                                    else
                                    {
                                        newNode = nextNode(newNode, -1, false, false);
                                    }
                                    lcd.printAt(LCD_COL_NODE, LCD_ROW_BOT, HEX_CHARS[newNode]);
                                    changed = true;
                                    break;
                case BUTTON_SELECT: if (   (jumpers)
                                        || (node != newNode))
                                    {
                                        if (confirm())
                                        {
                                            node = renumberNode(node, (jumpers ? I2C_MODULE_ID_JUMPERS : newNode));
                                            readOutput(node, pin);
                                            lcd.printAt(LCD_COL_NODE, LCD_ROW_TOP, HEX_CHARS[node]);
                                            finished = true;
                                        }
                                        else
                                        {
                                            displayNewNode(newNode);
                                            lcd.printAt(LCD_COL_NODE, LCD_ROW_BOT, (jumpers ? CHAR_DOT : HEX_CHARS[newNode]));
                                            markField(LCD_COL_NODE, LCD_ROW_BOT, 1, true);
                                        }
                                    }
                                    else
                                    {
                                        finished = true;
                                    }
                                    break;
                case BUTTON_LEFT:   if (   (!changed)
                                        || (cancel()))
                                    {
                                        finished = true;
                                    }
                                    else
                                    {
                                        displayNewNode(newNode);
                                        markField(LCD_COL_NODE, LCD_ROW_BOT, 1, true);
                                    }
                                    break;
                case BUTTON_RIGHT:  jumpers = !jumpers;
                                    lcd.printAt(LCD_COL_NODE, LCD_ROW_BOT, (jumpers ? CHAR_DOT : HEX_CHARS[newNode]));
                                    break;
            }
        }

        displayDetail();
    }


    /** Change an output node's number.
     *  Return the number it was changed to (if successful).
     */
    uint8_t renumberNode(uint8_t aOldNode, uint8_t aNewNode)
    {
        int response = aOldNode;
        
        Wire.beginTransmission(systemData.i2cOutputBaseID + node);
        Wire.write(COMMS_CMD_SYSTEM | COMMS_SYS_RENUMBER);
        Wire.write(aNewNode);
        if (   ((response = Wire.endTransmission()) == 0)
            && ((response = Wire.requestFrom(systemData.i2cOutputBaseID + node, OUTPUT_RENUMBER_LEN)) == OUTPUT_RENUMBER_LEN)
            && ((response = Wire.read()) >= 0))
        {
            response &= OUTPUT_NODE_MASK;

            if (aOldNode != response)       // Change actually happened.
            {
                // Mark the old node absent and the new one present.
                setOutputNodeAbsent(aOldNode);
                setOutputNodePresent(response);
                
                // Renumber all the effected inputs' Output nodes.
                for (uint8_t node = 0; node < INPUT_NODE_MAX; node++)
                {
                    for (uint8_t pin = 0; pin < INPUT_PIN_MAX; pin++)
                    {
                        loadInput(node, pin);

                        // Adjust all the Input's Outputs if they reference either the old or new node number.
                        for (uint8_t index = 0; index < INPUT_OUTPUT_MAX; index++)
                        {
                            if (inputDef.getOutputNode(index) == aOldNode)
                            {
                                inputDef.setOutputNode(index, response);
                                saveInput();
                            }
                            else if (inputDef.getOutputNode(index) == response)
                            {
                                inputDef.setOutputNode(index, aOldNode);
                                saveInput();
                            }
                        }
                    }
                }
    
                if (isDebug(DEBUG_BRIEF))
                {
                    Serial.print(millis());
                    Serial.print(CHAR_TAB);
                    Serial.print(PGMT(M_DEBUG_RENUMBER));
                    Serial.print(CHAR_SPACE);
                    Serial.print(aOldNode, HEX);
                    Serial.print(PGMT(M_DEBUG_NODE));
                    Serial.print(aNewNode, HEX);
                    Serial.println();    
                }
            }
        }
        else
        {
            if (isDebug(DEBUG_ERRORS))
            {
                Serial.print(millis());
                Serial.print(CHAR_TAB);
                Serial.print(PGMT(M_DEBUG_RENUMBER));
                Serial.print(CHAR_SPACE);
                Serial.print(aOldNode, HEX);
                Serial.print(PGMT(M_DEBUG_NODE));
                Serial.print(aNewNode, HEX);
                Serial.print(PGMT(M_DEBUG_RETURN));
                Serial.print(response, HEX);
                Serial.println();    
            }

            systemFail(M_I2C_ERROR, response, DELAY_READ);
            response = aOldNode;
        }

        return response;
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
                case BUTTON_UP:     pin += 2;                                               // Use +1 to compensate for the -1 that the code below will do.
                case BUTTON_DOWN:   pin -= 1;
                                    pin += (aIsInput ? INPUT_PIN_MAX : OUTPUT_PIN_MAX);     // Ensure within range.
                                    pin %= (aIsInput ? INPUT_PIN_MAX : OUTPUT_PIN_MAX);
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
                case BUTTON_SELECT: if (aIsInput)
                                    {
                                        testInput();
                                    }
                                    else
                                    {
                                        testOutput(false);
                                    }
                                    break;
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
                case BUTTON_UP:     inputType += 2;                 // Use +1 to compensate for the -1 that the code below will do.
                case BUTTON_DOWN:   inputType -= 1;
                                    inputType += INPUT_TYPE_MAX;    // Ensure in-range.
                                    inputType %= INPUT_TYPE_MAX;
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

        uint8_t index    = 0;

        lcd.clearRow(LCD_COL_INPUT_OUTPUT, LCD_ROW_BOT);
        lcd.printAt(LCD_COL_INPUT_OUTPUT, LCD_ROW_BOT, (INPUT_OUTPUT_ID(index)));
        displayInputEdit(index);
        markField(LCD_COL_INPUT_OUTPUT, LCD_ROW_BOT, 1, true);

        while (!finished)
        {
            switch (waitForButton())
            {
                case BUTTON_NONE:   break;
                case BUTTON_UP:     index += 2;                     // Use +1 to compensate for the -1 that the code below will do.
                case BUTTON_DOWN:   index -= 1;
                                    index += INPUT_OUTPUT_MAX;      // Ensure in-range.
                                    index %= INPUT_OUTPUT_MAX;
                                    lcd.printAt(LCD_COL_INPUT_OUTPUT, LCD_ROW_BOT, (INPUT_OUTPUT_ID(index)));
                                    displayInputEdit(index);
                                    break;
                case BUTTON_SELECT: testInputOutput(index);
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
                                        inputDef.setOutputNode(aIndex, nextNode(inputDef.getOutputNode(aIndex), 1, false, true));
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
                                        inputDef.setOutputNode(aIndex, nextNode(inputDef.getOutputNode(aIndex), -1, false, true));
                                    }
                                    displayInputEdit(aIndex);
                                    changed = true;
                                    break;
                case BUTTON_SELECT: // Enable/disable this output.
                                    changed = true;
                                    inputDef.setDisabled(aIndex, !inputDef.isDisabled(aIndex));
                                    displayInputEdit(aIndex);
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
                case BUTTON_SELECT: changed = true;
                                    inputDef.setDisabled(aIndex, !inputDef.isDisabled(aIndex));
                                    displayInputEdit(aIndex);
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


    /** Operate the Input's specified Output to test if it works.
     */
    void testInputOutput(uint8_t aIndex)
    {
        boolean currentState = false;
        
        readOutput(inputDef.getOutput(aIndex));
        currentState = outputDef.getState();
        processInputOutput(aIndex, !currentState, 0);
        waitForButtonRelease();
        processInputOutput(aIndex,  currentState, 0);
    }


    /** Process Output menu.
     */
    void menuOutput()
    {
        boolean finished   = false;
        boolean changed    = false;
        uint8_t outputType = outputDef.getType();       // Retrieve type

        // Mark the field.
        markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, true);

        while (!finished)
        {
            switch (waitForButton())
            {
                case BUTTON_NONE:   break;
                case BUTTON_UP:     outputType += 2;                    // Use +1 to compensate for the -1 that the code below will do.
                case BUTTON_DOWN:   outputType -= 1;
                                    outputType += OUTPUT_TYPE_MAX;      // Ensure in-range.
                                    outputType %= OUTPUT_TYPE_MAX;
                                    outputDef.setType(outputType);
                                    writeOutput(false);
                                    
                                    lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_OUTPUT_TYPES[outputType], LCD_LEN_OPTION);
                                    displayOutputParams(outputType);
                                    markField(LCD_COL_START, LCD_ROW_BOT, LCD_COL_MARK, true);
                                    changed = true;
                                    break;
                case BUTTON_SELECT: if (changed)
                                    {
                                        if (confirm())
                                        {
                                            writeOutput(true);
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
                                            resetOutput();
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
                                        case OUTPUT_TYPE_NONE:   break;
                                        case OUTPUT_TYPE_SERVO:
                                        case OUTPUT_TYPE_SIGNAL: changed |= menuOutputLo(OUTPUT_SERVO_MAX);
                                                                 break;
                                        case OUTPUT_TYPE_LED:
                                        case OUTPUT_TYPE_FLASH:
                                        case OUTPUT_TYPE_BLINK:  changed |= menuOutputLo(OUTPUT_LED_MAX);
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
        boolean state    = outputDef.getState();

        displayOutputAngles();
        markField(LCD_COL_OUTPUT_LO, LCD_ROW_BOT, OUTPUT_HI_LO_SIZE, true);
        
        outputDef.setState(false);
        writeOutput(pin);

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
                                    writeOutput(false);
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
                                    writeOutput(false);
                                    changed = true;
                                    break;
                case BUTTON_SELECT: testOutput(false);
                                    break;
                case BUTTON_LEFT:   finished = true;
                                    break;
                case BUTTON_RIGHT:  markField(LCD_COL_OUTPUT_LO, LCD_ROW_BOT, OUTPUT_HI_LO_SIZE, false);
                                    changed |= menuOutputHi(aLimit);
                                    markField(LCD_COL_OUTPUT_LO, LCD_ROW_BOT, OUTPUT_HI_LO_SIZE, true);
                                    break;
            }
        }

        markField(LCD_COL_OUTPUT_LO, LCD_ROW_BOT, OUTPUT_HI_LO_SIZE, false);
        
        outputDef.setState(state);
        writeOutput(pin);
        
        return changed;
    }


    /** Process the Output Hi parameter.
     */
    boolean menuOutputHi(int aLimit)
    {
        boolean finished = false;
        boolean changed  = false;

        markField(LCD_COL_OUTPUT_HI, LCD_ROW_BOT, OUTPUT_HI_LO_SIZE, true);

        outputDef.setState(true);
        writeOutput(pin);

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
                                    writeOutput(false);
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
                                    writeOutput(false);
                                    changed = true;
                                    break;
                case BUTTON_SELECT: testOutput(false);
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

        outputDef.setState(false);
        writeOutput(pin);
                
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
                                    writeOutput(false);
                                    changed = true;
                                    break;
                case BUTTON_DOWN:   value = (value - 1) & OUTPUT_PACE_MASK;
                                    outputDef.setPace(value);
                                    lcd.printAt(LCD_COL_OUTPUT_PACE, LCD_ROW_BOT, HEX_CHARS[value]);
                                    writeOutput(false);
                                    changed = true;
                                    break;
                case BUTTON_SELECT: testOutput(false);
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
                case BUTTON_SELECT: testOutput(true);
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
     *  Most outputs change state and then change back again.
     *  Flashers always go Hi first, then Lo.
     */
    void testOutput(boolean aIncludeDelay)
    {
        if (outputDef.isFlasher())
        {
            writeOutputState(true,  aIncludeDelay ? outputDef.getDelay() : 0);
            waitForButtonRelease();
            writeOutputState(false, aIncludeDelay ? outputDef.getDelay() : 0);
            writeOutput(false);
        }
        else
        {
            writeOutputState(!outputDef.getState(), aIncludeDelay ? outputDef.getDelay() : 0);
            waitForButtonRelease();
            writeOutputState( outputDef.getState(), aIncludeDelay ? outputDef.getDelay() : 0);
        }
    }


    /** Mark a variable with field markers.
     */
    void markField(int aCol, int aRow, int aLen, boolean aShow)
    {
        if (aCol > 0)
        {
            lcd.printAt(aCol - 1, aRow, aShow ? CHAR_RIGHT : CHAR_SPACE);
        }
        lcd.printAt(aCol + aLen, aRow, aShow ? CHAR_LEFT : CHAR_SPACE);
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
