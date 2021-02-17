/** Control panel.
 *
 *
 *  (c)Copyright Tony Clulow  2021	tony.clulow@pentadtech.com
 *
 *  This work is licensed under the:
 *      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 *      http://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 *   For commercial use, please contact the original copyright holder(s) to agree licensing terms
 */

#define MASTER true         // The master (Uno) device.


#include <EEPROM.h>
#include <Wire.h>

#include "Config.h"
#include "Messages.h"
#include "System.h"
#include "Comms.h"
#include "EzyBus.h"
#include "SignalBox.h"
#include "Lcd.h"
#include "Output.h"
#include "Input.h"
#include "Buttons.h"
#include "Report.h"
#include "ImportExport.h"
#include "Configure.h"


// Timeout for the display when important messages are showing.
long displayTimeout = 1L;   // Using 1 forces an initial redisplay unless a start-up process has requested a delay.

// Record state of input switches.
uint16_t currentSwitchState[INPUT_NODE_MAX];    // Current state of inputs.

// Ticking
long now           = 0;     // The current time in millisecs.
long tickScan      = 0;     // The time of the last scan of input switches.
long tickHeartBeat = 0;     // Time of last heartbeat.


/** Announce ourselves.
 */
void announce()
{
    lcd.clear();
    lcd.printAt(LCD_COL_START,                       LCD_ROW_TOP, M_SOFTWARE);
//    for (uint8_t index = 0; index < LOGO_LEN; index++)
//    {
//        lcd.print((char)index);
//    }
    lcd.printAt(LCD_COLS - strlen_P(M_VERSION),      LCD_ROW_TOP, M_VERSION);
    lcd.printAt(LCD_COLS - strlen_P(M_VERSION_DATE), LCD_ROW_BOT, M_VERSION_DATE);
}


/** Set the display timeout for an important message.
 */
void setDisplayTimeout(long aTimeout)
{
    displayTimeout = millis() + aTimeout;
}


/** Map the Input and Output nodes.
 */
void scanHardware()
{
    // Scan for Input nodes.
    lcd.clear();
    lcd.printAt(LCD_COL_START, LCD_ROW_TOP, M_NODES);
    lcd.setCursor(LCD_COLS - INPUT_NODE_MAX, 0);
    
    for (uint8_t node = 0; node < INPUT_NODE_MAX; node++)
    {
        Wire.beginTransmission(systemData.i2cInputBaseID + node);
        if (Wire.endTransmission())   
        {
            lcd.print(CHAR_DOT); 
        }
        else
        {  
            lcd.print(HEX_CHARS[node]);
            setInputNodePresent(node);
        }
    }
    delay(DELAY_READ);
    waitForButtonRelease();     // Allow more time ro read display
    
    // Scan for Output nodes.
    lcd.clear();
    lcd.setCursor(LCD_COL_START, LCD_ROW_TOP);

    for (uint8_t node = 0; node < OUTPUT_NODE_MAX; node++)
    {
        if (node == LCD_COLS)
        {
            lcd.setCursor(LCD_COL_START, LCD_ROW_BOT);
        }
        char state = readOutputStates(node);

        if (state == 0)
        {
            lcd.print(HEX_CHARS[node]);
        }
        else
        {
            lcd.print(state); 
        }
    }
    delay(DELAY_READ);
    waitForButtonRelease();     // Allow more time to read display

    // Report absence of hardware.
    if (   (inputNodes  == 0)
        || (outputNodes == 0))
    {
        uint8_t row = LCD_ROW_TOP;
        lcd.clear();
        if (inputNodes == 0)
        {
            lcd.printAt(LCD_COL_START, row++, M_NO_INPUTS);
        }
        if (outputNodes == 0)
        {
            lcd.printAt(LCD_COL_START, row++, M_NO_OUTPUTS);
        }
        delay(DELAY_READ);
    }
}


/** Configure all inputs.
 */
void initInputs()
{ 
    lcd.clear();
    lcd.printAt(LCD_COL_START, LCD_ROW_TOP, M_INIT_INPUTS);
    lcd.setCursor(LCD_COL_START, LCD_ROW_BOT);

    // Initialise every Input node
    for(uint8_t node = 0; node < INPUT_NODE_MAX; node++)
    {
        if (isInputNode(node))
        {
            lcd.print(HEX_CHARS[node]);
            
            // Configure for input
            Wire.beginTransmission(systemData.i2cInputBaseID + node); 
            Wire.write(MCP_IODIRA);
            Wire.write(MCP_ALL_HIGH);
            Wire.endTransmission();

            Wire.beginTransmission(systemData.i2cInputBaseID + node);  
            Wire.write(MCP_IODIRB);
            Wire.write(MCP_ALL_HIGH);
            Wire.endTransmission();

            Wire.beginTransmission(systemData.i2cInputBaseID + node);
            Wire.write (MCP_GPPUA);
            Wire.write(MCP_ALL_HIGH);
            Wire.endTransmission();  
             
            Wire.beginTransmission(systemData.i2cInputBaseID + node);
            Wire.write(MCP_GPPUB);
            Wire.write(MCP_ALL_HIGH);
            Wire.endTransmission();

            // Record current switch state
            currentSwitchState[node] = readInputNode(node);

//            // Ensure toggle switches are in current state.
//            for (uint16_t pin = 0, mask = 1; pin < INPUT_PIN_MAX; pin++, mask <<= 1)
//            {
//                loadInput(node, pin);
//                if (inputType == INPUT_TYPE_TOGGLE)
//                {
//                    processInput(currentSwitchState[node] & mask);
//                }
//            }
        }
        else
        {
            // Absent input node.
            lcd.print(CHAR_DOT);
            currentSwitchState[node] = 0xffff;
        }
    }   
    delay(DELAY_READ);
}


/** Software hasn't been run before.
 */
void firstRun()
{
    // Initialise SystemData.
    systemData.magic   = MAGIC_NUMBER;

    //Calibrate the LCD buttons.
    calibrateButtons();

    // Decide if EzyBus conversion required.
    if (ezyBusDetected())
    {
        lcd.clear();
        lcd.printAt(LCD_COL_START, LCD_ROW_TOP, M_EZY_FOUND);
        lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_EZY_UPDATE);

        ezyBusClear();
        if (waitForButton() == BUTTON_SELECT)
        {
            convertEzyBus();
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
    
    delay(DELAY_READ);
}


/** Set the default initial setup
 *  1-1 mapping, inputs to outputs.
 */
void defaultInputs(uint8_t aInputType)
{
    lcd.clear();
    lcd.printAt(LCD_COL_START, LCD_ROW_TOP, M_INITIALISING);
    lcd.setCursor(LCD_COL_START, LCD_ROW_BOT);

    inputNumber = 0;
    inputType   = aInputType;
    
    for (uint8_t node = 0; node < INPUT_NODE_MAX; node++)
    {
        lcd.print(HEX_CHARS[node]);

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
void convertEzyBus()
{
    int     ezyBus = 0;
    uint8_t value  = 0;
    
    lcd.clear();
    lcd.printAt(LCD_COL_START, LCD_ROW_TOP, M_EZY_UPDATING);
    lcd.setCursor(LCD_COL_START, LCD_ROW_BOT);

    for (outputNode = 0; outputNode < EZY_NODE_MAX; outputNode++)
    {
        lcd.print(HEX_CHARS[outputNode]);

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
        if (isOutputNode(node))
        {
            Wire.beginTransmission(systemData.i2cOutputBaseID + node);
            Wire.write(COMMS_CMD_DEBUG | (getDebug() & COMMS_OPTION_MASK));
            Wire.endTransmission();

            if (isDebug(DEBUG_BRIEF))
            {
                Serial.print(millis());
                Serial.print(CHAR_TAB);
                Serial.print(PGMT(M_DEBUG_DEBUG));
                Serial.print(PGMT(M_DEBUG_NODE));
                Serial.print(node);
                Serial.print(CHAR_SPACE);
                Serial.print(PGMT(M_DEBUG_PROMPTS[getDebug()]));
                Serial.println();
            }
        }
    }
}


///** Scan all (know) outputs for their current state.
// */
//void scanOutputs()
//{
//    for (uint8_t node = 0; node < OUTPUT_NODE_MAX; node++)
//    {
//        if (isOutputNode(node))
//        {
//            readOutputStates(node);
//        }
//    }
//}


/** Scan all the inputs.
 *  Process any that have changed.
 */
void scanInputs()
{ 
    // Scan all the nodes. 
    for (uint8_t node = 0; node < INPUT_NODE_MAX; node++)
    {
        if (isInputNode(node))                                        
        {
            // Read current state of pins and if there's been a change
            uint16_t pins = readInputNode(node);
            if (pins != currentSwitchState[node])
            {
                // Process all the changed pins.
                for (uint16_t pin = 0, mask = 1; pin < INPUT_PIN_MAX; pin++, mask <<= 1)
                {
                    uint16_t state = pins & mask;
                    if (state != (currentSwitchState[node] & mask))
                    {
                        loadInput(node, pin);
                        processInput(state);
                    }
                }
            
                // Record new state.
                currentSwitchState[node] = pins;
            }
        }
    }
}


/** Read the pins of a InputNode.
 *  Return the state of the pins, 16 bits, both ports.
 *  Return current state if there's a communication error, 
 *  this will prevent any actions being performed.
 */
uint16_t readInputNode(uint8_t node)
{
    int      error = 0;
    uint16_t value = 0;

    Wire.beginTransmission(systemData.i2cInputBaseID + node);    
    Wire.write(MCP_GPIOA);
    error = Wire.endTransmission();
    if (error)
    {
        systemFail(M_I2C_ERROR, error, DELAY_READ);
        value = currentSwitchState[node];  // Pretend no change if comms error.
    }
    else if ((value = Wire.requestFrom(systemData.i2cInputBaseID + node, 2)) != 2)
    {
        systemFail(M_I2C_COMMS, error, DELAY_READ);
        value = currentSwitchState[node];  // Pretend no change if comms error.
    }
    else
    {
        value = Wire.read()
              + (Wire.read() << 8);
    }

    return value;
}


/** Process the changed input.
 */
void processInput(uint16_t aState)
{
    boolean newState = false;
    
    // Process all input state changes for Toggles, only state going low for other Input types.
    if (   (aState == 0)
        || (inputType == INPUT_TYPE_TOGGLE))
    {
        // Set desired new state based on Input's type/state and Output's state.
        switch (inputType)
        {
            case INPUT_TYPE_TOGGLE: newState = aState != 0;     // Set state to that of the Toggle.
                                    break;
            case INPUT_TYPE_ON_OFF: // Find first real output (not a delay) to determine new state.
                                    for (uint8_t index = 0; index < INPUT_OUTPUT_MAX; index++)
                                    {
                                        if (!inputDef.isDelay(index))
                                        {
                                            newState = !getOutputState(inputDef.getOutputNode(index), inputDef.getOutputPin(index));
                                            break;
                                        }
                                    }
                                    break;
            case INPUT_TYPE_ON:     newState = true;            // Set the state.
                                    break;
            case INPUT_TYPE_OFF:    newState = false;           // Clear the state.
                                    break;
        }

        // Report state change if reporting enabled.
        if (reportEnabled(REPORT_SHORT))
        {
            lcd.clear();
            lcd.printAt(LCD_COL_START, LCD_ROW_TOP, M_INPUT_TYPES[inputType & INPUT_TYPE_MASK]);
            lcd.printAt(LCD_COL_STATE, LCD_ROW_TOP, (newState ? M_HI : M_LO));
            lcd.printAt(LCD_COL_NODE,  LCD_ROW_TOP, HEX_CHARS[(inputNumber >> INPUT_NODE_SHIFT) & INPUT_NODE_MASK]);
            lcd.printAt(LCD_COL_PIN,   LCD_ROW_TOP, HEX_CHARS[(inputNumber                    ) & INPUT_PIN_MASK]);
            lcd.setCursor(LCD_COL_START + 1, LCD_ROW_BOT);
            setDisplayTimeout(reportDelay());

            if (isDebug(DEBUG_BRIEF))
            {
                Serial.println();
                Serial.print(millis());
                Serial.print(CHAR_TAB);
                Serial.print(PGMT(M_INPUT_TYPES[inputType & INPUT_TYPE_MASK]));
                Serial.print(PGMT(M_DEBUG_STATE));
                Serial.print(PGMT(newState ? M_HI : M_LO));
                Serial.print(PGMT(M_DEBUG_NODE));
                Serial.print((inputNumber >> INPUT_NODE_SHIFT) & INPUT_NODE_MASK, HEX);
                Serial.print(PGMT(M_DEBUG_PIN));
                Serial.print((inputNumber                    ) & INPUT_PIN_MASK,  HEX);
                Serial.println();
            }
        }

        // If not locked, process the Input's Outputs.
        if (!isLocked(newState))
        {
            processInputOutputs(newState);
        }
    }
}


/** Check if any of the Input's Outputs are locked.
 */
boolean isLocked(boolean aNewState)
{
    for (uint8_t inpIndex = 0; inpIndex < INPUT_OUTPUT_MAX; inpIndex++)
    {
        if (!inputDef.isDelay(inpIndex))
        {
            readOutput(inputDef.getOutputNode(inpIndex), inputDef.getOutputPin(inpIndex));
    
            for (uint8_t outIndex = 0; outIndex < OUTPUT_LOCK_MAX; outIndex++)
            {
                if (outputDef.isLock(aNewState, outIndex))
                {
                    boolean state = getOutputState(outputDef.getLockNode(aNewState, outIndex), outputDef.getLockPin(aNewState, outIndex));
                    if (outputDef.getLockState(aNewState, outIndex) == state)
                    {
                        if (reportEnabled(REPORT_SHORT))
                        {
                            lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_LOCK, LCD_LEN_OPTION);
                            lcd.print(aNewState ? CHAR_HI : CHAR_LO);
                            lcd.print(HEX_CHARS[outputNode]);
                            lcd.print(HEX_CHARS[outputPin]);
                            lcd.print(PGMT(M_VS));
                            lcd.print(state ? CHAR_HI : CHAR_LO);
                            lcd.print(HEX_CHARS[outputDef.getLockNode(aNewState, outIndex)]);
                            lcd.print(HEX_CHARS[outputDef.getLockPin (aNewState, outIndex)]);
                            setDisplayTimeout(reportDelay());
                        }

                        if (isDebug(DEBUG_BRIEF))
                        {
                            outputDef.printDef(M_LOCK, outputPin);
                            readOutput(outputDef.getLockNode(aNewState, outIndex), outputDef.getLockPin(aNewState, outIndex));
                            outputDef.printDef(M_VS, outputPin);
                        }
                        
                        return true;
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
 *  Accumulate delay before or after movement depending on direction outputs are being processed.
 */
uint8_t processInputOutput(uint8_t aIndex, uint8_t aState, uint8_t aDelay)
{
    uint8_t endDelay = aDelay;

    uint8_t outNode  = inputDef.getOutputNode(aIndex);
    uint8_t outPin   = inputDef.getOutputPin(aIndex);

    // Process the Input's Outputs.
    if (inputDef.isDelay(aIndex))
    {
        endDelay += inputDef.getOutputPin(aIndex);
    }
    else
    {
        if (reportEnabled(REPORT_PAUSE))
        {
            readOutput(inputDef.getOutput(aIndex));
            lcd.clearRow(LCD_COL_START, LCD_ROW_BOT);
            lcd.printAt(LCD_COL_START,  LCD_ROW_BOT, M_OUTPUT_TYPES[outputDef.getType()], LCD_LEN_OPTION);
            lcd.printAt(LCD_COL_OUTPUT_PARAM, LCD_ROW_BOT, HEX_CHARS[outNode]);
            lcd.print(HEX_CHARS[outPin]);
            lcd.print(CHAR_SPACE);
            lcd.print(CHAR_SPACE);
            lcd.print(HEX_CHARS[outputDef.getPace()]);
            lcd.print(CHAR_SPACE);
            lcd.print(CHAR_SPACE);
            lcd.print(HEX_CHARS[aDelay]);
            reportPause();
        }
        else if (reportEnabled(REPORT_SHORT))
        {
            lcd.print(CHAR_SPACE);
            lcd.print(HEX_CHARS[outNode]);
            lcd.print(HEX_CHARS[outPin]);
            setDisplayTimeout(reportDelay());
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

        writeOutputState(outNode, outPin, aState, endDelay);

        // Recover all states from output module (in case LED_4 has moved one).
        readOutputStates(outNode);
        // setOutputState(outNode, outPin, aState);
    }

    return endDelay;
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
    lcd.begin(LCD_COLS, LCD_ROWS);          // LCD panel.
    lcd.createChar(CHAR_LO, BYTES_LO);      // Custom character to indicate "Lo".
    for (uint8_t index = 0; index < CHAR_LO; index++)
    {
        lcd.createChar(index, LOGO[index]);
    }

    // Initial announcement/splash message.
    announce();

    // Add suitable startup/setup message
    if (loadSystemData())
    {
        lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_STARTUP);
    }
    else
    {
        lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_SETUP);
    }
    
    delay(DELAY_START);                 // Wait to avoid programmer conflicts.
    Serial.begin(SERIAL_SPEED);         // Serial IO.
    
    if (isDebug(DEBUG_FULL))
    {
        Serial.print(millis());
        Serial.print(CHAR_TAB);
        Serial.print(PGMT(M_DEBUG_SYSTEM));
        Serial.print(CHAR_SPACE);
        Serial.print(SYSTEM_BASE, HEX);
        Serial.print(CHAR_DASH);
        Serial.print(SYSTEM_END, HEX);
        Serial.print(PGMT(M_DEBUG_INPUTS));
        Serial.print(INPUT_BASE, HEX);
        Serial.print(CHAR_DASH);
        Serial.print(INPUT_END, HEX);
        Serial.println();
    }

    // Initialise alternate button pins.
    initButtonPins();
    
    // Flash our version number on the built-in LED.
    flashVersion();
    
    // Initialise subsystems.
    Wire.begin(systemData.i2cControllerID);     // I2C network
    // Wire.setTimeout(25000L);                 // Doesn't seem to have any effect.
    pinMode(PIN_CALIBRATE, INPUT_PULLUP);       // Calibration input pin (11).

    // Deal with first run (software has never been run before).
    if (!loadSystemData())
    {
        firstRun();
    }
    else if (   (digitalRead(PIN_CALIBRATE) == 0)   // Calibration pin grounded.
             || (readButton() != BUTTON_NONE))      // An input button is being pressed.
    {
        // Calibration requested.
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

        lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_UPDATE);

        // Do the update here.
        delay(DELAY_READ);          // Nothing to do, just show it's happening.
        
        systemData.version = VERSION;
        saveSystemData();
    }

    // Discover and initialise attached hardware.
    scanHardware();                           // Scan for attached hardware.
    initInputs();                             // Initialise all inputs.
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

    now = millis();

    // Process any inputs
    if (now > tickScan)
    {
        tickScan = now + STEP_INPUT_SCAN;
        // scanOutputs();
        scanInputs();
    }
    
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
            int hours = (tickHeartBeat)                                                       / MILLIS_PER_HOUR;
            int mins  = (tickHeartBeat - MILLIS_PER_HOUR * hours)                             / MILLIS_PER_MINUTE;
            int secs  = (tickHeartBeat - MILLIS_PER_HOUR * hours  - MILLIS_PER_MINUTE * mins) / MILLIS_PER_SECOND;
            
            lcd.setCursor(LCD_COL_START, LCD_ROW_BOT);
            if (hours > 0)
            {
                lcd.printDec(hours, 1, CHAR_ZERO);
                lcd.print(CHAR_COLON);
            }
            lcd.printDec(mins, 2, CHAR_ZERO);
            lcd.print(CHAR_COLON);
            lcd.printDec(secs, 2, CHAR_ZERO);
        }
    }
}
