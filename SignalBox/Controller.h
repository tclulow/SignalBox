/** Controller
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

#ifndef Controller_h
#define Controller_h


/** Controller module.
 *  Detects inputs and actions their outputs.
 */
class Controller
{
    private:

    long     tickHardwareScan = 0L;         // Time for next scan for hardware.
    long     tickInputScan    = 0L;         // Time for next scan of input switches.
    long     tickHeartBeat    = 0L;         // Time for next heartbeat.

    long     displayTimeout   = 1L;         // Timeout for the display when important messages are showing.
                                            // Using 1 forces an initial redisplay unless a start-up process has requested a delay.
    long     timeoutInterlock = 0L;         // Timeout for interlock warning to be reset.
    long     timeoutBuzzer    = 0L;         // Time at which buzzer2 sound should be made.

    uint16_t inputState[INPUT_NODE_MAX];    // Current state of inputs.


    public:
    
    /** Announce ourselves.
     */
    void announce()
    {
        disp.clear();
        disp.printProgStrAt(LCD_COL_START,             LCD_ROW_TOP, M_SOFTWARE);
        disp.printProgStrAt(-strlen_P(M_VERSION),      LCD_ROW_TOP, M_VERSION);
        disp.printProgStrAt(-strlen_P(M_VERSION_DATE), LCD_ROW_DET, M_VERSION_DATE);
    }


    /** Run all update tasks.
     *  Turn off interlock warning pin if it's set.
     *  Play second buzzer sound if it's due.
     *  Show the heartbeat (unless there's a display timeout pending).
     */
    void update()
    {
        long now = millis();

        if (INTERLOCK_WARNING_PIN > 0)
        {
            // Check for interlock warning expired
            if (   (timeoutInterlock > 0)
                && (timeoutInterlock < now))
            {
                timeoutInterlock = 0L;
                digitalWrite(INTERLOCK_WARNING_PIN, LOW);
            }
        }

        if (INTERLOCK_BUZZER_PIN > 0)
        {
            if (   (timeoutBuzzer > 0)
                && (timeoutBuzzer < now))
            {
                timeoutBuzzer = 0;
                tone(INTERLOCK_BUZZER_PIN, INTERLOCK_BUZZER_FREQ2, INTERLOCK_BUZZER_TIME2);
            }
        }

        // Rescan for new hardware
        if (now > tickHardwareScan)
        {
            tickHardwareScan = now + STEP_HARDWARE_SCAN;
            scanInputHardware();
            scanOutputHardware();
        }
    
        // Process any inputs
        if (now > tickInputScan)
        {
            tickInputScan = now + STEP_INPUT_SCAN;
            // scanOutputs();
            scanInputs(NULL);
        }
    
        // If display timeout has expired, clear it.
        if (   (displayTimeout > 0)
            && (now > displayTimeout))
        {
            displayTimeout = 0L;
            announce();
        }

        // Show heartbeat if no display timeout is pending.
        if (   (displayTimeout == 0)
            && (now > tickHeartBeat))
        {
            int hours = (now)                                                       / MILLIS_PER_HOUR;
            int mins  = (now - MILLIS_PER_HOUR * hours)                             / MILLIS_PER_MINUTE;
            int secs  = (now - MILLIS_PER_HOUR * hours  - MILLIS_PER_MINUTE * mins) / MILLIS_PER_SECOND;

            tickHeartBeat = now + STEP_HEARTBEAT;
            disp.setCursor(LCD_COL_START, LCD_ROW_DET);
            if (hours > 0)
            {
                disp.printDec(hours, 1, CHAR_ZERO);
                disp.printCh(CHAR_COLON);
            }
            disp.printDec(mins, 2, CHAR_ZERO);
            disp.printCh(CHAR_COLON);
            disp.printDec(secs, 2, CHAR_ZERO);
        }
    }


    /** Gets the current state of all the inputs on one input node.
     */
    uint16_t getInputState(uint8_t aInputNode)
    {
        return inputState[aInputNode];
    }


    /** Scan for Input and Output nodes.
     */
    void scanHardware()
    {
        buttons.waitForButtonRelease();

        // Scan for Input nodes.
        scanInputHardware();
        dispInputHardware();

        // Scan for Output nodes.
        scanOutputHardware();
        dispOutputHardware();

        // Report absence of hardware.
        if (   (inputNodes  == 0)
            || (outputNodes == 0))
        {
            uint8_t row = LCD_ROW_TOP;
            disp.clear();
            if (inputNodes == 0)
            {
                disp.printProgStrAt(LCD_COL_START, row++, M_NO_INPUT);
            }
            if (outputNodes == 0)
            {
                disp.printProgStrAt(LCD_COL_START, row++, M_NO_OUTPUT);
            }
            buttons.waitForButtonClick();
        }
    }


    /** Scan all the Inputs.
     *  Parameter indicates if Configuration is in progress.
     */
    void scanInputs(void aCallback(uint8_t, uint8_t))
    {
        // Scan all the nodes.
        for (uint8_t node = 0; node < INPUT_NODE_MAX; node++)
        {
            if (isInputNodePresent(node))
            {
                // Read current state of pins and if there's been a change.
                uint16_t pins = readInputNode(node);
                if (pins != inputState[node])
                {
                    // Process all the changed pins.
                    for (uint16_t pin = 0, mask = 1; pin < INPUT_PIN_MAX; pin++, mask <<= 1)
                    {
                        uint16_t state = pins & mask;
                        if (state != (inputState[node] & mask))
                        {
                            // Ensure Input is loaded and handle the action.
                            inputMgr.loadInput(node, pin);
                            if (aCallback)
                            {
                                aCallback(node, pin);           // Notify the caller via the callback function.
                            }
                            else
                            {
                                processInput(state != 0);       // Normal processing, action the input.
                            }
                        }
                    }

                    // Record new input states.
                    inputState[node] = pins;
                }
            }
            else
            {
                inputState[node] = 0xffff;
            }
        }
    }


    /** Process the changed input for the specified Input.
     *  aState is the state of the input switch.
     */
    void processInput(uint8_t aNode, uint8_t aPin, bool aState)
    {
        inputMgr.loadInput(aNode, aPin);
        processInput(aState);
    }


    /** Process all the Input's Outputs.
     */
    void processInputOutputs(bool aNewState)
    {
        uint8_t endDelay = 0;

        // Process all the Input's outputs.
        // In reverse order if setting lo.
        if (aNewState)
        {
            for (int index = 0; index < INPUT_OUTPUT_MAX; index++)
            {
                endDelay = processInputOutput(index, aNewState, endDelay);
            }
        }
        else
        {
            for (int index = INPUT_OUTPUT_MAX - 1; index >= 0; index--)
            {
                endDelay = processInputOutput(index, aNewState, endDelay);
            }
        }
    }


    /** Process an Input's n'th Output, setting it to the given state.
     *  Accumulate delay as we go.
     *  Returns the accumulated delay.
     */
    uint8_t processInputOutput(uint8_t aIndex, uint8_t aState, uint8_t aDelay)
    {
        uint8_t endDelay = aDelay;

        uint8_t outNode  = inputDef.getOutputNode(aIndex);
        uint8_t outPin   = inputDef.getOutputPin(aIndex);

        // Process the Input's Outputs.
        if (inputDef.isDelay(aIndex))
        {
            endDelay += inputDef.getOutputPin(aIndex);          // Accumulate the delay.
        }
        else
        {
            if (   (systemMgr.isReportEnabled(REPORT_PAUSE)))
//                || (   (isReportEnabled(REPORT_SHORT))
//                    && (inputDef.getOutputCount() <= 1)))
            {
                readOutput(inputDef.getOutput(aIndex));
                disp.printProgStrAt(LCD_COL_START,  LCD_ROW_BOT, M_OUTPUT_TYPES[outputDef.getType()], LCD_COLS);
                disp.printHexChAt(LCD_COL_OUTPUT_PARAM, LCD_ROW_BOT, outNode);
                disp.printHexCh(outPin);
                disp.printCh(CHAR_SPACE);
                disp.printHexCh(outputDef.getPace());
                disp.printCh(CHAR_SPACE);
                disp.printCh(outputDef.getResetCh());
                disp.setCursor(-2, LCD_ROW_BOT);
                disp.printDec(aDelay, 2, CHAR_SPACE);

                if (systemMgr.isReportEnabled(REPORT_PAUSE))
                {
                    reportPause();
                }
            }
            else if (systemMgr.isReportEnabled(REPORT_SHORT))
            {
                disp.printCh(CHAR_SPACE);
                disp.printHexCh(outNode);
                disp.printHexCh(outPin);
                setDisplayTimeout(systemMgr.getReportDelay());
            }

            if (isDebug(DEBUG_BRIEF))
            {
                Serial.print(millis());
                Serial.print(CHAR_TAB);
                Serial.print(PGMT(M_OUTPUT));
                Serial.print(PGMT(M_DEBUG_STATE));
                Serial.print(PGMT(aState ? M_HI : M_LO));
                Serial.print(PGMT(M_DEBUG_NODE));
                Serial.print(outNode, HEX);
                Serial.print(PGMT(M_DEBUG_PIN));
                Serial.print(outPin,  HEX);
                Serial.print(PGMT(M_DEBUG_DELAY_TO));
                Serial.print(aDelay, HEX);
                Serial.println();
            }

            // Action the Output state change.
            writeOutputState(outNode, outPin, aState, endDelay);

            // Recover all states from output module (in case a double-LED has changed one).
            readOutputStates(outNode);
            // setOutputState(outNode, outPin, aState);
        }

        return endDelay;
    }


    /** Set the display timeout for an important message.
     */
    void setDisplayTimeout(long aTimeout)
    {
        displayTimeout = millis() + aTimeout;
    }


    /** Sends the current debug level to all the connected outputs.
     */
    void sendDebugLevel()
    {
        for (uint8_t node = 0; node < OUTPUT_NODE_MAX; node++)
        {
            if (isOutputNodePresent(node))
            {
                i2cComms.sendShort(I2C_OUTPUT_BASE_ID + node, COMMS_CMD_DEBUG | (systemMgr.getDebugLevel() & COMMS_OPTION_MASK));

                if (isDebug(DEBUG_BRIEF))
                {
                    Serial.print(millis());
                    Serial.print(CHAR_TAB);
                    Serial.print(PGMT(M_DEBUG_DEBUG));
                    Serial.print(PGMT(M_DEBUG_NODE));
                    Serial.print(HEX_CHARS[node]);
                    Serial.print(CHAR_SPACE);
                    Serial.print(PGMT(M_DEBUG_PROMPTS[systemMgr.getDebugLevel()]));
                    Serial.println();
                }
            }
        }
    }


    private:

    /** Process the changed input for the current Input.
     *  aState is the state of the input switch.
     */
    void processInput(bool aState)
    {
        bool newState = aState;
        uint8_t first    = 0;
        uint8_t node     = (inputNumber >> INPUT_NODE_SHIFT) & INPUT_NODE_MASK;
        uint8_t pin      = (inputNumber                    ) & INPUT_PIN_MASK;

        // Process all input state changes for Toggles, only state going low for other Input types.
        if (   (!aState)
            || (inputType == INPUT_TYPE_TOGGLE))
        {
            // Set desired new state based on Input's type/state and Output's state.
            switch (inputType)
            {
                case INPUT_TYPE_TOGGLE: // newState = aState;      // Set state to that of the Toggle.
                                        break;

                case INPUT_TYPE_ON_OFF: // Find first real output (not a delay) to determine new state.
                                        first = inputDef.getFirstOutput();
                                        newState = !getOutputState(inputDef.getOutputNode(first), inputDef.getOutputPin(first));
                                        break;

                case INPUT_TYPE_ON:     newState = true;            // Set the state.
                                        break;

                case INPUT_TYPE_OFF:    newState = false;           // Clear the state.
                                        break;
            }

            // Report state change if reporting enabled.
            if (systemMgr.isReportEnabled(REPORT_SHORT))
            {
                disp.clearBottomRows();
                disp.printProgStrAt(LCD_COL_START, LCD_ROW_EDT, M_INPUT_TYPES[inputType & INPUT_TYPE_MASK]);
                disp.printProgStrAt(LCD_COL_STATE, LCD_ROW_EDT, (newState ? M_HI : M_LO));
                disp.printHexChAt(LCD_COL_NODE,    LCD_ROW_EDT, node);
                disp.printHexChAt(LCD_COL_PIN,     LCD_ROW_EDT, pin);
                disp.setCursor(LCD_COL_START + 1,  LCD_ROW_BOT);
                setDisplayTimeout(systemMgr.getReportDelay());
            }

            if (isDebug(DEBUG_BRIEF))
            {
                Serial.print(millis());
                Serial.print(CHAR_TAB);
                Serial.print(PGMT(M_INPUT_TYPES[inputType & INPUT_TYPE_MASK]));
                Serial.print(PGMT(M_DEBUG_STATE));
                Serial.print(PGMT(newState ? M_HI : M_LO));
                Serial.print(PGMT(M_DEBUG_NODE));
                Serial.print(node, HEX);
                Serial.print(PGMT(M_DEBUG_PIN));
                Serial.print(pin,  HEX);
                Serial.println();
            }

            // If not locked, process the Input's Outputs.
            if (!isLocked(newState))
            {
                i2cComms.sendGateway((newState ? COMMS_CMD_INP_HI : COMMS_CMD_INP_LO) | pin, node, -1);
                processInputOutputs(newState);
            }

            if (isDebug(DEBUG_BRIEF))
            {
                Serial.println();
            }
        }
    }


    /** Scan for attached Input hardware.
     */
    void scanInputHardware()
    {
        for (uint8_t node = 0; node < INPUT_NODE_MAX; node++)
        {
            if (!isInputNodePresent(node))
            {
                if (disp.getLcdId() != (I2C_INPUT_BASE_ID + node))
                {
                    // Send message to the Input and see if it responds.
                    if (i2cComms.exists(I2C_INPUT_BASE_ID + node))
                    {
                        setInputNodePresent(node, true);

                        // Configure MCP for input.
                        for (uint8_t command = 0; command < INPUT_COMMANDS_LEN; command++)
                        {
                            i2cComms.sendData(I2C_INPUT_BASE_ID + node, INPUT_COMMANDS[command], MCP_ALL_HIGH, -1);
                        }

                        // Record current switch state
                        inputState[node] = readInputNode(node);
                    }
                    else
                    {
                        inputState[node] = 0xffff;
                    }
                }
            }
        }
    }


    /** Display the Input nodes present.
     *  Show either the Input module's ID, or a dot character.
     */
    void dispInputHardware()
    {
        disp.clear();
        disp.printProgStrAt(LCD_COL_START, LCD_ROW_TOP, M_NODES);
        disp.printProgStrAt(LCD_COL_START, LCD_ROW_DET, M_INPUT, LCD_LEN_OPTION);
        disp.setCursor(-8, LCD_ROW_TOP);

        for (uint8_t node = 0; node < INPUT_NODE_MAX; node++)
        {
            if (node == 8)          // If there are more than 8 inputs, show them on the second row.
            {
                disp.setCursor(-8, LCD_ROW_DET);
            }
            
            if (disp.getLcdId() == (I2C_INPUT_BASE_ID + node))
            {
                disp.printCh(CHAR_HASH);
            }
            else
            {
                if (isInputNodePresent(node))
                {
                    disp.printHexCh(node);
                }
                else
                {
                    disp.printCh(CHAR_DOT);
                }
            }
        }

        buttons.waitForButtonClick();
    }


    /** Scan for attached Output hardware.
     */
    void scanOutputHardware()
    {
        for (uint8_t node = 0; node < OUTPUT_NODE_MAX; node++)
        {
            if (!isOutputNodePresent(node))
            {
                readOutputStates(node);     // Automatically marked as present if it responds.
            }
        }
    }


    /** Display the Output nodes present.
     *  Show either the Output module's ID, or a dot character.
     */
    void dispOutputHardware()
    {
        disp.printProgStrAt(LCD_COL_START, LCD_ROW_DET, M_OUTPUT, LCD_LEN_OPTION);
        disp.setCursor(-OUTPUT_NODE_HALF, LCD_ROW_EDT);

        for (uint8_t node = 0; node < OUTPUT_NODE_MAX; node++)
        {
            if (node == OUTPUT_NODE_HALF)
            {
                disp.setCursor(-OUTPUT_NODE_HALF, LCD_ROW_BOT);
            }

            if (isOutputNodePresent(node))
            {
                disp.printHexCh(node);
            }
            else
            {
                disp.printCh(CHAR_DOT);
            }
        }

        buttons.waitForButtonClick();
    }


    /** Record a node input error.
     */
    void recordInputError(uint8_t aNode)
    {
        systemFail(M_INPUT, aNode);
        setInputNodePresent(aNode, false);
    }


    /** Read the pins of a InputNode.
     *  Return the state of the pins, 16 bits, both ports.
     *  Return current state if there's a communication error,
     *  this will prevent any actions being performed.
     */
    uint16_t readInputNode(uint8_t aNode)
    {
        uint16_t value = 0;

        if (   (i2cComms.sendShort(I2C_INPUT_BASE_ID + aNode, MCP_GPIOA))
            || (!i2cComms.requestPacket(I2C_INPUT_BASE_ID + aNode, INPUT_STATE_LEN)))
        {
            recordInputError(aNode);
            value = inputState[aNode];  // Pretend no change if comms error.
        }
        else
        {
            value = i2cComms.readWord();
        }

        return value;
    }


    /** Check if any of the Input's Outputs are locked.
     */
    bool isLocked(bool aNewState)
    {
        // Check all the Input's Outputs.
        for (uint8_t inpIndex = 0; inpIndex < INPUT_OUTPUT_MAX; inpIndex++)
        {
            // Process all definitions that aren't "delay"s.
            if (!inputDef.isDelay(inpIndex))
            {
                readOutput(inputDef.getOutputNode(inpIndex), inputDef.getOutputPin(inpIndex));

                // Check all the Output's locks.
                for (uint8_t outIndex = 0; outIndex < OUTPUT_LOCK_MAX; outIndex++)
                {
                    // If there's an active lock
                    if (outputDef.isLock(aNewState, outIndex))
                    {
                        // And the state change is prohibited.
                        bool state = getOutputState(outputDef.getLockNode(aNewState, outIndex), outputDef.getLockPin(aNewState, outIndex));
                        if (outputDef.getLockState(aNewState, outIndex) == state)
                        {
                            if (systemMgr.isReportEnabled(REPORT_SHORT))
                            {
                                disp.printProgStrAt(LCD_COL_START, LCD_ROW_BOT, M_LOCK, LCD_LEN_OPTION);
                                disp.printCh(aNewState ? CHAR_HI : CHAR_LO);
                                disp.printHexCh(outputNode);
                                disp.printHexCh(outputPin);
                                disp.printProgStr(M_VS);
                                disp.printCh(state ? CHAR_HI : CHAR_LO);
                                disp.printHexCh(outputDef.getLockNode(aNewState, outIndex));
                                disp.printHexCh(outputDef.getLockPin (aNewState, outIndex));
                                setDisplayTimeout(systemMgr.getReportDelay());
                            }

                            if (isDebug(DEBUG_BRIEF))
                            {
                                outputDef.printDef(M_LOCK, outputNode, outputPin);
                                readOutput(outputDef.getLockNode(aNewState, outIndex), outputDef.getLockPin(aNewState, outIndex));
                                outputDef.printDef(M_VS, outputNode, outputPin);
                            }

                            if (INTERLOCK_WARNING_PIN)
                            {
                                pinMode(INTERLOCK_WARNING_PIN, OUTPUT);
                                digitalWrite(INTERLOCK_WARNING_PIN, HIGH);
                                timeoutInterlock = millis() + INTERLOCK_WARNING_TIME;
                            }

                            if (INTERLOCK_BUZZER_PIN)
                            {
                                pinMode(INTERLOCK_BUZZER_PIN, OUTPUT);
                                tone(INTERLOCK_BUZZER_PIN, INTERLOCK_BUZZER_FREQ1, INTERLOCK_BUZZER_TIME1);
                                timeoutBuzzer = millis() + INTERLOCK_BUZZER_TIME1;
                            }
                            
                            return true;            // A lock exists.
                        }
                    }
                }
            }
        }

        return false;
    }
};


// Singleton instance of the class
Controller controller;


#endif
