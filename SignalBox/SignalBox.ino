/** SignalBox.
 *
 *
 *  (c)Copyright Tony Clulow  2021	tony.clulow@pentadtech.com
 *
 *  This work is licensed under the:
 *      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 *      http://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 *  For commercial use, please contact the original copyright holder(s) to agree licensing terms.
 */

#define MASTER true         // The master (Uno) device.


#include "All.h"


#define COMMAND_BUFFER_LEN   8                  // Serial command buffer length
char    commandBuffer[COMMAND_BUFFER_LEN + 1];  // Buffer to read characters with null terminator on the end.
uint8_t commandLen = 0;                         // Length of command.


// Ticking
long    now              = 0L;      // Current time in millisecs.
long    tickHardwareScan = 0L;      // Time for next scan for hardware.
long    tickInputScan    = 0L;      // Time for next scan of input switches.
long    tickGateway      = 0L;      // Time for next gateway request.
long    tickHeartBeat    = 0L;      // Time of last heartbeat.

long    timeoutInterlock = 0L;      // Timeout for interlock warning to be reset.
long    displayTimeout   = 1L;      // Timeout for the display when important messages are showing.
                                    // Using 1 forces an initial redisplay unless a start-up process has requested a delay.


/** Announce ourselves.
 */ 
void announce()
{
    disp.clear();
    disp.printProgStrAt(LCD_COL_START,             LCD_ROW_TOP, M_SOFTWARE);
    disp.printProgStrAt(-strlen_P(M_VERSION),      LCD_ROW_TOP, M_VERSION);
    disp.printProgStrAt(-strlen_P(M_VERSION_DATE), LCD_ROW_DET, M_VERSION_DATE);
}


/** Set the display timeout for an important message.
 */
void setDisplayTimeout(long aTimeout)
{
    displayTimeout = millis() + aTimeout;
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
                    currentSwitchState[node] = readInputNode(node);
                }
                else
                {
                    currentSwitchState[node] = 0xffff;
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
    disp.setCursor(-INPUT_NODE_MAX, LCD_ROW_TOP);

    for (uint8_t node = 0; node < INPUT_NODE_MAX; node++)
    {
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
    
    waitForButtonClick();
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
    
    waitForButtonClick();
}


/** Scan for Input and Output nodes.
 */
void scanHardware()
{
    uint8_t id = 0;
    waitForButtonRelease();

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
        waitForButtonClick();
    }
}


/** Software hasn't been run before.
 */
void firstRun()
{
    // Initialise SystemData.
    systemData.magic   = MAGIC_NUMBER;

    // Calibrate the LCD buttons.
    if (hasLcdShield)
    {
        calibrateButtons();
    }

    // Decide if EzyBus conversion required.
    if (ezyBusDetected())
    {
        disp.clear();
        disp.printProgStrAt(LCD_COL_START, LCD_ROW_TOP, M_EZY_FOUND);
        disp.printProgStrAt(LCD_COL_START, LCD_ROW_DET, M_EZY_UPDATE);

        ezyBusClear();
        if (waitForButtonPress() == BUTTON_SELECT)
        {
            ezyBusConvert();
            waitForButtonClick();            
            defaultInputs(INPUT_TYPE_TOGGLE);
        }
        else
        {
            defaultInputs(INPUT_TYPE_ON_OFF);
        }
    }
    else
    {
        defaultInputs(INPUT_TYPE_ON_OFF);
    }

    // Save all data to EEPROM.
    saveSystemData();
    
    waitForButtonClick();
}


/** Set the default initial setup
 *  1-1 mapping, inputs to outputs.
 */
void defaultInputs(uint8_t aInputType)
{
    disp.clear();
    disp.printProgStrAt(LCD_COL_START, LCD_ROW_TOP, M_INITIALISING);
    disp.setCursor(LCD_COL_START, LCD_ROW_DET);

    inputNumber = 0;
    inputType   = aInputType;
    
    for (uint8_t node = 0; node < INPUT_NODE_MAX; node++)
    {
        disp.printHexCh(node);

        for (uint8_t pin = 0; pin < INPUT_PIN_MAX; pin++)
        {
            // Create an input.
            inputDef.setOutput(0, inputNumber);     // Map 1-1 inputs to outputs.
            inputDef.setDelay(0, false);
            for (uint8_t index = 1; index < INPUT_OUTPUT_MAX; index++)
            {
                inputDef.setOutput(index, 0);       // Zero-length delay.
                inputDef.setDelay(index, true);
            }

            saveInput();
            inputNumber += 1;       // Input numbers map nicely to OutputNumbers.
        }
    }
}


/** Convert EzyBus configuration.
 *  One-one mapping with EzyBus modules, and their inputs.
 */
void ezyBusConvert()
{
    int     ezyBus = 0;
    uint8_t value  = 0;
    
    disp.clearRow(LCD_COL_START, LCD_ROW_DET);
    disp.clearBottomRows();
    disp.printProgStrAt(LCD_COL_START, LCD_ROW_EDT, M_EZY_UPDATING, LCD_COLS);
    disp.setCursor(LCD_COL_START, LCD_ROW_BOT);

    for (outputNode = 0; outputNode < EZY_NODE_MAX; outputNode++)
    {
        disp.printHexCh(outputNode);

        for (outputPin = 0; outputPin < OUTPUT_PIN_MAX; outputPin++)
        {
            // Create an Output that reflects the Ezybus one.
            outputDef.setState(false);
            outputDef.setType((EEPROM.read(ezyBus++) + 1) & OUTPUT_TYPE_MASK);                  // Output types are 1 greater then those of Ezybus.
            outputDef.setLo(EEPROM.read(ezyBus++));        
            outputDef.setHi(EEPROM.read(ezyBus++));        
            outputDef.setPace((EEPROM.read(ezyBus++) >> EZY_SPEED_SHIFT) & OUTPUT_PACE_MASK);   // Convert Ezybus pace.
            outputDef.setReset(OUTPUT_DEFAULT_RESET);

            writeOutput();
            writeSaveOutput();
        }
    }
}


/** Sends the current debug level to all the connected outputs.
 */
void sendDebugLevel()
{
    for (uint8_t node = 0; node < OUTPUT_NODE_MAX; node++)
    {
        if (isOutputNodePresent(node))
        {
            i2cComms.sendShort(I2C_OUTPUT_BASE_ID + node, COMMS_CMD_DEBUG | (getDebug() & COMMS_OPTION_MASK));

            if (isDebug(DEBUG_BRIEF))
            {
                Serial.print(millis());
                Serial.print(CHAR_TAB);
                Serial.print(PGMT(M_DEBUG_DEBUG));
                Serial.print(PGMT(M_DEBUG_NODE));
                Serial.print(HEX_CHARS[node]);
                Serial.print(CHAR_SPACE);
                Serial.print(PGMT(M_DEBUG_PROMPTS[getDebug()]));
                Serial.println();
            }
        }
    }
}


/** Scan all the Inputs.
 *  Parameter indicates if Configuration is in progress.
 */
void scanInputs(boolean aConfiguration)
{ 
    // Scan all the nodes. 
    for (uint8_t node = 0; node < INPUT_NODE_MAX; node++)
    {
        if (isInputNodePresent(node))                                        
        {
            // Read current state of pins and if there's been a change.
            uint16_t pins = readInputNode(node);
            if (pins != currentSwitchState[node])
            {
                // Process all the changed pins.
                for (uint16_t pin = 0, mask = 1; pin < INPUT_PIN_MAX; pin++, mask <<= 1)
                {
                    uint16_t state = pins & mask;
                    if (state != (currentSwitchState[node] & mask))
                    {
                        // Ensure Input is loaded and handle the action.
                        loadInput(node, pin);
                        if (aConfiguration)
                        {
                            configure.displaySelectedInput(node, pin);  // Configuring is in progress, display the input actioned.
                        }
                        else
                        {
                            processInput(state != 0);                   // Normal processing, action the input.
                        }
                    }
                }
            
                // Record new input states.
                currentSwitchState[node] = pins;
            }
        }
    }
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
        value = currentSwitchState[aNode];  // Pretend no change if comms error.
    }
    else
    {
        value = i2cComms.readWord();
    }

    return value;
}


/** Process the changed input.
 *  aState is the state of the input switch.
 */
void processInput(boolean aState)
{
    boolean newState = aState;
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
        if (isReportEnabled(REPORT_SHORT))
        {
            disp.clearBottomRows();
            disp.printProgStrAt(LCD_COL_START, LCD_ROW_EDT, M_INPUT_TYPES[inputType & INPUT_TYPE_MASK]);
            disp.printProgStrAt(LCD_COL_STATE, LCD_ROW_EDT, (newState ? M_HI : M_LO));
            disp.printHexChAt(LCD_COL_NODE,    LCD_ROW_EDT, node);
            disp.printHexChAt(LCD_COL_PIN,     LCD_ROW_EDT, pin);
            disp.setCursor(LCD_COL_START + 1,  LCD_ROW_BOT);
            setDisplayTimeout(getReportDelay());
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



/** Check if any of the Input's Outputs are locked.
 */
boolean isLocked(boolean aNewState)
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
                    boolean state = getOutputState(outputDef.getLockNode(aNewState, outIndex), outputDef.getLockPin(aNewState, outIndex));
                    if (outputDef.getLockState(aNewState, outIndex) == state)
                    {
                        if (isReportEnabled(REPORT_SHORT))
                        {
                            disp.printProgStrAt(LCD_COL_START, LCD_ROW_BOT, M_LOCK, LCD_LEN_OPTION);
                            disp.printCh(aNewState ? CHAR_HI : CHAR_LO);
                            disp.printHexCh(outputNode);
                            disp.printHexCh(outputPin);
                            disp.printProgStr(M_VS);
                            disp.printCh(state ? CHAR_HI : CHAR_LO);
                            disp.printHexCh(outputDef.getLockNode(aNewState, outIndex));
                            disp.printHexCh(outputDef.getLockPin (aNewState, outIndex));
                            setDisplayTimeout(getReportDelay());
                        }

                        if (isDebug(DEBUG_BRIEF))
                        {
                            outputDef.printDef(M_LOCK, outputNode, outputPin);
                            readOutput(outputDef.getLockNode(aNewState, outIndex), outputDef.getLockPin(aNewState, outIndex));
                            outputDef.printDef(M_VS, outputNode, outputPin);
                        }
                        
#if INTERLOCK_WARNING_PIN
                        pinMode(INTERLOCK_WARNING_PIN, OUTPUT);
                        digitalWrite(INTERLOCK_WARNING_PIN, HIGH);
                        timeoutInterlock = millis() + INTERLOCK_WARNING_TIME;
                        // Alternative using tone function (about 700+ bytes of extra code).
                        // tone(INTERLOCK_WARNING_PIN, INTERLOCK_WARNING_FREQ, INTERLOCK_WARNING_TIME);
#endif

                        return true;            // A lock exists.
                    }
                }
            }
        }
    }

    return false;
}


/** Process all the Input's Outputs.
 */
void processInputOutputs(boolean aNewState)
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
        if (   (isReportEnabled(REPORT_PAUSE)))
//            || (   (isReportEnabled(REPORT_SHORT))
//                && (inputDef.getOutputCount() <= 1)))
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
            
//            if (isReportEnabled(REPORT_PAUSE))
            {
                reportPause();
            }
        }
        else if (isReportEnabled(REPORT_SHORT))
        {
            disp.printCh(CHAR_SPACE);
            disp.printHexCh(outNode);
            disp.printHexCh(outPin);
            setDisplayTimeout(getReportDelay());
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


/** Process a received command.
 *  Using the contents of the commandBuffer:
 *      iNP - Action input for node N, pin P.
 *      lNP - Action output Lo for node N, pin P.
 *      hNP - Action output Hi for node N, pin P.
 *      oNP - Action output Hi/Lo (based on current state) for node N, pin P.
 */
void processCommand()
{
    boolean executed = false;
    uint8_t node     = 0;
    uint8_t pin      = 0;
    boolean state    = true;
    
    if (isDebug(DEBUG_BRIEF))
    {
      Serial.print(millis());
      Serial.print(CHAR_TAB);
      Serial.print(PGMT(M_INPUT));
      Serial.print(PGMT(M_DEBUG_COMMAND));
      Serial.println(commandBuffer);
    }

    // Expect three characters, command, nodeId, pinId
    if (strlen(commandBuffer) == 3)
    {
        node = charToHex(commandBuffer[1]);
        pin  = charToHex(commandBuffer[2]);
                      
        switch (commandBuffer[0] | 0x20)            // Command character converted to lower-case.
        {
            case 'i': if (   (node < INPUT_NODE_MAX)
                          && (pin  < INPUT_PIN_MAX))
                      {
                          loadInput(node, pin);
                          processInput(false);
                          executed = true;
                      }
                      break;

            case 'o': state = getOutputState(node, pin);
            case 'l': state = !state;
            case 'h': if (   (node < OUTPUT_NODE_MAX)
                          && (pin  < OUTPUT_PIN_MAX))
                      {
                          writeOutputState(node, pin, state, 0);
                          readOutputStates(node);                   // Recover states in case LED_4 has moved one.
                          executed = true;
                      }

            default:  break;
        }
    }

    // Report error if not executed.
    if (!executed)
    {
        if (isDebug(DEBUG_ERRORS))
        {
          Serial.print(millis());
          Serial.print(CHAR_TAB);
          Serial.print(PGMT(M_UNKNOWN));
          Serial.print(PGMT(M_DEBUG_COMMAND));
          Serial.println(commandBuffer);
        }

        if (isReportEnabled(REPORT_SHORT))
        {
            disp.clearRow(LCD_COL_START, LCD_ROW_BOT);
            disp.setCursor(LCD_COL_START, LCD_ROW_BOT);
            disp.printProgStr(M_UNKNOWN);
            disp.printCh(CHAR_SPACE);
            disp.printStr(commandBuffer);
            setDisplayTimeout(getReportDelay());
        }
    }        
}


/** See if there's a Gateway request.
 *  Process the request.
 *  Return true if work done.
 */
boolean gatewayRequest()
{
    boolean workDone = false;

    i2cComms.sendGateway(COMMS_CMD_SYSTEM | COMMS_SYS_GATEWAY, -1, -1);
    if (i2cComms.requestGateway())
    {
        uint8_t command = i2cComms.readByte();
        uint8_t node    = i2cComms.readByte();
        uint8_t option  = command & COMMS_OPTION_MASK;
        uint8_t pin     = option & OUTPUT_PIN_MASK;
        command = command & COMMS_COMMAND_MASK;

        switch (command)
        {
            case COMMS_CMD_SYSTEM: if (option == COMMS_SYS_OUT_STATES)
                                   {
                                       i2cComms.sendGateway(COMMS_CMD_SYSTEM | COMMS_SYS_OUT_STATES, getOutputStates(node), -1);
                                       workDone = true;
                                   }
                                   else
                                   {
                                       systemFail(M_GATEWAY, command | option);
                                   }
                                   break;

            case COMMS_CMD_NONE:   break;

            default:               systemFail(M_GATEWAY, command | option);
        }
    }
//    else
//    {
//        // Turn off Gateway if there's a comms error.
//        i2cComms.setGateway(0);
//    }

    i2cComms.readAll();

    return workDone;
}


//uint8_t * heapPtr, * stackPtr;
//
//void checkMem(const char* aMessage)
//{
//  stackPtr = (uint8_t *)malloc(4);  // use stackPtr temporarily
//  heapPtr = stackPtr;               // save value of heap pointer
//  free(stackPtr);                   // free up the memory again (sets stackPtr to 0)
//  stackPtr =  (uint8_t *)(SP);      // save value of stack pointer
//  Serial.print(aMessage);
//  Serial.print(" ");
//  Serial.print((int)heapPtr);
//  Serial.print(" ");
//  Serial.println((int)stackPtr);
//}

    
/** Setup the Arduino.
 */
void setup()
{
    // Start Serial IO first - needed if there's any debug output.
    Serial.begin(SERIAL_SPEED);

    // Detect presence of LCD shield using LCD_SHIELD_DETECT_PIN if necessary
#if ! LCD_SHIELD && LCD_SHIELD_DETECT_PIN
    pinMode(LCD_SHIELD_DETECT_PIN, INPUT_PULLUP);
    hasLcdShield = !digitalRead(LCD_SHIELD_DETECT_PIN);
#endif

    if (hasLcdShield)
    {
        disp.createLcdShield();

        // Initial announcement/splash message.
        announce();
        disp.printProgStrAt(LCD_COL_START, LCD_ROW_DET, M_INIT_I2C, LCD_LEN_STATUS);
    }
    
    // Initialise I2C.
    Wire.begin(I2C_CONTROLLER_ID);          // I2C network
    // Wire.setTimeout(25000L);             // Doesn't seem to have any effect.

#if LCD_I2C
    // Scan for I2C LCD.
    for (uint8_t id = I2C_LCD_HI; id >= I2C_LCD_LO; id--)
    {
        if (i2cComms.exists(id))   
        {
            disp.setLcd(id);
            announce();                     // Again for I2C LCD.
            break;
        }
    }
#endif

    // Force an LCD shield if no I2C LCD present.
    if (   (!hasLcdShield)
        && (disp.getLcdId() == 0))
    {
        hasLcdShield = true;
        disp.createLcdShield();
        announce();
    }

    // Scan for I2C Gateway.
    for (uint8_t id = I2C_GATEWAY_LO; id <= I2C_GATEWAY_HI; id++)
    {
        if (i2cComms.exists(id))   
        {
            i2cComms.setGateway(id);
            break;
        }
    }

    // Initialise
    disp.printProgStrAt(LCD_COL_START, LCD_ROW_DET, M_STARTUP, LCD_LEN_STATUS);
    
    initButtonPins();                                   // Initialise alternate button pins.
    flashVersion();                                     // Flash our version number on the built-in LED.

    // Deal with first run (software has never been run before).
    if (!loadSystemData())
    {
        firstRun();
    }
    else if (   (hasLcdShield)
             && (   (calibrationRequired())             // Calibration required if it's never been done.
                 || (readButton() != BUTTON_NONE)))     // or an input button is being pressed.
    {
        calibrateButtons();
        saveSystemData();
    }

    // Dump memory in raw format if debug-full.
    if (isDebug(DEBUG_FULL))
    {
        dumpMemory();
    }

    // Check if version update required.
    if (systemData.version != VERSION)
    {
        if (isDebug(DEBUG_NONE))
        {
            Serial.print(PGMT(M_UPDATE));
            Serial.print(CHAR_SPACE);
            Serial.print(systemData.version, HEX);
            Serial.print(CHAR_SPACE);
            Serial.print(VERSION, HEX);
            Serial.println();
        }

        disp.printProgStrAt(LCD_COL_START, LCD_ROW_DET, M_UPDATE, LCD_LEN_STATUS);

        // Do the update here.
        waitForButtonClick();           // Nothing to do, just show it's happening.
        
        systemData.version = VERSION;
        saveSystemData();
    }

    // Scan for Input and Output nodes.
    scanHardware();
}


/** Main loop.
 */
void loop()
{
    // Press any button to configure.
    if (readButton())
    {
        configure.run();
        announce();
    }

    // Look for command characters    
    while (Serial.available() > 0)
    {
        char ch = Serial.read();
        if (ch == CHAR_RETURN)
        {
            // Ignore carriage-return
        }
        else if (ch == CHAR_NEWLINE)
        {
            // Process the received command
            if (commandLen > 0)
            {
                commandBuffer[commandLen] = CHAR_NULL;
                processCommand();
                commandLen = 0;
            }
        }
        else if (commandLen <= COMMAND_BUFFER_LEN)
        {
            commandBuffer[commandLen++] = ch;
        }
    }    

    now = millis();

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
        scanInputs(false);
    }

    // See if there are any Gateway requests
    if (now > tickGateway)
    {
        if (!gatewayRequest())
        {
            tickGateway = now + STEP_GATEWAY;
        }
    }

#if INTERLOCK_WARNING_PIN
    // Check for interlock warning expired
    if (   (timeoutInterlock > 0)
        && (now > timeoutInterlock))
    {
        timeoutInterlock = 0L;
        digitalWrite(INTERLOCK_WARNING_PIN, LOW);
    }
#endif
    
    // Show heartbeat.
    if (now > tickHeartBeat)
    {
        tickHeartBeat = now + STEP_HEARTBEAT;
        
        // If display timeout has expired, clear it.
        if (   (displayTimeout > 0)
            && (now > displayTimeout))
        {
            displayTimeout = 0L;
            announce();
        }

        // Show heartbeat if no display timeout is pending.
        if (displayTimeout == 0)
        {
            int hours = (now)                                                       / MILLIS_PER_HOUR;
            int mins  = (now - MILLIS_PER_HOUR * hours)                             / MILLIS_PER_MINUTE;
            int secs  = (now - MILLIS_PER_HOUR * hours  - MILLIS_PER_MINUTE * mins) / MILLIS_PER_SECOND;
            
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
}
