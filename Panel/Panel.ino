/** Control panel.
 */

#include <EEPROM.h>
#include <Wire.h>

#include "Config.h"
#include "Messages.h"
#include "Common.h"
#include "Memory.h"
#include "Comms.h"
#include "EzyBus.h"
#include "Panel.h"
#include "Lcd.h"
#include "Output.h"
#include "Input.h"
#include "Buttons.h"
#include "System.h"
#include "Report.h"
#include "ImportExport.h"
#include "Configure.h"


// Timeout for the display when important messages are showing.
long displayTimeout = 0L;

// Record state of input switchess.
uint16_t currentSwitchState[INPUT_NODE_MAX];    // Current state of inputs.



/** Announce ourselves.
 */
void announce()
{
    lcd.clear();
    lcd.printAt(LCD_COL_START,                       LCD_ROW_TOP, M_SOFTWARE);
    lcd.printAt(LCD_COLS - strlen_P(M_VERSION_DATE), LCD_ROW_TOP, M_VERSION_DATE);
    lcd.printAt(LCD_COLS - strlen_P(M_VERSION),      LCD_ROW_BOT, M_VERSION);
}


/** Set the display timeout for an important message.
 */
void setDisplayTimeout(long aTimeout)
{
    displayTimeout = millis() + aTimeout;
}


/** Map the Input and Output nodes.
 */
void mapHardware()
{
    lcd.clear();
    lcd.printAt(LCD_COL_START, LCD_ROW_TOP, M_SCAN_NODES);
    lcd.setCursor(LCD_COLS - INPUT_NODE_MAX, 0);
    
    // Scan for Input nodes.
    for (int node = 0; node < INPUT_NODE_MAX; node++)
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

    // Scan for Output nodes.
    lcd.setCursor(0, 1);
    for (int node = 0; node < OUTPUT_NODE_MAX; node++)
    {
        char state = readOutputStates(node);

        if (state == 0)
        {
            lcd.print(HEX_CHARS[node]);
            setOutputNodePresent(node);
        }
        else
        {
            lcd.print(CHAR_DOT); 
        }
    }

    delay(DELAY_READ);

    // Report abscence of hardware (and default to all if necessary).
    if (   (inputNodes  == 0)
        || (outputNodes == 0))
    {
        int row = LCD_ROW_TOP;
        lcd.clear();
        if (inputNodes == 0)
        {
            lcd.printAt(LCD_COL_START, row++, M_NO_INPUTS);
            inputNodes  = INPUT_NODE_ALL_MASK;
        }
        if (outputNodes == 0)
        {
            lcd.printAt(LCD_COL_START, row++, M_NO_OUTPUTS);
            outputNodes = OUTPUT_NODE_ALL_MASK;
        }
        delay(DELAY_READ);
    }
}


/** Configure all inputs for input.
 */
void initInputs()
{ 
    lcd.clear();
    lcd.printAt(LCD_COL_START, LCD_ROW_TOP, M_INIT_INPUTS);
    lcd.setCursor(LCD_COL_START, LCD_ROW_BOT);

    // Clear state of Inputs, all high.
    for (int node = 0; node < INPUT_NODE_MAX; node++)
    {
        currentSwitchState[node] = 0xffff;
    }

    // For every Input node, set it's mode of operation.
    for(int node = 0; node < INPUT_NODE_MAX; node++)
    {
        if (isInputNode(node))
        {
            lcd.print(HEX_CHARS[node]);
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
        }
        else
        {
            lcd.print(CHAR_DOT);
        }
    }   
    delay(DELAY_READ);
}


/** Software hasn't been run before.
 */
void firstRun()
{
    lcd.clear();
    lcd.printAt(LCD_COL_START, LCD_ROW_TOP, M_SETUP);
    delay(DELAY_READ);

    // Initialise SystemData.
    systemData.magic   = MAGIC_NUMBER;
    systemData.version = VERSION;

    systemData.i2cControllerID = DEFAULT_I2C_CONTROLLER_ID;
    systemData.i2cInputBaseID  = DEFAULT_I2C_INPUT_BASE_ID;
    systemData.i2cOutputBaseID = DEFAULT_I2C_OUTPUT_BASE_ID;
    systemData.reportLevel     = DEFAULT_REPORT;

    for (int rfu = 0; rfu < SYSTEM_RFU; rfu++)
    {
        systemData.rfu[rfu] = 0;
    }

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
        }
        else
        {
            defaultSetup();
        }
    }
    else
    {
        defaultSetup();
    }

    // Save all data to EEPROM.
    saveSystemData();
    
    delay(DELAY_READ);
}


/** Set the default initial setup
 *  Servos with mid-position and mid-pace.
 */
void defaultSetup()
{
    lcd.clear();
    lcd.printAt(LCD_COL_START, LCD_ROW_TOP, M_INITIALISING);
    lcd.setCursor(LCD_COL_START, LCD_ROW_BOT);

    inputNumber = 0;

    for (outputNode = 0; outputNode < OUTPUT_NODE_MAX; outputNode++)
    {
        lcd.print(HEX_CHARS[outputNode]);

        for (outputPin = 0; outputPin < OUTPUT_PIN_MAX; outputPin++)
        {
            // Create an input.
            for (int index = 0; index < INPUT_OUTPUT_MAX; index++)
            {
                inputDef.setOutput(index, inputNumber);
                inputDef.setDisabled(index, index != 0);
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
    // TODO - implement with Wire
//    int ezyBus = OUTPUT_NODE_MAX * OUTPUT_NODE_SIZE * OUTPUT_SIZE;
//    lcd.clear();
//    lcd.printAt(LCD_COL_START, LCD_ROW_TOP, M_EZY_UPDATING);
//    lcd.setCursor(LCD_COL_START, LCD_ROW_BOT);
//
//    for (outputNumber = OUTPUT_NODE_MAX * OUTPUT_NODE_SIZE - 1; outputNumber >= 0; outputNumber--) 
//    {
//        if ((outputNumber & OUTPUT_NODE_PIN_MASK) == OUTPUT_NODE_PIN_MASK)
//        {
//            lcd.print(HEX_CHARS[(outputNumber >> OUTPUT_NODE_SHIFT) & OUTPUT_NODE_MASK]);
//        }
//
//        ezyBus -= OUTPUT_SIZE;
//        EEPROM.get(ezyBus, outputDef);
//        
//        // Pace was in steps of 4 (2-bits), drop one bit, store in left-most nibble
//        outputDef.pace = ((outputDef.pace >> EZY_SPEED_SHIFT) & OUTPUT_PACE_MASK) << OUTPUT_PACE_SHIFT;
//        
//        saveOutput();
//
//        // Create an input.
//        inputNumber = outputNumber;
//        
//        inputDef.output[0] = outputNumber;
//        inputDef.output[1] = INPUT_DISABLED_MASK;
//        inputDef.output[2] = INPUT_DISABLED_MASK;
//        inputType = INPUT_TYPE_TOGGLE;
//        
//        saveInput();
//    }
}


/** Scan all the inputs.
 *  Process any that have changed.
 */
void scanInputs()
{ 
//  #if DEBUG
//  processInput(3, 4, 0x00);
//  processInput(3, 4, 0x80);
//  processInput(3, 5, 0x00);
//  processInput(3, 5, 0x80);
//  #endif 

    // Scan all the nodes. 
    for (uint8_t node = 0; node < INPUT_NODE_MAX; node++)
    {
        if (isInputNode(node))                                        
        {
            // Read current state of pins and if there's been a change
            int pins = readInputNode(node);
            if (pins != currentSwitchState[node])
            {
                // Process all the changed pins.
                for (int pin = 0, mask = 1; pin < INPUT_NODE_SIZE; pin++, mask <<= 1)
                {
                    int state = pins & mask;
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
int readInputNode(int node)
{
    int value = 0;

    Wire.beginTransmission(systemData.i2cInputBaseID + node);    
    Wire.write(MCP_GPIOA);
    value = Wire.endTransmission();
    if (value)
    {
        systemFail(M_MCP_ERROR, value, DELAY_READ);
        value = currentSwitchState[node];  // Pretend no change if comms error.
    }
    else if ((value = Wire.requestFrom(systemData.i2cInputBaseID + node, 2)) != 2)
    {
        systemFail(M_MCP_COMMS, value, DELAY_READ);
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
void processInput(uint8_t aState)
{
    boolean newState = false;
    
    // Process all input state changes for Toggles, only state going low for other Input types.
    if (   (aState == 0)
        || (inputType == INPUT_TYPE_TOGGLE))
    {
        // Report state change if reporting enabled.
        if (reportEnabled(REPORT_SHORT))
        {
            lcd.clear();
            lcd.printAt(LCD_COL_START, LCD_ROW_TOP, M_INPUT_TYPES[inputType & INPUT_TYPE_MASK]);
            lcd.printAt(LCD_COL_STATE, LCD_ROW_TOP, (aState ? M_HI : M_LO));
            lcd.printAt(LCD_COL_NODE,  LCD_ROW_TOP, HEX_CHARS[(inputNumber >> INPUT_NODE_SHIFT) & INPUT_NODE_MASK]);
            lcd.printAt(LCD_COL_PIN,   LCD_ROW_TOP, HEX_CHARS[(inputNumber                    ) & INPUT_PIN_MASK]);
            setDisplayTimeout(reportDelay());
            
            #if DEBUG
                Serial.println();
                Serial.print(millis());
                Serial.print("\tInput ");
                Serial.print(PGMT(M_INPUT_TYPES[inputType & INPUT_TYPE_MASK]));
                Serial.print(CHAR_SPACE);
                Serial.print(PGMT(aState ? M_HI : M_LO));
                Serial.print(CHAR_SPACE);
                Serial.print(HEX_CHARS[(inputNumber >> INPUT_NODE_SHIFT) & INPUT_NODE_MASK]);
                Serial.print(HEX_CHARS[(inputNumber                    ) & INPUT_PIN_MASK]);
                Serial.println();
            #endif
        }
                        
        // Set desired new state based on Input's type/state and Output's state.
        switch (inputType)
        {
            case INPUT_TYPE_TOGGLE: newState = aState != 0;     // Set state to that of the Toggle.
                                    break;
            case INPUT_TYPE_ON_OFF: readOutput(inputDef.getOutput(0));      // TODO - avoid reading Outputs.
                                    newState = !outputDef.getState();
                                    // newState = !getOutputState(inputDef.getOutputNode(0), inputDef.getOutputPin(0));
                                    break;
            case INPUT_TYPE_ON:     newState = true;            // Set the state.
                                    break;
            case INPUT_TYPE_OFF:    newState = false;           // Clear the state.
                                    break;
        }

        processInputOutputs(newState);
    }
}


/** Process all the Input's Outputs.
 */
void processInputOutputs(boolean aNewState)
{
    uint8_t delay = 0;
    
    // Process all the Input's outputs.
    // In reverse order if setting lo.
    if (aNewState)
    {
        for (int index = 0; index < INPUT_OUTPUT_MAX; index++)
        {
            delay = processInputOutput(index, aNewState, delay);
        }
    }
    else
    {
        // Get initial delay from Input's zeroth output.
        readOutput(inputDef.getOutput(0));                  // TODO - avoid having to fetch Output's def.
        delay = outputDef.getDelay();
        
        for (int index = INPUT_OUTPUT_MAX - 1; index >= 0; index--)
        {
            delay = processInputOutput(index, aNewState, delay);
        }
    }
}


/** Process an Input's n'th Output, setting it to the given state.
 *  Accumulate delay before or after movement depending on direction outputs are being processed.
 */
uint8_t processInputOutput(int aIndex, uint8_t aNewState, uint8_t aDelay)
{
    uint8_t delay = aDelay;
    
    // Process the Input's zeroth Output, and others if not disabled.
    if (!inputDef.isDisabled(aIndex))
    {
        readOutput(inputDef.getOutput(aIndex));
        delay += outputDef.getDelay();

        // Can't delay beyond the maximum possible.
        if (delay > OUTPUT_DELAY_MASK)
        {
            delay = OUTPUT_DELAY_MASK;
        }

        outputDef.setState(aNewState);
        setOutputState(outputNode, outputPin, aNewState);
        writeOutputState(outputNode, outputPin, aNewState, delay);
    }

    return delay;
}


/** Send a change-of-state to a particular Output.
 */
int writeOutputState(uint8_t aNode, uint8_t aPin, boolean aState, uint8_t aDelay)
{
//  #if DEBUG
//  // Report output
//  Serial.print("Output ");
//  Serial.print(aState ? "Hi" : "Lo");
//  Serial.print(" ");
//  Serial.print(HEX_CHARS[(outputNumber >> OUTPUT_NODE_SHIFT) & OUTPUT_NODE_MASK]);
//  Serial.print(" ");
//  Serial.print(HEX_CHARS[(outputNumber                     ) & OUTPUT_PIN_MASK ]);
//  Serial.print(" ");
//  Serial.print(aValue, HEX);
//  Serial.print(" ");
//  Serial.print(aPace);
//  Serial.println();
//  #endif

    if (reportEnabled(REPORT_SHORT))
    {
        lcd.clearRow(LCD_COL_START, LCD_ROW_BOT);
        lcd.printAt(LCD_COL_START,  LCD_ROW_BOT, M_OUTPUT_TYPES[outputDef.getType()]);
        lcd.printAt(LCD_COL_STATE,  LCD_ROW_BOT, (aState ? M_HI : M_LO));
        lcd.printAt(LCD_COL_NODE,   LCD_ROW_BOT, HEX_CHARS[outputNode]);
        lcd.printAt(LCD_COL_PIN,    LCD_ROW_BOT, HEX_CHARS[outputPin]);
        setDisplayTimeout(reportDelay());
        
        #if DEBUG
            Serial.print(millis());
            Serial.print(CHAR_TAB);
            Serial.print(PGMT(M_OUTPUT_TYPES[outputDef.getType()]));
            Serial.print(CHAR_SPACE);
            Serial.print(PGMT(aState ? M_HI : M_LO));
            Serial.print(CHAR_SPACE);
            Serial.print(HEX_CHARS[outputNode]);
            Serial.print(HEX_CHARS[outputPin]);
            Serial.print(CHAR_SPACE);
            Serial.print(aDelay, HEX);
            Serial.println();
        #endif
        
        reportPause();
    }

    Wire.beginTransmission(systemData.i2cOutputBaseID + aNode);
    Wire.write((aState ? COMMS_CMD_SET_HI : COMMS_CMD_SET_LO) | aPin);
    if (aDelay)
    {
        Wire.write(aDelay);
    }
    return Wire.endTransmission();
}


//uint8_t * heapPtr, * stackPtr;
//
//void checkMem(char* aMessage)
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
    lcd.begin(LCD_COLS, LCD_ROWS);            // LCD panel.
    announce();
    lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_STARTUP);
    
    initialise();
    
//    #if DEBUG
//        Serial.print("System  ");
//        Serial.print(SYSTEM_BASE, HEX);
//        Serial.print(" to ");
//        Serial.print(SYSTEM_END, HEX);
//        Serial.println();
//        Serial.print("Outputs ");
//        Serial.print(OUTPUT_BASE, HEX);
//        Serial.print(" to ");
//        Serial.print(OUTPUT_END, HEX);
//        Serial.println();
//        Serial.print("Inputs  ");
//        Serial.print(INPUT_BASE, HEX);
//        Serial.print(" to ");
//        Serial.print(INPUT_END, HEX);
//        Serial.println();
//    #endif

    // Initialise subsystems.
    Wire.begin(systemData.i2cControllerID);   // I2C network
    pinMode(PIN_CALIBRATE, INPUT_PULLUP);     // Calibration input pin (11).

    // Deal with first run (software has never been run before).
    if (!loadSystemData())      // || ezyBusDetected())
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

    // Check if version update required.
    if (systemData.version != VERSION)
    {
        Serial.print(PGMT(M_UPDATE));
        Serial.print(CHAR_SPACE);
        Serial.print(systemData.version, HEX);
        Serial.print(CHAR_SPACE);
        Serial.print(VERSION, HEX);
        Serial.println();

        lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_UPDATE);

        // Do the update here.
        delay(DELAY_READ);          // Nothing to do, just show it's happening.
        
        systemData.version = VERSION;
        saveSystemData();
    }

    // Discover and initialise attached hardware.
    mapHardware();                            // Scan for attached hardware.
    initInputs();                             // Initialise all inputs.

    // Announce ourselves.
    announce();
}


/** Main loop.
 */
void loop()
{
    long lastLoop = millis();
    long count = 0;

    // Loop forever
    while (true)
    {
        // Press any button to configure.
        if (readButton())
        {
            configure.run();
            announce();
        }

        // Process any inputs
        scanInputs();           

        // Show heartbeat.
        if (millis() - lastLoop > DELAY_HEARTBEAT)
        {
            // If display timeout has expired, clear it.
            if (   (displayTimeout > 0)
                && (millis() > displayTimeout))
            {
                displayTimeout = 0L;
                announce();
            }

            // Show heartbeat if no display timeout is pending.
            if (displayTimeout == 0)
            {
                lastLoop = millis();
                int hours = lastLoop / MILLIS_PER_HOUR;
                int mins  = (lastLoop - MILLIS_PER_HOUR * hours) / MILLIS_PER_MINUTE;
                int secs  = (lastLoop - MILLIS_PER_HOUR * hours  - MILLIS_PER_MINUTE * mins) / MILLIS_PER_SECOND;
                
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
}
 
