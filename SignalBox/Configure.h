/** Setup hardware.
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

#ifndef Configure_h
#define Configure_h


// Top-level menu states.
const uint8_t TOP_SYSTEM = 0;
const uint8_t TOP_INPUT  = 1;
const uint8_t TOP_OUTPUT = 2;
const uint8_t TOP_LOCKS  = 3;
const uint8_t TOP_EXPORT = 4;
const uint8_t TOP_IMPORT = 5;
const uint8_t TOP_MAX    = 6;

// Sys menu states.
const uint8_t SYS_REPORT = 0;
const uint8_t SYS_NODES  = 1;
const uint8_t SYS_IDENT  = 2;
const uint8_t SYS_DEBUG  = 3;

// Don't include DEBUG state if it's disabled.
#ifndef isDebug
const uint8_t SYS_MAX    = 4;
#else
const uint8_t SYS_MAX    = 3;
#endif


/** Helper for use in callback to displayScannedInput().
 *  Forward reference.
 */
void displayScannedInput(uint8_t aNode, uint8_t aPin);


/** Configure the system.
 *  Shows menus and updates configuration as specified using shield or Uno buttons.
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
        disp.clear();
        disp.printProgStrAt(LCD_COL_START, LCD_ROW_TOP, M_CONFIG);
        buttons.waitForButtonRelease();

        menuTop();
    }


    /** Helper to display currently-selected input.
     *  Used when scanning inputs during configuration.
     */
    void displaySelectedInput(uint8_t aNode, uint8_t aPin)
    {
        inpNode = aNode;        // Record the node and pin.
        inpPin  = aPin;

        displayInputNode();     // Display the Input and it's configuration.
        displayDetail();
    }


    private:
    
    /** Process Top menu.
     */
    void menuTop()
    {
        bool finished = false;

        // Ensure node numbers are legitimate.
        if (!isInputNodePresent(inpNode))
        {
            inpNode = nextNode(inpNode, 1, true, true);
        }
        if (!isOutputNodePresent(outNode))
        {
            outNode = nextNode(outNode, 1, false, true);
        }

        disp.clear();
        displayAll();
        markField(LCD_COL_START, LCD_ROW_TOP, LCD_COL_MARK, true);

        while (!finished)
        {
            // Ensure correct Input and Output node definitions are loaded.
            inputMgr.loadInput(inpNode, inpPin);
            if (isOutputNodePresent(outNode))
            {
                readOutput(outNode, outPin);
            }

            switch (buttons.waitForButtonPress())
            {
                case BUTTON_NONE:   break;

                case BUTTON_UP:     topMenu += 2;           // Use +1 to compensate for the -1 that the code below will do.
                                    [[fallthrough]];
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
                                                         if (isOutputNodePresent(outNode))  // The outNode may have been corrupted by input node processing.
                                                         {
                                                            readOutput(outNode, outPin);    // So reload it.
                                                         }
                                                         break;
                                                         
                                        case TOP_OUTPUT:
                                        case TOP_LOCKS:  if (isOutputNodePresent(outNode))  // Only if the output node is present
                                                         {
                                                            menuNode(false);                // Process the Output node.
                                                         }
                                                         break;
                                                         
                                        case TOP_EXPORT: menuExport();
                                                         break;
                                                         
                                        case TOP_IMPORT: menuImport();
                                                         break;
                                                         
                                        default:         configFail(M_CONFIG, topMenu);
                                                         break;
                                    }

                                    markField(LCD_COL_START, LCD_ROW_TOP, LCD_COL_MARK, true);
                                    break;
            }
        }

        disp.clear();
        buttons.waitForButtonRelease();
    }


    /** Process System menu.
     */
    void menuSystem()
    {
        bool finished   = false;
        bool changed    = false;
        uint8_t debugLevel = systemMgr.getDebugLevel();

        markField(LCD_COL_START, LCD_ROW_DET, LCD_COL_MARK, true);

        while (!finished)
        {
            switch (buttons.waitForButtonPress())
            {
                case BUTTON_NONE:   break;

                case BUTTON_UP:     sysMenu += 2;                   // Use +1 to compensate for the -1 that the code below will do.
                                    [[fallthrough]];
                case BUTTON_DOWN:   sysMenu -= 1;
                                    sysMenu += SYS_MAX;             // Ensure in-range.
                                    sysMenu %= SYS_MAX;
                                    displayDetailSystem();
                                    markField(LCD_COL_START, LCD_ROW_DET, LCD_COL_MARK, true);
                                    break;

                case BUTTON_SELECT: if (changed)
                                    {
                                        if (confirm())              // Ask if change should be saved.
                                        {
                                            systemMgr.saveSystemData();
                                            if (debugLevel != systemMgr.getDebugLevel())
                                            {
                                                controller.sendDebugLevel();
                                            }
                                            displayDetailSystem();
                                            finished = true;
                                        }
                                        else
                                        {
                                            displayDetailSystem();
                                            markField(LCD_COL_START, LCD_ROW_DET, LCD_COL_MARK, true);
                                        }
                                    }
                                    else
                                    {
                                        finished = true;
                                    }
                                    break;

                case BUTTON_LEFT:   if (changed)
                                    {
                                        if (cancel())               // Ask if change should be canelled.
                                        {
                                            systemMgr.loadSystemData();
                                            displayDetailSystem();
                                            finished = true;
                                        }
                                        else
                                        {
                                            displayDetailSystem();
                                            markField(LCD_COL_START, LCD_ROW_DET, LCD_COL_MARK, true);
                                        }
                                    }
                                    else
                                    {
                                        finished = true;
                                    }
                                    break;

                case BUTTON_RIGHT:  markField(LCD_COL_START, LCD_ROW_DET, LCD_COL_MARK, false);
                                    switch (sysMenu)
                                    {
                                        case SYS_REPORT: changed |= menuSystemReport();
                                                         break;
                                        case SYS_NODES:  controller.scanHardware();
                                                         break;
                                        case SYS_IDENT:  identOutputs();
                                                         break;
                                        case SYS_DEBUG:  changed |= menuSystemDebug();
                                                         break;
                                        default:         configFail(M_SYSTEM, sysMenu);
                                                         break;
                                    }

                                    displayAll();
                                    markField(LCD_COL_START, LCD_ROW_DET, LCD_COL_MARK, true);
                                    break;
            }
        }

        markField(LCD_COL_START, LCD_ROW_DET, LCD_COL_MARK, false);
    }


    /** Process System report menu.
     *  Reurn true if changes made.
     */
    bool menuSystemReport()
    {
        bool finished = false;
        bool changed  = false;
        uint8_t reportLevel = systemMgr.getReportLevel();

        markField(LCD_COL_REPORT_PARAM, LCD_ROW_DET, LCD_COL_REPORT_LENGTH, true);

        while (!finished)
        {
            switch (buttons.waitForButtonPress())
            {
                case BUTTON_NONE:   break;

                case BUTTON_UP:     reportLevel += 2;              // Allow for decrement in BUTTON_DOWN code below.
                                    [[fallthrough]];
                case BUTTON_DOWN:   reportLevel -= 1;
                                    reportLevel += REPORT_MAX;     // Ensure in-range.
                                    reportLevel %= REPORT_MAX;
                                    disp.printProgStrAt(LCD_COL_REPORT_PARAM, LCD_ROW_DET, M_REPORT_PROMPTS[reportLevel], LCD_COL_REPORT_LENGTH);
                                    changed = true;
                                    break;

                case BUTTON_SELECT: break;

                case BUTTON_LEFT:   finished = true;
                                    break;

                case BUTTON_RIGHT:  break;
            }
        }

        systemMgr.setReportLevel(reportLevel);
        markField(LCD_COL_REPORT_PARAM, LCD_ROW_DET, LCD_COL_REPORT_LENGTH, false);

        return changed;
    }


    /** Process System redebug menu.
     *  Reurn true if changes made.
     */
    bool menuSystemDebug()
    {
        bool finished   = false;
        bool changed    = false;
        uint8_t debugLevel = systemMgr.getDebugLevel();

        markField(LCD_COL_DEBUG_PARAM, LCD_ROW_DET, LCD_COL_DEBUG_LENGTH, true);

        while (!finished)
        {
            switch (buttons.waitForButtonPress())
            {
                case BUTTON_NONE:   break;

                case BUTTON_UP:     debugLevel += 2;             // Allow for decrement in BUTTON_DOWN code below.
                                    [[fallthrough]];
                case BUTTON_DOWN:   debugLevel -= 1;
                                    debugLevel += DEBUG_MAX;     // Ensure in-range.
                                    debugLevel %= DEBUG_MAX;
                                    disp.printProgStrAt(LCD_COL_DEBUG_PARAM, LCD_ROW_DET, M_DEBUG_PROMPTS[debugLevel], LCD_COL_DEBUG_LENGTH);
                                    changed = true;
                                    break;

                case BUTTON_SELECT: break;

                case BUTTON_LEFT:   finished = true;
                                    break;

                case BUTTON_RIGHT:  break;
            }
        }

        systemMgr.setDebugLevel(debugLevel);
        markField(LCD_COL_DEBUG_PARAM, LCD_ROW_DET, LCD_COL_DEBUG_LENGTH, false);

        return changed;
    }


    /** Process Node menu for Input or Output.
     */
    void menuNode(bool aIsInput)
    {
        bool finished    = false;
        int8_t  reportLevel = systemMgr.getReportLevel();       // Record reportLevel so we can turn it back on again.

        systemMgr.setReportLevel(REPORT_NONE);
        markField(LCD_COL_NODE, LCD_ROW_TOP, 1, true);

        while (!finished)
        {
            int adjust = 0;

            switch (buttons.waitForButtonPress())
            {
                case BUTTON_NONE:   break;

                case BUTTON_UP:     adjust += 2;            // Use +1 to compensate for the -1 that the code below will do.
                                    [[fallthrough]];
                case BUTTON_DOWN:   adjust -= 1;
                                    if (aIsInput)
                                    {
                                        inpNode = nextNode(inpNode, adjust, aIsInput, true);
                                        inputMgr.loadInput(inpNode, inpPin);
                                    }
                                    else
                                    {
                                        outNode = nextNode(outNode, adjust, aIsInput, true);
                                        readOutput(outNode, outPin);

                                        // Handle (rare) case where output node has failed (and this is first time we noticed).
                                        if (!isOutputNodePresent(outNode))
                                        {
                                            delay(DELAY_READ);      // Time to read error message.
                                            displayAll();           // Recover display.
                                            finished = true;        // Abort.
                                        }
                                    }

                                    disp.printHexChAt(LCD_COL_NODE, LCD_ROW_TOP, aIsInput ? inpNode : outNode);
                                    displayDetail();
                                    break;

                case BUTTON_SELECT: markField(LCD_COL_NODE, LCD_ROW_TOP, 1, false);
                                    if (aIsInput)
                                    {
                                        menuScanInputs();
                                    }
                                    else
                                    {
                                        menuNewNode();
                                    }
                                    markField(LCD_COL_NODE, LCD_ROW_TOP, 1, true);
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

        systemMgr.setReportLevel(reportLevel);
        markField(LCD_COL_NODE, LCD_ROW_TOP, 1, false);
    }


    /** Scan inputs waiting for one to be actioned.
     */
    void menuScanInputs()
    {
        uint8_t button   = BUTTON_NONE;     // The button that was pressed.
        uint8_t curNode  = inpNode;         // The current input node and pin.
        uint8_t curPin   = inpPin;

        // Announce we're scanning
        disp.printProgStrAt(LCD_COL_START, LCD_ROW_DET, M_SCANNING, LCD_COLS);
        buttons.waitForButtonRelease();

        // Scan all the input nodes until a button is pressed.
        while ((button = buttons.readButton()) == BUTTON_NONE)
        {
            controller.scanInputs(displayScannedInput);        // Calls displayScannedInput() when an input is pressed.
            delay(DELAY_BUTTON_WAIT);
        }

        // If operation cancelled, revert to original input.
        if (button != BUTTON_SELECT)
        {
            inpNode = curNode;
            inpPin  = curPin;
        }

        // Load/reload selected (or original) input and display attributes.
        inputMgr.loadInput(inpNode, inpPin);
        displayInputNode();
        displayDetail();
    }


    /** Move a node to a new number.
     */
    void menuNewNode()
    {
        bool finished = false;       // Set when done.
        bool changed  = false;       // Set if a new node number has been chosen.
        bool jumpers  = false;       // Set to reset node to its jumper setting.
        uint8_t newNode  = outNode;     // The new node number to renumber to.

        displayNewNode(jumpers, newNode);
        markField(LCD_COL_NODE, LCD_ROW_DET, 1, true);

        while (!finished)
        {
            int adjust = 0;

            switch (buttons.waitForButtonPress())
            {
                case BUTTON_NONE:   break;

                case BUTTON_UP:     adjust += 2;        // Use +1 to compensate for the -1 that the code below will do.
                                    [[fallthrough]];
                case BUTTON_DOWN:   adjust -= 1;
                                    if (jumpers)
                                    {
                                        jumpers = false;
                                    }
                                    else
                                    {
                                        newNode = nextNode(newNode, adjust, false, false);
                                    }
                                    disp.printHexChAt(LCD_COL_NODE, LCD_ROW_DET, newNode);
                                    changed = true;
                                    break;

                case BUTTON_SELECT: if (   (jumpers)
                                        || (outNode != newNode))
                                    {
                                        if (confirm())      // Ask if change should be saved.
                                        {
                                            outNode = renumberNode(outNode, (jumpers ? I2C_MODULE_ID_JUMPERS : newNode));
                                            readOutput(outNode, outPin);
                                            displayAll();
                                            finished = true;
                                        }
                                        else
                                        {
                                            displayNewNode(jumpers, newNode);
                                            markField(LCD_COL_NODE, LCD_ROW_DET, 1, true);
                                        }
                                    }
                                    else
                                    {
                                        finished = true;
                                    }
                                    break;

                case BUTTON_LEFT:   if (   (!changed)
                                        || (cancel()))      // Ask if change should be cancelled    .
                                    {
                                        finished = true;
                                    }
                                    else
                                    {
                                        displayNewNode(jumpers, newNode);
                                        markField(LCD_COL_NODE, LCD_ROW_DET, 1, true);
                                    }
                                    break;

                case BUTTON_RIGHT:  jumpers = !jumpers;
                                    changed = true;
                                    disp.setCursor(LCD_COL_NODE, LCD_ROW_DET);
                                    if (jumpers)
                                    {
                                        disp.printCh(CHAR_DOT);
                                    }
                                    else
                                    {
                                        disp.printHexCh(newNode);
                                    }
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

        // Send the renumber command to the node concerned.
        if (   ((response = i2cComms.sendData(I2C_OUTPUT_BASE_ID + aOldNode, COMMS_CMD_SYSTEM | COMMS_SYS_RENUMBER, aOldNode, aNewNode)) == 0)
            && ((response = i2cComms.requestByte(I2C_OUTPUT_BASE_ID + aOldNode)) >= 0))
        {
            response &= OUTPUT_NODE_MASK;       // The new node number of the Output as returned by the node

            if (isDebug(DEBUG_BRIEF))
            {
                Serial.print(millis());
                Serial.print(CHAR_TAB);
                Serial.print(PGMT(M_RENUMBER));
                Serial.print(PGMT(M_DEBUG_NODE));
                Serial.print(aOldNode, HEX);
                Serial.print(PGMT(M_DEBUG_TO));
                Serial.print(aNewNode, HEX);
                Serial.println();
            }

            if (aOldNode != response)           // Change actually happened.
            {
                // Mark the old node absent and the new one present.
                setOutputNodePresent(aOldNode, false);
                setOutputNodePresent(response, true);

                // Swap the state flags of the two nodes.
                uint8_t oldStates = getOutputStates(aOldNode);
                setOutputStates(aOldNode, getOutputStates(response));
                setOutputStates(response, oldStates);

                // Show work as Inputs are updated.
                disp.clearRow(LCD_COL_START, LCD_ROW_DET);
                disp.printProgStrAt(LCD_COL_START, LCD_ROW_TOP, M_RENUMBER);
                disp.printProgStrAt(LCD_COL_START, LCD_ROW_DET, M_INPUT, LCD_LEN_OPTION);
                disp.setCursor(-INPUT_NODE_MAX, LCD_ROW_DET);

                // Renumber all the effected inputs' Output nodes.
                for (uint8_t node = 0; node < INPUT_NODE_MAX; node++)
                {
                    if (isInputNodePresent(node))
                    {
                        disp.printHexCh(node);
                    }
                    else
                    {
                        disp.printCh(CHAR_DOT);
                    }

                    // For all the Input's pins.
                    for (uint8_t pin = 0; pin < INPUT_PIN_MAX; pin++)
                    {
                        bool changed = false;
                        inputMgr.loadInput(node, pin);

                        // Adjust all the Input's Outputs if they referencethe old node number.
                        for (uint8_t index = 0; index < INPUT_OUTPUT_MAX; index++)
                        {
                            if (inputDef.getOutputNode(index) == aOldNode)
                            {
                                inputDef.setOutputNode(index, response);
                                changed = true;
                            }
                        }
                        if (changed)
                        {
                            inputMgr.saveInput();
                        }
                    }
                }
                buttons.waitForButtonClick();

                // Show work as Output locks are updated.
                disp.clearBottomRows();
                disp.printProgStrAt(LCD_COL_START, LCD_ROW_DET, M_OUTPUT, LCD_LEN_OPTION);
                disp.setCursor(-OUTPUT_NODE_HALF, LCD_ROW_EDT);

                // Renumber all the Outputs' locks as necessary.
                for (uint8_t node = 0; node < OUTPUT_NODE_MAX; node++)
                {
                    if (node == OUTPUT_NODE_HALF)
                    {
                        disp.setCursor(-OUTPUT_NODE_HALF, LCD_ROW_BOT);
                    }
                    if (isOutputNodePresent(node))
                    {
                        disp.printHexCh(node);

                        if (isDebug(DEBUG_BRIEF))
                        {
                            Serial.print(millis());
                            Serial.print(CHAR_TAB);
                            Serial.print(PGMT(M_DEBUG_MOVE));
                            Serial.print(CHAR_SPACE);
                            Serial.print(node, HEX);
                            Serial.print(PGMT(M_DEBUG_NODE));
                            Serial.print(aOldNode, HEX);
                            Serial.print(PGMT(M_DEBUG_TO));
                            Serial.print(response, HEX);
                            Serial.println();
                        }

                        i2cComms.sendData(I2C_OUTPUT_BASE_ID + node, COMMS_CMD_SYSTEM | COMMS_SYS_MOVE_LOCKS, aOldNode, response);
                    }
                    else
                    {
                        disp.printCh(CHAR_DOT);
                    }
                }
            }
        }
        else
        {
            if (isDebug(DEBUG_ERRORS))
            {
                Serial.print(millis());
                Serial.print(CHAR_TAB);
                Serial.print(PGMT(M_RENUMBER));
                Serial.print(CHAR_SPACE);
                Serial.print(aOldNode, HEX);
                Serial.print(PGMT(M_DEBUG_NODE));
                Serial.print(aNewNode, HEX);
                Serial.print(PGMT(M_DEBUG_RETURN));
                Serial.print(response, HEX);
                Serial.println();
            }

            configFail(M_OUTPUT, response);
            response = aOldNode;
        }

        buttons.waitForButtonClick();
        return response;
    }


    /** Process Pin menu.
     */
    void menuPin(bool aIsInput)
    {
        bool finished = false;

        markField(LCD_COL_PIN, LCD_ROW_TOP, 1, true);

        while (!finished)
        {
            switch (buttons.waitForButtonPress())
            {
                case BUTTON_NONE:   break;

                case BUTTON_UP:     if (aIsInput)
                                    {
                                        inpPin = (inpPin + 1) & INPUT_PIN_MASK;
                                        inputMgr.loadInput(inpNode, inpPin);
                                    }
                                    else
                                    {
                                        outPin = (outPin + 1) & OUTPUT_PIN_MASK;
                                        readOutput(outNode, outPin);
                                    }
                                    disp.printHexChAt(LCD_COL_PIN, LCD_ROW_TOP, aIsInput ? inpPin : outPin);
                                    displayDetail();
                                    break;

                case BUTTON_DOWN:   if (aIsInput)
                                    {
                                        inpPin = (inpPin - 1) & INPUT_PIN_MASK;
                                        inputMgr.loadInput(inpNode, inpPin);
                                    }
                                    else
                                    {
                                        outPin = (outPin - 1) & OUTPUT_PIN_MASK;
                                        readOutput(outNode, outPin);
                                    }
                                    disp.printHexChAt(LCD_COL_PIN, LCD_ROW_TOP, aIsInput ? inpPin : outPin);
                                    displayDetail();
                                    break;

                case BUTTON_SELECT: if (aIsInput)
                                    {
                                        testInput();
                                    }
                                    else
                                    {
                                        writeOutput();                      // Ensure changes aren't persisted.
                                        testOutput();
                                        resetOutput();                      // Ensure output is reset.
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
                                        if (topMenu == TOP_OUTPUT)
                                        {
                                            menuOutput();
                                        }
                                        else
                                        {
                                            menuLocks();
                                        }
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
        bool finished = false;
        bool changed  = false;

        markField(LCD_COL_START, LCD_ROW_DET, LCD_COL_MARK, true);

        while (!finished)
        {
            switch (buttons.waitForButtonPress())
            {
                case BUTTON_NONE:   break;

                case BUTTON_UP:     inputType += 2;                 // Use +1 to compensate for the -1 that the code below will do.
                                    [[fallthrough]];
                case BUTTON_DOWN:   inputType -= 1;
                                    inputType += INPUT_TYPE_MAX;    // Ensure in-range.
                                    inputType %= INPUT_TYPE_MAX;
                                    disp.printProgStrAt(LCD_COL_START, LCD_ROW_DET, M_INPUT_TYPES[inputType], LCD_LEN_OPTION);
                                    changed = true;
                                    break;

                case BUTTON_SELECT: if (changed)
                                    {
                                        if (confirm())              // Ask if change should be saved.
                                        {
                                            inputMgr.saveInput();
                                            finished = true;
                                        }
                                        else
                                        {
                                            displayDetailInput();
                                            markField(LCD_COL_START, LCD_ROW_DET, LCD_COL_MARK, true);
                                        }
                                    }
                                    else
                                    {
                                        finished = true;
                                    }
                                    break;

                case BUTTON_LEFT:   if (changed)
                                    {
                                        if (cancel())               // Ask if change should be cancelled.
                                        {
                                            inputMgr.loadInput(inpNode, inpPin);
                                            finished = true;
                                        }
                                        else
                                        {
                                            displayDetailInput();
                                            markField(LCD_COL_START, LCD_ROW_DET, LCD_COL_MARK, true);
                                        }
                                    }
                                    else
                                    {
                                        finished = true;
                                    }
                                    break;

                case BUTTON_RIGHT:  markField(LCD_COL_START, LCD_ROW_DET, LCD_COL_MARK, false);
                                    changed |= menuInputSelect();
                                    displayDetailInputOutput();
                                    markField(LCD_COL_START, LCD_ROW_DET, LCD_COL_MARK, true);
                                    break;
            }
        }

        // Ensure output node is reset (we may have corrupted it when chaninging an Input's Outputs).
        readOutput(outNode, outPin);

        displayDetailInput();
    }


    /** Select the Input Output to edit.
     */
    bool menuInputSelect()
    {
        bool finished = false;
        bool changed  = false;

        uint8_t index    = 0;

        disp.clearRow(LCD_COL_INPUT_OUTPUT, LCD_ROW_DET);
        disp.printChAt(LCD_COL_INPUT_OUTPUT, LCD_ROW_DET, (OPTION_ID(index)));
        displayInputEdit(index);
        markField(LCD_COL_INPUT_OUTPUT, LCD_ROW_DET, 1, true);

        while (!finished)
        {
            switch (buttons.waitForButtonPress())
            {
                case BUTTON_NONE:   break;

                case BUTTON_UP:     index += 2;                     // Use +1 to compensate for the -1 that the code below will do.
                                    [[fallthrough]];
                case BUTTON_DOWN:   index -= 1;
                                    index += INPUT_OUTPUT_MAX;      // Ensure in-range.
                                    index %= INPUT_OUTPUT_MAX;
                                    disp.printChAt(LCD_COL_INPUT_OUTPUT, LCD_ROW_DET, (OPTION_ID(index)));
                                    displayInputEdit(index);
                                    break;

                case BUTTON_SELECT: testInputOutput(index);
                                    break;

                case BUTTON_LEFT:   finished = true;
                                    break;

                case BUTTON_RIGHT:  markField(LCD_COL_INPUT_OUTPUT, LCD_ROW_DET, 1, false);
                                    changed |= menuInputOutputNode(index);
                                    markField(LCD_COL_INPUT_OUTPUT, LCD_ROW_DET, 1, true);
                                    break;
            }
        }

        markField(LCD_COL_INPUT_OUTPUT, LCD_ROW_DET, 1, false);

        return changed;
    }


    /** Process an Input's Output node (at the given index).
     */
    bool menuInputOutputNode(uint8_t aIndex)
    {
        bool changed  = false;
        bool finished = false;

        markField(LCD_COL_NODE, LCD_ROW_DET, 1, true);

        while (!finished)
        {
            int adjust = 0;

            switch (buttons.waitForButtonPress())
            {
                case BUTTON_NONE:   break;

                case BUTTON_UP:     adjust += 2;        // Use +1 to compensate for the -1 that the code below will do.
                                    [[fallthrough]];
                case BUTTON_DOWN:   adjust -= 1;
                                    if (inputDef.isDelay(aIndex))
                                    {
                                        inputDef.setDelay(aIndex, false);
                                    }
                                    else
                                    {
                                        // Increment/Decrement the node number (to the next available) at this index.
                                        inputDef.setOutputNode(aIndex, nextNode(inputDef.getOutputNode(aIndex), adjust, false, true));
                                    }
                                    displayInputEdit(aIndex);
                                    changed = true;
                                    break;

                case BUTTON_SELECT: changed = true;
                                    inputDef.setDelay(aIndex, !inputDef.isDelay(aIndex));       // Enable/disable this output.
                                    displayInputEdit(aIndex);
                                    break;

                case BUTTON_LEFT:   finished = true;
                                    break;

                case BUTTON_RIGHT:  markField(LCD_COL_NODE, LCD_ROW_DET, 1, false);
                                    changed |= menuInputOutputPin(aIndex);
                                    markField(LCD_COL_NODE, LCD_ROW_DET, 1, true);
                                    break;
            }
        }

        markField(LCD_COL_NODE, LCD_ROW_DET, 1, false);

        return changed;
    }


    /** Process an input's output pin.
     */
    bool menuInputOutputPin(uint8_t aIndex)
    {
        bool finished = false;
        bool changed  = false;

        markField(LCD_COL_PIN, LCD_ROW_DET, 1, true);

        while (!finished)
        {
            switch (buttons.waitForButtonPress())
            {
                case BUTTON_NONE:   break;

                case BUTTON_UP:     // Increment the pin number at this index.
                                    inputDef.setOutputPin(aIndex, inputDef.getOutputPin(aIndex) + 1);
                                    displayInputEdit(aIndex);
                                    changed = true;
                                    break;

                case BUTTON_DOWN:   // Decrement the pin number at this index.
                                    inputDef.setOutputPin(aIndex, inputDef.getOutputPin(aIndex) - 1);
                                    displayInputEdit(aIndex);
                                    changed = true;
                                    break;

                case BUTTON_SELECT: testInputOutput(aIndex);
                                    break;

                case BUTTON_LEFT:   finished = true;
                                    break;

                case BUTTON_RIGHT:  break;
            }
        }

        markField(LCD_COL_PIN, LCD_ROW_DET, 1, false);

        return changed;
    }


    /** Process Output menu.
     */
    void menuOutput()
    {
        bool finished   = false;
        bool changed    = false;
        uint8_t outputType = outputDef.getType();       // Retrieve type

        // Mark the field.
        markField(LCD_COL_START, LCD_ROW_DET, LCD_COL_MARK, true);

        while (!finished)
        {
            switch (buttons.waitForButtonPress())
            {
                case BUTTON_NONE:   break;

                case BUTTON_UP:     outputType += 2;                    // Use +1 to compensate for the -1 that the code below will do.
                                    [[fallthrough]];
                case BUTTON_DOWN:   outputType -= 1;
                                    outputType += OUTPUT_TYPE_MAX;      // Ensure in-range.
                                    outputType %= OUTPUT_TYPE_MAX;
                                    outputDef.setType(outputType);
                                    writeOutput();

                                    disp.printProgStrAt(LCD_COL_START, LCD_ROW_DET, M_OUTPUT_TYPES[outputType], LCD_LEN_OPTION);
                                    displayOutputParams(outputType);
                                    markField(LCD_COL_START, LCD_ROW_DET, LCD_COL_MARK, true);
                                    changed = true;
                                    break;

                case BUTTON_SELECT: if (changed)
                                    {
                                        if (confirm())                  // Ask if change should be saved.
                                        {
                                            writeSaveOutput();
                                            finished = true;
                                        }
                                        else
                                        {
                                            displayDetailOutput();
                                            markField(LCD_COL_START, LCD_ROW_DET, LCD_COL_MARK, true);
                                        }
                                    }
                                    else
                                    {
                                        finished = true;
                                    }
                                    break;

                case BUTTON_LEFT:   if (changed)
                                    {
                                        if (cancel())                   // Ask if change should be cancelled.
                                        {
                                            finished = true;
                                        }
                                        else
                                        {
                                            outputDef.setType(outputType);
                                            displayDetailOutput();
                                            markField(LCD_COL_START, LCD_ROW_DET, LCD_COL_MARK, true);
                                        }
                                    }
                                    else
                                    {
                                        finished = true;
                                    }
                                    break;

                case BUTTON_RIGHT:  markField(LCD_COL_START, LCD_ROW_DET, LCD_COL_MARK, false);
                                    switch (outputType)
                                    {
                                        case OUTPUT_TYPE_NONE:    break;
                                        case OUTPUT_TYPE_SERVO:
                                        case OUTPUT_TYPE_SIGNAL:  changed |= menuOutputLo(OUTPUT_SERVO_MAX);
                                                                  break;
                                        case OUTPUT_TYPE_LED:
                                        case OUTPUT_TYPE_LED_4:
                                        case OUTPUT_TYPE_ROAD_UK:
                                        case OUTPUT_TYPE_ROAD_RW:
                                        case OUTPUT_TYPE_FLASH:
                                        case OUTPUT_TYPE_BLINK:
                                        case OUTPUT_TYPE_RANDOM:  changed |= menuOutputLo(OUTPUT_LED_MAX);
                                                                  break;
                                        default:                  configFail(M_OUTPUT, outputType);
                                                                  break;
                                    }

                                    displayAll();
                                    markField(LCD_COL_START, LCD_ROW_DET, LCD_COL_MARK, true);
                                    break;
            }
        }

        resetOutput();              // Ensure output module is in correct state (and will persist state changes).
        displayDetailOutput();
    }


    /** Process Output's Lo parameter (0-180) menu.
     */
    bool menuOutputLo(uint8_t aLimit)
    {
        bool finished = false;
        bool changed  = false;
        bool state    = outputDef.getState();

        displayOutputAngles();
        markField(LCD_COL_OUTPUT_LO, LCD_ROW_BOT, OUTPUT_HI_LO_SIZE, true);

        outputDef.setState(false);
        writeOutput();

        while (!finished)
        {
            int autoRepeat = DELAY_BUTTON_DELAY;
            switch (buttons.waitForButtonPress())
            {
                case BUTTON_NONE:   break;

                case BUTTON_UP:     do
                                    {
                                        outputDef.setLo(outputDef.getLo() + 1);
                                        if (outputDef.getLo() > aLimit)
                                        {
                                            outputDef.setLo(0);
                                        }
                                        disp.printDecAt(LCD_COL_OUTPUT_LO, LCD_ROW_BOT, outputDef.getLo(), OUTPUT_HI_LO_SIZE);
                                        // writeOutput();
                                        i2cComms.sendData(I2C_OUTPUT_BASE_ID + outNode, COMMS_CMD_SET | outPin, outputDef.getLo(), -1);
                                        delay(autoRepeat);
                                        autoRepeat = DELAY_BUTTON_REPEAT;
                                    }
                                    while (buttons.readButton() != 0);
                                    changed = true;
                                    break;

                case BUTTON_DOWN:   do
                                    {
                                        outputDef.setLo(outputDef.getLo() - 1);
                                        if (outputDef.getLo() > aLimit)
                                        {
                                            outputDef.setLo(aLimit);
                                        }
                                        disp.printDecAt(LCD_COL_OUTPUT_LO, LCD_ROW_BOT, outputDef.getLo(), OUTPUT_HI_LO_SIZE);
                                        // writeOutput();
                                        i2cComms.sendData(I2C_OUTPUT_BASE_ID + outNode, COMMS_CMD_SET | outPin, outputDef.getLo(), -1);
                                        delay(autoRepeat);
                                        autoRepeat = DELAY_BUTTON_REPEAT;
                                    }
                                    while (buttons.readButton() != 0);
                                    changed = true;
                                    break;

                case BUTTON_SELECT: testOutput();
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
        writeOutput();

        return changed;
    }


    /** Process the Output Hi parameter.
     */
    bool menuOutputHi(uint8_t aLimit)
    {
        bool finished = false;
        bool changed  = false;

        markField(LCD_COL_OUTPUT_HI, LCD_ROW_BOT, OUTPUT_HI_LO_SIZE, true);

        outputDef.setState(true);
        writeOutput();

        while (!finished)
        {
            int autoRepeat = DELAY_BUTTON_DELAY;
            switch (buttons.waitForButtonPress())
            {
                case BUTTON_NONE:   break;

                case BUTTON_UP:     do
                                    {
                                        outputDef.setHi(outputDef.getHi() + 1);
                                        if (outputDef.getHi() > aLimit)
                                        {
                                            outputDef.setHi(0);
                                        }
                                        disp.printDecAt(LCD_COL_OUTPUT_HI, LCD_ROW_BOT, outputDef.getHi(), OUTPUT_HI_LO_SIZE);
                                        // writeOutput();
                                        i2cComms.sendData(I2C_OUTPUT_BASE_ID + outNode, COMMS_CMD_SET | outPin, outputDef.getHi(), -1);
                                        delay(autoRepeat);
                                        autoRepeat = DELAY_BUTTON_REPEAT;
                                    }
                                    while (buttons.readButton() != 0);
                                    changed = true;
                                    break;

                case BUTTON_DOWN:   do
                                    {
                                        outputDef.setHi(outputDef.getHi() - 1);
                                        if (outputDef.getHi() > aLimit)
                                        {
                                            outputDef.setHi(aLimit);
                                        }
                                        disp.printDecAt(LCD_COL_OUTPUT_HI, LCD_ROW_BOT, outputDef.getHi(), OUTPUT_HI_LO_SIZE);
                                        // writeOutput();
                                        i2cComms.sendData(I2C_OUTPUT_BASE_ID + outNode, COMMS_CMD_SET | outPin, outputDef.getHi(), -1);
                                        delay(autoRepeat);
                                        autoRepeat = DELAY_BUTTON_REPEAT;
                                    }
                                    while (buttons.readButton() != 0);
                                    changed = true;
                                    break;

                case BUTTON_SELECT: testOutput();
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
        writeOutput();

        return changed;
    }


    /** Process the Output's Pace parameter.
     */
    bool menuOutputPace()
    {
        bool finished = false;
        bool changed  = false;
        uint8_t value    = outputDef.getPace();

        displayOutputDelayPace();
        markField(LCD_COL_OUTPUT_PACE, LCD_ROW_BOT, 1, true);

        while (!finished)
        {
            switch (buttons.waitForButtonPress())
            {
                case BUTTON_NONE:   break;

                case BUTTON_UP:     value = (value + 1) & OUTPUT_PACE_MASK;
                                    outputDef.setPace(value);
                                    disp.printHexChAt(LCD_COL_OUTPUT_PACE, LCD_ROW_BOT, value);
                                    writeOutput();
                                    changed = true;
                                    break;

                case BUTTON_DOWN:   value = (value - 1) & OUTPUT_PACE_MASK;
                                    outputDef.setPace(value);
                                    disp.printHexChAt(LCD_COL_OUTPUT_PACE, LCD_ROW_BOT, value);
                                    writeOutput();
                                    changed = true;
                                    break;

                case BUTTON_SELECT: testOutput();
                                    break;

                case BUTTON_LEFT:   finished = true;
                                    break;

                case BUTTON_RIGHT:  markField(LCD_COL_OUTPUT_PACE, LCD_ROW_BOT, 1, false);
                                    changed |= menuOutputReset();
                                    markField(LCD_COL_OUTPUT_PACE, LCD_ROW_BOT, 1, true);
                                    break;
            }
        }

        markField(LCD_COL_OUTPUT_PACE, LCD_ROW_BOT, 1, false);

        return changed;
    }


    /** Process the Output's Reset parameter.
     */
    bool menuOutputReset()
    {
        bool finished = false;
        bool changed  = false;

        markField(LCD_COL_OUTPUT_RESET, LCD_ROW_BOT, OUTPUT_HI_LO_SIZE, true);

        while (!finished)
        {
            int autoRepeat = DELAY_BUTTON_DELAY;
            switch (buttons.waitForButtonPress())
            {
                case BUTTON_NONE:   break;

                case BUTTON_UP:     do
                                    {
                                        outputDef.setReset(outputDef.getReset() + 1);
                                        disp.printDecAt(LCD_COL_OUTPUT_HI, LCD_ROW_BOT, outputDef.getReset(), OUTPUT_HI_LO_SIZE);
                                        delay(autoRepeat);
                                        autoRepeat = DELAY_BUTTON_REPEAT;
                                    }
                                    while (buttons.readButton() != 0);
                                    writeOutput();
                                    changed = true;
                                    break;

                case BUTTON_DOWN:   do
                                    {
                                        outputDef.setReset(outputDef.getReset() - 1);
                                        disp.printDecAt(LCD_COL_OUTPUT_HI, LCD_ROW_BOT, outputDef.getReset(), OUTPUT_HI_LO_SIZE);
                                        delay(autoRepeat);
                                        autoRepeat = DELAY_BUTTON_REPEAT;
                                    }
                                    while (buttons.readButton() != 0);
                                    writeOutput();
                                    changed = true;
                                    break;

                case BUTTON_SELECT: testOutput();
                                    break;

                case BUTTON_LEFT:   finished = true;
                                    break;

                case BUTTON_RIGHT:  break;
            }
        }

        markField(LCD_COL_OUTPUT_RESET, LCD_ROW_BOT, OUTPUT_HI_LO_SIZE, false);

        return changed;
    }


    /** Process Output locks.
     */
    bool menuLocks()
    {
        bool finished = false;
        bool changed  = false;
        bool hi       = false;

        displayLockDetail(hi);
        markField(LCD_COL_START, LCD_ROW_DET, LCD_COL_LOCK_MARK, true);

        while (!finished)
        {
            switch (buttons.waitForButtonPress())
            {
                case BUTTON_NONE:   break;

                case BUTTON_UP:
                case BUTTON_DOWN:   hi = !hi;
                                    disp.printProgStrAt(LCD_COL_START, LCD_ROW_DET, hi ? M_HI : M_LO);
                                    displayLockEdit(hi, 0);
                                    break;

                case BUTTON_SELECT: if (changed)
                                    {
                                        if (confirm())                  // Ask if change should be saved.
                                        {
                                            writeOutput();
                                            writeSaveOutput();
                                            finished = true;
                                        }
                                        else
                                        {
                                            displayLockDetail(hi);
                                            markField(LCD_COL_START, LCD_ROW_DET, LCD_COL_LOCK_MARK, true);
                                        }
                                    }
                                    else
                                    {
                                        finished = true;
                                    }
                                    break;

                case BUTTON_LEFT:   if (changed)
                                    {
                                        if (cancel())                   // Ask if change should be cancelled.
                                        {
                                            resetOutput();
                                            finished = true;
                                        }
                                        else
                                        {
                                            displayLockDetail(hi);
                                            markField(LCD_COL_START, LCD_ROW_DET, LCD_COL_LOCK_MARK, true);
                                        }
                                    }
                                    else
                                    {
                                        finished = true;
                                    }
                                    break;

                case BUTTON_RIGHT:  markField(LCD_COL_START, LCD_ROW_DET, LCD_COL_LOCK_MARK, false);
                                    changed |= menuLockSelect(hi);
                                    markField(LCD_COL_START, LCD_ROW_DET, LCD_COL_LOCK_MARK, true);
                                    break;
            }
        }

        // markField(LCD_COL_START, LCD_ROW_DET, LCD_COL_LOCK_MARK, false);
        displayDetailOutput();

        return changed;
    }


    /** Process the lock selection.
     */
    bool menuLockSelect(bool aHi)
    {
        bool finished = false;
        bool changed  = false;

        uint8_t index    = 0;

        disp.clearRow(LCD_COL_LOCK_MARK, LCD_ROW_DET);
        displayLockEdit(aHi, index);
        markField(LCD_COL_LOCK_SELECT, LCD_ROW_DET, 1, true);

        while (!finished)
        {
            switch (buttons.waitForButtonPress())
            {
                case BUTTON_NONE:   break;

                case BUTTON_UP:     index += 2;                     // Use +1 to compensate for the -1 that the code below will do.
                                    [[fallthrough]];
                case BUTTON_DOWN:   index -= 1;
                                    index += OUTPUT_LOCK_MAX;       // Ensure in-range.
                                    index %= OUTPUT_LOCK_MAX;
                                    disp.printChAt(LCD_COL_LOCK_SELECT, LCD_ROW_DET, OPTION_ID(index));
                                    displayLockEdit(aHi, index);
                                    break;

                case BUTTON_SELECT: if (outputDef.isLock(aHi, index))
                                    {
                                        testOutput(outputDef.getLockNode(aHi, index), outputDef.getLockPin(aHi, index));
                                    }
                                    break;

                case BUTTON_LEFT:   finished = true;
                                    break;

                case BUTTON_RIGHT:  markField(LCD_COL_LOCK_SELECT, LCD_ROW_DET, 1, false);
                                    changed |= menuLockState(aHi, index);
                                    markField(LCD_COL_LOCK_SELECT, LCD_ROW_DET, 1, true);
                                    break;
            }
        }

        markField(LCD_COL_LOCK_SELECT, LCD_ROW_DET, 1, false);
        // displayLockEdit(aHi, index);

        return changed;
    }


    /** Display Lock parameters for edit.
     */
    void displayLockEdit(bool aHi, uint8_t aIndex)
    {
        disp.printChAt(LCD_COL_LOCK_SELECT, LCD_ROW_DET, OPTION_ID(aIndex));
        if (outputDef.isLock(aHi, aIndex))
        {
            disp.printProgStrAt(LCD_COL_LOCK_STATE, LCD_ROW_DET, (outputDef.getLockState(aHi, aIndex) ? M_HI : M_LO));
            disp.printHexChAt(LCD_COL_NODE,         LCD_ROW_DET, outputDef.getLockNode(aHi, aIndex));
            disp.printHexChAt(LCD_COL_PIN,          LCD_ROW_DET, outputDef.getLockPin(aHi, aIndex));
        }
        else
        {
            disp.printChAt(LCD_COL_LOCK_STATE,      LCD_ROW_DET, CHAR_DOT);
            disp.printChAt(LCD_COL_LOCK_STATE + 1,  LCD_ROW_DET, CHAR_DOT);
            disp.printChAt(LCD_COL_NODE,            LCD_ROW_DET, CHAR_DOT);
            disp.printChAt(LCD_COL_PIN,             LCD_ROW_DET, CHAR_DOT);
        }
    }


    /** Process the lock state.
     */
    bool menuLockState(bool aHi, uint8_t aIndex)
    {
        bool finished = false;
        bool changed  = false;

        markField(LCD_COL_LOCK_STATE, LCD_ROW_DET, LCD_COL_LOCK_MARK, true);

        while (!finished)
        {
            switch (buttons.waitForButtonPress())
            {
                case BUTTON_NONE:   break;

                case BUTTON_UP:
                case BUTTON_DOWN:   if (!outputDef.isLock(aHi, aIndex))         // If currently no lock, enable one.
                                    {
                                        outputDef.setLock(aHi, aIndex, true);
                                        displayLockEdit(aHi, aIndex);
                                    }
                                    else                                        // Change the lock type (Lo/Hi).
                                    {
                                        outputDef.setLockState(aHi, aIndex, !outputDef.getLockState(aHi, aIndex));
                                        disp.printProgStrAt(LCD_COL_LOCK_STATE, LCD_ROW_DET, outputDef.getLockState(aHi, aIndex) ? M_HI : M_LO);
                                    }
                                    changed = true;
                                    break;

                case BUTTON_SELECT: outputDef.setLock(aHi, aIndex, !outputDef.isLock(aHi, aIndex));
                                    displayLockEdit(aHi, aIndex);
                                    changed = true;
                                    break;

                case BUTTON_LEFT:   finished = true;
                                    break;

                case BUTTON_RIGHT:  if (!outputDef.isLock(aHi, aIndex))
                                    {
                                        outputDef.setLock(aHi, aIndex, true);
                                        displayLockEdit(aHi, aIndex);
                                        changed = true;
                                    }
                                    markField(LCD_COL_LOCK_STATE, LCD_ROW_DET, LCD_COL_LOCK_MARK, false);
                                    changed |= menuLockNode(aHi, aIndex);
                                    markField(LCD_COL_LOCK_STATE, LCD_ROW_DET, LCD_COL_LOCK_MARK, true);
                                    break;
            }
        }

        markField(LCD_COL_LOCK_STATE, LCD_ROW_DET, LCD_COL_LOCK_MARK, false);

        return changed;
    }


    /** Process the lock node.
     */
    bool menuLockNode(bool aHi, uint8_t aIndex)
    {
        bool changed  = false;
        bool finished = false;

        markField(LCD_COL_NODE, LCD_ROW_DET, 1, true);

        while (!finished)
        {
            int adjust = 0;

            switch (buttons.waitForButtonPress())
            {
                case BUTTON_NONE:   break;

                case BUTTON_UP:     adjust += 2;    // Use +1 to compensate for the -1 that the code below will do.
                                    [[fallthrough]];
                case BUTTON_DOWN:   adjust -= 1;
                                    outputDef.setLockNode(aHi, aIndex, nextNode(outputDef.getLockNode(aHi, aIndex), adjust, false, true));
                                    disp.printHexChAt(LCD_COL_NODE, LCD_ROW_DET, outputDef.getLockNode(aHi, aIndex));
                                    changed = true;
                                    break;

                case BUTTON_SELECT: testOutput(outputDef.getLockNode(aHi, aIndex), outputDef.getLockPin(aHi, aIndex));
                                    break;

                case BUTTON_LEFT:   finished = true;
                                    break;

                case BUTTON_RIGHT:  markField(LCD_COL_NODE, LCD_ROW_DET, 1, false);
                                    changed |= menuLockPin(aHi, aIndex);
                                    markField(LCD_COL_NODE, LCD_ROW_DET, 1, true);
                                    break;
            }
        }

        markField(LCD_COL_NODE, LCD_ROW_DET, 1, false);

        return changed;
    }


    /** Process the lock node.
     */
    bool menuLockPin(bool aHi, uint8_t aIndex)
    {
        bool changed  = false;
        bool finished = false;

        markField(LCD_COL_PIN, LCD_ROW_DET, 1, true);

        while (!finished)
        {
            switch (buttons.waitForButtonPress())
            {
                case BUTTON_NONE:   break;

                case BUTTON_UP:     outputDef.setLockPin(aHi, aIndex, (outputDef.getLockPin(aHi, aIndex) + 1) & OUTPUT_PIN_MASK);
                                    disp.printHexChAt(LCD_COL_PIN, LCD_ROW_DET, outputDef.getLockPin(aHi, aIndex));
                                    changed = true;
                                    break;

                case BUTTON_DOWN:   outputDef.setLockPin(aHi, aIndex, (outputDef.getLockPin(aHi, aIndex) - 1) & OUTPUT_PIN_MASK);
                                    disp.printHexChAt(LCD_COL_PIN, LCD_ROW_DET, outputDef.getLockPin(aHi, aIndex));
                                    changed = true;
                                    break;

                case BUTTON_SELECT: testOutput(outputDef.getLockNode(aHi, aIndex), outputDef.getLockPin(aHi, aIndex));
                                    break;

                case BUTTON_LEFT:   finished = true;
                                    break;

                case BUTTON_RIGHT:  break;
            }
        }

        markField(LCD_COL_PIN, LCD_ROW_DET, 1, false);

        return changed;
    }


    /** Process Export menu.
     */
    void menuExport()
    {
        bool finished = false;

        markField(LCD_COL_START, LCD_ROW_DET, LCD_COL_MARK, true);

        while (!finished)
        {
            switch (buttons.waitForButtonPress())
            {
                case BUTTON_NONE:   break;

                case BUTTON_UP:     expMenu += 2;           // Use +1 to compensate for the -1 that the code below will do.
                                    [[fallthrough]];
                case BUTTON_DOWN:   expMenu -= 1;
                                    expMenu += EXP_MAX;     // Ensure in-range.
                                    expMenu %= EXP_MAX;
                                    displayDetailExport();
                                    break;

                case BUTTON_SELECT: break;

                case BUTTON_LEFT:   finished = true;
                                    break;

                case BUTTON_RIGHT:  markField(LCD_COL_START, LCD_ROW_DET, LCD_COL_MARK, false);
                                    importExport.doExport(expMenu);
                                    markField(LCD_COL_START, LCD_ROW_DET, LCD_COL_MARK, true);
                                    break;
            }
        }

        markField(LCD_COL_START, LCD_ROW_DET, LCD_COL_MARK, false);
    }


    /** Process the Import menu (which doesn't exist).
     *  Just run the import.
     */
    void menuImport()
    {
        importExport.doImport();
        displayAll();
    }


    /** Display all current data.
     */
    void displayAll()
    {
        disp.clear();
        displayTop();
        displayDetail();
    }


    /** Display the top row of data.
     */
    void displayTop()
    {
        disp.printProgStrAt(LCD_COL_START, LCD_ROW_TOP, M_TOP_MENU[topMenu], LCD_LEN_OPTION);
        switch (topMenu)
        {
            case TOP_SYSTEM: displaySystem();
                             break;

            case TOP_INPUT:  displayInputNode();
                             break;

            case TOP_OUTPUT:
            case TOP_LOCKS:  displayOutputNode();
                             break;

            case TOP_EXPORT:
            case TOP_IMPORT: displaySystem();
                             break;

            default:         configFail(M_ALL, topMenu);
                             break;
        }
    }


    /** Display Sysyem information.
     */
    void displaySystem()
    {
        disp.clearRow(LCD_COL_MARK, LCD_ROW_TOP);
        disp.printProgStrAt(-strlen_P(M_VERSION), LCD_ROW_TOP, M_VERSION);
    }


    /** Display the input node/pin selection line of the menu.
     */
    void displayInputNode()
    {
        disp.clearRow    (LCD_COL_MARK, LCD_ROW_TOP);
        disp.printHexChAt(LCD_COL_NODE, LCD_ROW_TOP, inpNode);
        disp.printHexChAt(LCD_COL_PIN,  LCD_ROW_TOP, inpPin);
    }


    /** Display the output node/pin selection line of the menu.
     */
    void displayOutputNode()
    {
        disp.clearRow(LCD_COL_MARK, LCD_ROW_TOP);
        if (isOutputNodePresent(outNode))
        {
            disp.printHexChAt(LCD_COL_NODE, LCD_ROW_TOP, outNode);
            disp.printHexChAt(LCD_COL_PIN,  LCD_ROW_TOP, outPin);
        }
    }


    /** Display the detail line of the menu.
     */
    void displayDetail()
    {
        disp.clearRow(LCD_COL_START, LCD_ROW_DET);
        switch (topMenu)
        {
            case TOP_SYSTEM: displayDetailSystem();
                             break;

            case TOP_INPUT:  displayDetailInput();
                             break;

            case TOP_OUTPUT:
            case TOP_LOCKS:  displayDetailOutput();
                             break;

            case TOP_EXPORT: displayDetailExport();
                             break;

            case TOP_IMPORT: displayDetailImport();
                             break;

            default:         configFail(M_DETAIL, topMenu);
                             break;
        }
    }


    /** Display detail for System menu.
     */
    void displayDetailSystem()
    {
        disp.printProgStrAt(LCD_COL_START, LCD_ROW_DET, M_SYS_TYPES[sysMenu], LCD_LEN_OPTION);
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

            case SYS_NODES:  displaySystemNodesParams();
                             break;

            case SYS_IDENT:  displaySystemIdentParams();
                             break;

            case SYS_DEBUG:  displaySystemDebugParams();
                             break;

            default:         configFail(M_PARAMS, sysMenu);
                             break;
        }
    }


    /** Display System's report parameter.
     */
    void displaySystemReportParams()
    {
        disp.clearRow(LCD_COL_MARK, LCD_ROW_DET);
        disp.printProgStrAt(LCD_COL_REPORT_PARAM, LCD_ROW_DET, M_REPORT_PROMPTS[systemMgr.getReportLevel()], LCD_LEN_OPTION);
    }


    /** Display System's report parameter.
     */
    void displaySystemNodesParams()
    {
        disp.clearRow(LCD_COL_MARK, LCD_ROW_DET);
    }


    /** Display System's report parameter.
     */
    void displaySystemIdentParams()
    {
        disp.clearRow(LCD_COL_MARK, LCD_ROW_DET);
    }


    /** Display System's report parameter.
     */
    void displaySystemDebugParams()
    {
        disp.clearRow(LCD_COL_MARK, LCD_ROW_DET);
        disp.printProgStrAt(LCD_COL_DEBUG_PARAM, LCD_ROW_DET, M_DEBUG_PROMPTS[systemMgr.getDebugLevel()], LCD_LEN_OPTION);
    }


    /** Display Input details.
     */
    void displayDetailInput()
    {
        disp.printProgStrAt(LCD_COL_START, LCD_ROW_DET, M_INPUT_TYPES[inputType], LCD_LEN_OPTION);
        displayDetailInputOutput();
    }


    /** Display Input's Output details.
     */
    void displayDetailInputOutput()
    {
        disp.clearRow(LCD_COL_MARK, LCD_ROW_DET);
        disp.setCursor(LCD_COL_INPUT_OUTPUT, LCD_ROW_DET);

        // Show as many Outputs as will fit in the space available. 3 characters per output.
        for (uint8_t index = 0; index <= (LCD_COLS - LCD_COL_INPUT_OUTPUT) / 3; index++)
        {
            if (inputDef.isDelay(index))
            {
                disp.printCh(CHAR_DOT);
                if (inputDef.getOutputPin(index) == 0)
                {
                    disp.printCh(CHAR_DOT);
                }
                else
                {
                    disp.printHexCh(inputDef.getOutputPin(index));
                }
            }
            else
            {
                disp.printHexCh(inputDef.getOutputNode(index));
                disp.printHexCh(inputDef.getOutputPin(index));
            }
            disp.printCh(CHAR_SPACE);
        }
    }


    /** Display Output details.
     */
    void displayDetailOutput()
    {
        if (isOutputNodePresent(outNode))
        {
            disp.printProgStrAt(LCD_COL_START, LCD_ROW_DET, M_OUTPUT_TYPES[outputDef.getType()], LCD_LEN_OPTION + 1);
            displayOutputParams(outputDef.getType());
        }
        else
        {
            disp.printProgStrAt(LCD_COL_START, LCD_ROW_DET, M_NO_OUTPUT, LCD_COLS);
        }
    }


    /** Display Output's parameters depending on type.
     */
    void displayOutputParams(uint8_t aType)
    {
        disp.clearRow(LCD_COL_OUTPUT_PARAM, LCD_ROW_DET);
        disp.setCursor(LCD_COL_OUTPUT_PARAM, LCD_ROW_DET);

        if (aType != OUTPUT_TYPE_NONE)
        {
            if (topMenu == TOP_OUTPUT)
            {
                disp.printHexByte(outputDef.getLo());
                disp.printCh(CHAR_SPACE);
                disp.printHexByte(outputDef.getHi());
                disp.printCh(CHAR_SPACE);
                disp.printHexCh(outputDef.getPace());
                disp.printCh(CHAR_SPACE);
                disp.printCh(outputDef.getResetCh());
            }
            else if (topMenu == TOP_LOCKS)
            {
                disp.printProgStr(M_LO);
                disp.printCh(CHAR_SPACE);
                disp.printHexCh(outputDef.getLockCount(false));
                disp.printCh(CHAR_SPACE);
                disp.printProgStr(M_HI);
                disp.printCh(CHAR_SPACE);
                disp.printHexCh(outputDef.getLockCount(true));
            }
            else
            {
                configFail(M_DETAIL, topMenu);
            }
        }
    }


    /** Display Export detail menu.
     */
    void displayDetailExport()
    {
        disp.printProgStrAt(LCD_COL_START, LCD_ROW_DET, M_EXPORT_TYPES[expMenu], LCD_LEN_OPTION);
    }


    /** Display Import detail menu.
     */
    void displayDetailImport()
    {
        disp.clearRow(LCD_COL_START, LCD_ROW_DET);
    }


    /** Display an Input's output settings, node and pin.
     */
    void displayInputEdit(uint8_t aIndex)
    {
        if (inputDef.isDelay(aIndex))
        {
            disp.printChAt(LCD_COL_NODE, LCD_ROW_DET, CHAR_DOT);
            if (inputDef.getOutputPin(aIndex) == 0)
            {
                disp.printChAt(LCD_COL_PIN,  LCD_ROW_DET, CHAR_DOT);
            }
            else
            {
                disp.printHexChAt(LCD_COL_PIN,  LCD_ROW_DET, inputDef.getOutputPin(aIndex));
            }
        }
        else
        {
            disp.printHexChAt(LCD_COL_NODE, LCD_ROW_DET, inputDef.getOutputNode(aIndex));
            disp.printHexChAt(LCD_COL_PIN,  LCD_ROW_DET, inputDef.getOutputPin(aIndex));
        }
    }


    /** Display Output's delay and pace parameters with suitable prompts.
     */
    void displayOutputDelayPace()
    {
        disp.clearRow(LCD_COL_OUTPUT_PARAM, LCD_ROW_EDT);
        disp.printProgStrAt(LCD_COL_OUTPUT_PACE  - 1, LCD_ROW_EDT, M_PACE);
        disp.printProgStrAt(LCD_COL_OUTPUT_RESET - 1, LCD_ROW_EDT, M_RESET);

        disp.clearRow(LCD_COL_OUTPUT_PARAM,    LCD_ROW_BOT);
        disp.printHexChAt(LCD_COL_OUTPUT_PACE, LCD_ROW_BOT, outputDef.getPace());
        disp.printDecAt(LCD_COL_OUTPUT_RESET,  LCD_ROW_BOT, outputDef.getReset(), OUTPUT_HI_LO_SIZE);
    }


    /** Display Output's angles with suitable prompts above.
     */
    void displayOutputAngles()
    {
        disp.clearRow(LCD_COL_OUTPUT_PARAM, LCD_ROW_EDT);
        disp.printProgStrAt(LCD_COL_OUTPUT_LO + OUTPUT_HI_LO_SIZE - sizeof(M_LO) + 1, LCD_ROW_EDT, M_LO);
        disp.printProgStrAt(LCD_COL_OUTPUT_HI + OUTPUT_HI_LO_SIZE - sizeof(M_HI) + 1, LCD_ROW_EDT, M_HI);

        disp.clearRow(LCD_COL_OUTPUT_PARAM, LCD_ROW_BOT);
        disp.printDecAt(LCD_COL_OUTPUT_LO, LCD_ROW_BOT, outputDef.getLo(), OUTPUT_HI_LO_SIZE);
        disp.printDecAt(LCD_COL_OUTPUT_HI, LCD_ROW_BOT, outputDef.getHi(), OUTPUT_HI_LO_SIZE);
    }


    /** Display move node ID.
     */
    void displayNewNode(bool aJumpers, uint8_t aNode)
    {
        disp.clearRow      (LCD_COL_START, LCD_ROW_DET);
        disp.printProgStrAt(LCD_COL_START, LCD_ROW_DET, M_NEW_NODE_NO);
        if (aJumpers)
        {
            disp.printChAt(LCD_COL_NODE, LCD_ROW_DET, CHAR_DOT);
        }
        else
        {
            disp.printHexChAt  (LCD_COL_NODE,  LCD_ROW_DET, aNode);
        }
    }


    /** Display the lock detail.
     */
    void displayLockDetail(bool aHi)
    {
        disp.clearRow(LCD_COL_START, LCD_ROW_DET);
        disp.printProgStrAt(LCD_COL_START, LCD_ROW_DET, aHi ? M_HI : M_LO);
        displayLockEdit(aHi, 0);
    }


    /** Output confirmation message.
     *  Wait for response.
     */
    bool confirm()
    {
        disp.printProgStrAt(LCD_COL_START, LCD_ROW_BOT, M_CONFIRM);
        return waitForConfirm(M_CONFIRMED);
    }


    /** Output cancellation message.
     *  Wait for response.
     */
    bool cancel()
    {
        disp.printProgStrAt(LCD_COL_START, LCD_ROW_BOT, M_CANCEL);
        return waitForConfirm(M_CANCELLED);
    }


    /** Wait for confirmation of confirm/cancel.
     *  Return true if confirmed.
     */
    bool waitForConfirm(PGM_P aMessagePtr)
    {
        uint8_t button = buttons.waitForButtonPress();

        if (button == BUTTON_SELECT)
        {
            disp.printProgStrAt(LCD_COL_START, LCD_ROW_BOT, aMessagePtr, LCD_COLS);
        }
        else
        {
            disp.printProgStrAt(LCD_COL_START, LCD_ROW_BOT, M_ABANDONED, LCD_COLS);
        }

        buttons.waitForButtonRelease();
        disp.clearRow(LCD_COL_START, LCD_ROW_BOT);

        return button == BUTTON_SELECT;
    }


    /** Identify all the configured outputs in turn.
     */
    void identOutputs()
    {
        bool       interrupted = false;
        uint8_t       button      = BUTTON_NONE;
        unsigned long interval    = 0L;           // Interval between changes of output.
        unsigned long finishAt    = 0L;           // Time to finish output's ident.

        buttons.waitForButtonRelease();
        if (outputNodes == 0)
        {
            disp.printProgStrAt(LCD_COL_START, LCD_ROW_BOT, M_NO_OUTPUT);
            buttons.waitForButtonClick();
        }
        else
        {
            disp.clearRow(LCD_COLS - LCD_LEN_OPTION, LCD_ROW_TOP);
            disp.printProgStrAt(LCD_COLS - LCD_LEN_OPTION, LCD_ROW_TOP, M_IDENT, LCD_LEN_OPTION);
            disp.clearRow(LCD_COL_START, LCD_ROW_DET);

            // Test all the nodes in turn.
            for (uint8_t node = 0; (node < OUTPUT_NODE_MAX) && !interrupted; node++)
            {
                if (isOutputNodePresent(node))
                {
                    // Test all the pins in turn.
                    for (int pin = 0; pin < OUTPUT_PIN_MAX && !interrupted; ) // pin++)
                    {
                        readOutput(node, pin);

                        // Pin is active, test it.
                        if (outputDef.getType() != OUTPUT_TYPE_NONE)
                        {
                            disp.printProgStrAt(LCD_COL_START, LCD_ROW_DET, M_OUTPUT_TYPES[outputDef.getType()], LCD_LEN_OPTION);
                            disp.printHexChAt(LCD_COL_NODE, LCD_ROW_DET, node);
                            disp.printHexChAt(LCD_COL_PIN,  LCD_ROW_DET, pin);

                            // Set parameters for test.
                            finishAt = millis() + DELAY_READ * 2;       // Time to see 2 states.
                            if (outputDef.isServo())
                            {
                                interval = DELAY_READ;
                            }
                            else
                            {
                                interval = DELAY_BLINK;
                            }

                            // Run the test for a while.
                            while (   (millis() < finishAt)
                                   && (button == BUTTON_NONE))
                            {
                                outputDef.setState(!outputDef.getState());
                                writeOutput();
                                button = delayFor(interval);

                                outputDef.setState(!outputDef.getState());
                                writeOutput();
                                if (button == BUTTON_NONE)
                                {
                                    button = delayFor(interval);
                                }
                            }
                            resetOutput();
                        }

                        // If button pressed, handle the interuption.
                        if (button != BUTTON_NONE)
                        {
                            int adjust = 0;

                            switch (button)
                            {
                                case BUTTON_NONE:   break;

                                case BUTTON_UP:     adjust += 2;    // Use +1 to compensate for the -1 that the code below will do.
                                                    [[fallthrough]];
                                case BUTTON_DOWN:   adjust -= 1;
                                                    node = nextNode(node, adjust, false, true);
                                                    pin = 0;
                                                    break;

                                case BUTTON_SELECT: interrupted = true;
                                                    break;

                                case BUTTON_LEFT:   for (pin -= 1; pin > 0; pin--)
                                                    {
                                                        readOutput(node, pin);
                                                        if (outputDef.getType() != OUTPUT_TYPE_NONE)
                                                        {
                                                            break;
                                                        }
                                                    }
                                                    break;

                                case BUTTON_RIGHT:  pin += 1;
                                                    break;
                            }

                            disp.printProgStrAt(LCD_COL_START, LCD_ROW_DET, M_INTERRUPT);
                            buttons.waitForButtonRelease();
                            button = BUTTON_NONE;
                            disp.clearRow(LCD_COL_START, LCD_ROW_DET);
                        }
                        else
                        {
                            pin += 1;
                        }
                    }
                }
            }
        }

        // Ensure original output is loaded.
        readOutput(outNode, outPin);
    }


    /** Delay for an interval,
     *  or until a button is pressed.
     *  Return the button pressed (if any).
     */
    uint8_t delayFor(unsigned long aInterval)
    {
        uint8_t       button = BUTTON_NONE;
        unsigned long endAt  = millis() + aInterval;

        while (   ((button = buttons.readButton()) == BUTTON_NONE)
               && (millis() < endAt))
        {
            delay(DELAY_BUTTON_WAIT);
        }

        return button;
    }


    /** Find the next (inUse) node in the given direction.
     *  Or not inUse if flag indicates such.
     *  Start at aStart and increment/decrement by aAdjust (ie search up or down).
     *  aIsInput indicates if looking for an Input or an Output node number.
     */
    uint8_t nextNode(uint8_t aStart, int aAdjust, bool aIsInput, bool aInUse)
    {
        uint8_t next = aStart & (aIsInput ? INPUT_NODE_MASK : OUTPUT_NODE_MASK);

        for (uint8_t ind = 0; ind < (aIsInput ? INPUT_NODE_MAX : OUTPUT_NODE_MAX); ind++)
        {
            next = (next + aAdjust) & (aIsInput ? INPUT_NODE_MASK : OUTPUT_NODE_MASK);

            if (   (aIsInput)
                && (aInUse == isInputNodePresent(next)))
            {
                break;
            }
            else if (   (!aIsInput)
                     && (aInUse == isOutputNodePresent(next)))
            {
                break;
            }
        }

        // If there are no inputs, move to next one anyway
        if (   (aIsInput)
            && (next   == aStart)                           // Didn't find a suitable input.
            && (aInUse != isInputNodePresent(next)))        // And this node isn't correct either.
        {
            next = (next + aAdjust) & INPUT_NODE_MASK;
        }

        return next;
    }


    /** Operate the current input to test if it works.
     */
    void testInput()
    {
        bool currentState = false;

        readOutput(inputDef.getOutput(inputDef.getFirstOutput()));
        currentState = outputDef.getState();

        controller.processInputOutputs(!currentState);
        buttons.waitForButtonRelease();
        controller.processInputOutputs(currentState);
    }


    /** Operate the Input's specified Output to test if it works.
     */
    void testInputOutput(uint8_t aIndex)
    {
        bool currentState = false;

        readOutput(inputDef.getOutput(aIndex));
        currentState = outputDef.getState();
        controller.processInputOutput(aIndex, !currentState, 0);
        buttons.waitForButtonRelease();
        controller.processInputOutput(aIndex,  currentState, 0);
    }


    /** Test the current Configuration.
     *  Most outputs change state and then change back again.
     *  Flashers always go Hi first, then back to current state.
     */
    void testOutput()
    {
        if (outputDef.isFlasher())
        {
            writeOutputState(outputNode, outputPin, true,  0);
            buttons.waitForButtonRelease();
            writeOutput();
        }
        else
        {
            writeOutputState(outputNode, outputPin, !outputDef.getState(), 0);
            buttons.waitForButtonRelease();
            writeOutputState(outputNode, outputPin,  outputDef.getState(), 0);
        }
    }


    /** Test a specific output (not the current one).
     *  Can't tell if it's a flasher, so just move to alternate state and back.
     */
    void testOutput(uint8_t aNode, uint8_t aPin)
    {
        writeOutputState(aNode, aPin, !getOutputState(aNode, aPin), 0);
        buttons.waitForButtonRelease();
        writeOutputState(aNode, aPin,  getOutputState(aNode, aPin), 0);
    }


    /** Mark a variable with field markers.
     */
    void markField(int aCol, int aRow, int aLen, bool aShow)
    {
        if (aCol > 0)
        {
            disp.printChAt(aCol - 1, aRow, aShow ? CHAR_RIGHT : CHAR_SPACE);
        }
        disp.printChAt(aCol + aLen, aRow, aShow ? CHAR_LEFT : CHAR_SPACE);
    }


    /** Report a failure during configuration
     */
    void configFail(PGM_P aMessage, int aValue)
    {
        systemFail(aMessage, aValue);
        buttons.waitForButtonClick();
    }
};


/** A singleton instance of the Configure class.
 */
Configure configure;


/** Helper for use in callback to displayScannedInput().
 */
void displayScannedInput(uint8_t aNode, uint8_t aPin)
{
    configure.displaySelectedInput(aNode, aPin);
}


#endif
