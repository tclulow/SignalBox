/** Control panel.
 */

#include <EEPROM.h>
#include <Wire.h>

#include "Messages.h"
#include "Common.h"
#include "Config.h"
#include "Memory.h"
#include "EzyBus.h"
#include "Panel.h"
#include "Lcd.h"
#include "Output.h"
#include "Input.h"
#include "Buttons.h"
#include "System.h"
#include "Debug.h"
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
        Wire.beginTransmission(systemData.i2cOutputBaseID + node);
        if (Wire.endTransmission())
        {
            lcd.print(CHAR_DOT); 
        }
        else
        {
            lcd.print(HEX_CHARS[node]);
            setOutputNodePresent(node);  
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
    systemData.debugLevel      = DEFAULT_DEBUG;

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
    int input = 0;

    lcd.clear();
    lcd.printAt(LCD_COL_START, LCD_ROW_TOP, M_INITIALISING);
    lcd.setCursor(LCD_COL_START, LCD_ROW_BOT);
    
    for (outputNumber = 0; outputNumber < OUTPUT_NODE_MAX * OUTPUT_NODE_SIZE; outputNumber++) 
    {
        if ((outputNumber & OUTPUT_NODE_PIN_MASK) == 0)
        {
            lcd.print(HEX_CHARS[(outputNumber >> OUTPUT_NODE_SHIFT) & OUTPUT_NODE_MASK]);
        }

        // Create the output.
        outputData.type = OUTPUT_TYPE_SERVO;
        outputData.lo   = OUTPUT_DEFAULT_LO;
        outputData.hi   = OUTPUT_DEFAULT_HI;
        outputData.pace = OUTPUT_DEFAULT_PACE;
        saveOutput();

        // Create an input.
        inputNumber = outputNumber;
        inputData.output[0] = outputNumber;
        inputData.output[1] = INPUT_DISABLED_MASK;
        inputData.output[2] = INPUT_DISABLED_MASK;
        inputType = INPUT_TYPE_ON_OFF;
        saveInput();
    }
}


/** Convert EzyBus configuration.
 *  One-one mapping with EzyBus modules, and their inputs.
 */
void convertEzyBus()
{
    int ezyBus = OUTPUT_NODE_MAX * OUTPUT_NODE_SIZE * OUTPUT_SIZE;
    lcd.clear();
    lcd.printAt(LCD_COL_START, LCD_ROW_TOP, M_EZY_UPDATING);
    lcd.setCursor(LCD_COL_START, LCD_ROW_BOT);

    for (outputNumber = OUTPUT_NODE_MAX * OUTPUT_NODE_SIZE - 1; outputNumber >= 0; outputNumber--) 
    {
        if ((outputNumber & OUTPUT_NODE_PIN_MASK) == OUTPUT_NODE_PIN_MASK)
        {
            lcd.print(HEX_CHARS[(outputNumber >> OUTPUT_NODE_SHIFT) & OUTPUT_NODE_MASK]);
        }

        ezyBus -= OUTPUT_SIZE;
        EEPROM.get(ezyBus, outputData);
        
        // Pace was in steps of 4 (2-bits), drop one bit, store in left-most nibble
        outputData.pace = ((outputData.pace >> EZY_SPEED_SHIFT) & OUTPUT_PACE_MASK) << OUTPUT_PACE_SHIFT;
        
        saveOutput();

        // Create an input.
        inputNumber = outputNumber;
        
        inputData.output[0] = outputNumber;
        inputData.output[1] = INPUT_DISABLED_MASK;
        inputData.output[2] = INPUT_DISABLED_MASK;
        inputType = INPUT_TYPE_TOGGLE;
        
        saveInput();
    }
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
void processInput(int aState)
{
    uint8_t newState = 0;
    
    // Process all input state changes for Toggles, only state going low for other Input types.
    if (   (aState == 0)
        || (inputType == INPUT_TYPE_TOGGLE))
    {
        // Report state change if debug enabled.
        if (debugEnabled(DEBUG_LOW))
        {
            lcd.clear();
            lcd.printAt(LCD_COL_START, LCD_ROW_TOP, M_INPUT_TYPES[inputType & INPUT_TYPE_MASK]);
            lcd.printAt(LCD_COL_STATE, LCD_ROW_TOP, (aState ? M_HI : M_LO));
            lcd.printAt(LCD_COL_NODE,  LCD_ROW_TOP, HEX_CHARS[(inputNumber >> INPUT_NODE_SHIFT) & INPUT_NODE_MASK]);
            lcd.printAt(LCD_COL_PIN,   LCD_ROW_TOP, HEX_CHARS[(inputNumber                    ) & INPUT_PIN_MASK]);
            setDisplayTimeout(DELAY_READ);
            
            #if DEBUG
                Serial.println();
                Serial.print(PGMT(M_INPUT_TYPES[inputType & INPUT_TYPE_MASK]));
                Serial.print(M_SPACE);
                Serial.print(PGMT(aState ? M_HI : M_LO));
                Serial.print(M_SPACE);
                Serial.print(HEX_CHARS[(inputNumber >> INPUT_NODE_SHIFT) & INPUT_NODE_MASK]);
                Serial.print(HEX_CHARS[(inputNumber                    ) & INPUT_PIN_MASK]);
                Serial.println();
            #endif
        }
                        
        // Set desired new state based on Input's type/state and Output's state.
        switch (inputType)
        {
            case INPUT_TYPE_TOGGLE: newState = aState ? OUTPUT_STATE : 0;   // Set state to that of the Toggle.
                                    break;
            case INPUT_TYPE_ON_OFF: loadOutput(inputData.output[0] & INPUT_OUTPUT_MASK);
                                    if (outputData.type & OUTPUT_STATE)     // Change the state.
                                    {
                                        newState = 0;
                                    }
                                    else
                                    {
                                        newState = OUTPUT_STATE;
                                    }
                                    break;
            case INPUT_TYPE_ON:     newState = OUTPUT_STATE;                // Set the state.
                                    break;
            case INPUT_TYPE_OFF:    newState = 0;                           // Clear the state.
                                    break;
        }

        processInputOutputs(newState);
    }
}


/** Process all the Input's Outputs.
 */
void processInputOutputs(uint8_t aNewState)
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
        for (int index = INPUT_OUTPUT_MAX - 1; index >= 0; index--)
        {
            delay = processInputOutput(index, aNewState, delay);
        }
    }
}


/** Process an Input's n'th Output, setting it to the given state.
 */
uint8_t processInputOutput(int aIndex, uint8_t aNewState, uint8_t aDelay)
{
    uint8_t delay = aDelay;
    
    // Process the Input's zeroth Output, and others if not disabled.
    if (   (aIndex == 0)
        || (!(inputData.output[aIndex] & INPUT_DISABLED_MASK)))
    {
        loadOutput(inputData.output[aIndex] & INPUT_OUTPUT_MASK);
        delay += outputData.pace & OUTPUT_DELAY_MASK;

        // Can't delay beyond the maximum possible.
        if (delay > OUTPUT_DELAY_MASK)
        {
            delay = OUTPUT_DELAY_MASK;
        }

        if (aNewState)
        {
            outputData.type |= OUTPUT_STATE;    // Set output state
        }
        else
        {
            outputData.type &= ~OUTPUT_STATE;   // Clear output state
        }
            
        sendOutputCommand((outputData.type & OUTPUT_STATE ? outputData.hi : outputData.lo), outputData.pace, delay, outputData.type & OUTPUT_STATE);
        saveOutput();
    }

    return delay;
}


/** Send a command to an output node.
 *  Return error code if any.
 */
int sendOutputCommand(uint8_t aValue, uint8_t aPace, uint8_t aDelay, uint8_t aState)
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

    if (debugEnabled(DEBUG_LOW))
    {
        lcd.clearRow(LCD_COL_START, LCD_ROW_BOT);
        lcd.printAt(LCD_COL_START,  LCD_ROW_BOT, M_OUTPUT_TYPES[outputData.type & OUTPUT_TYPE_MASK]);
        lcd.printAt(LCD_COL_STATE,  LCD_ROW_BOT, (aState ? M_HI : M_LO));
        lcd.printAt(LCD_COL_NODE,   LCD_ROW_BOT, HEX_CHARS[(outputNumber >> OUTPUT_NODE_SHIFT) & OUTPUT_NODE_MASK]);
        lcd.printAt(LCD_COL_PIN,    LCD_ROW_BOT, HEX_CHARS[(outputNumber                     ) & OUTPUT_PIN_MASK ]);
        setDisplayTimeout(DELAY_READ);
        
        #if DEBUG
            Serial.print(PGMT(M_OUTPUT_TYPES[outputData.type & OUTPUT_TYPE_MASK]));
            Serial.print(M_SPACE);
            Serial.print(PGMT(aState ? M_HI : M_LO));
            Serial.print(M_SPACE);
            Serial.print(HEX_CHARS[(outputNumber >> OUTPUT_NODE_SHIFT) & OUTPUT_NODE_MASK]);
            Serial.print(HEX_CHARS[(outputNumber                     ) & OUTPUT_PIN_MASK]);
            Serial.println();
        #endif
        
        debugPause();
    }
    
    Wire.beginTransmission(systemData.i2cOutputBaseID + ((outputNumber >> OUTPUT_NODE_SHIFT) & OUTPUT_NODE_MASK));
    Wire.write(((outputData.type & OUTPUT_TYPE_MASK) << OUTPUT_TYPE_SHIFT) | (outputNumber & OUTPUT_PIN_MASK));
    Wire.write(aValue);
    Wire.write((((aPace >> OUTPUT_PACE_SHIFT) & OUTPUT_PACE_MASK) << OUTPUT_PACE_MULT) + OUTPUT_PACE_OFFSET);
    Wire.write(aState ? 1 : 0);
    if (aDelay & OUTPUT_DELAY_MASK)
    {
        Wire.write(aDelay & OUTPUT_DELAY_MASK);
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
    
    initialise();
    
//  #if DEBUG
//  Serial.print("System  ");
//  Serial.print(SYSTEM_BASE, HEX);
//  Serial.print(" to ");
//  Serial.print(SYSTEM_END, HEX);
//  Serial.println();
//  Serial.print("Outputs ");
//  Serial.print(OUTPUT_BASE, HEX);
//  Serial.print(" to ");
//  Serial.print(OUTPUT_END, HEX);
//  Serial.println();
//  Serial.print("Inputs  ");
//  Serial.print(INPUT_BASE, HEX);
//  Serial.print(" to ");
//  Serial.print(INPUT_END, HEX);
//  Serial.println();
//  #endif

//  #if DEBUG
//  loadInput(3, 4);
//  inputData.output[0] = (0x1 << OUTPUT_NODE_SHIFT) | 7 | INPUT_TOGGLE_MASK; 
//  inputData.output[1] = (0xc << OUTPUT_NODE_SHIFT) | 5;
//  inputData.output[2] = INPUT_DISABLED_MASK;
//  saveInput();
//
//  loadInput(3, 5);
//  inputData.output[0] = (0x9 << OUTPUT_NODE_SHIFT) | 3; 
//  inputData.output[1] = INPUT_DISABLED_MASK;
//  inputData.output[2] = (0x6 << OUTPUT_NODE_SHIFT) | 1;
//  saveInput();
//
//  loadOutput(0x1, 7);
//  outputData.type = OUTPUT_TYPE_NONE;
//  outputData.lo   = 0x17;
//  outputData.hi   = 0x22;
//  outputData.pace = 0x33;
//  saveOutput();
//
//  loadOutput(0xc, 5);
//  outputData.type = OUTPUT_TYPE_SERVO;
//  outputData.lo   = 0xc5;
//  outputData.hi   = 0x44;
//  outputData.pace = 0x55;
//  saveOutput();
//
//  loadOutput(0x9, 3);
//  outputData.type = OUTPUT_TYPE_SIGNAL;
//  outputData.lo   = 0x93;
//  outputData.hi   = 0x66;
//  outputData.pace = 0x77;
//  saveOutput();
//  
//  loadOutput(0x6, 1);
//  outputData.type = OUTPUT_TYPE_LED;
//  outputData.lo   = 0x61;
//  outputData.hi   = 0x88;
//  outputData.pace = 0xaa;
//  saveOutput();
//  #endif


    // Initialise subsystems.
    Wire.begin(systemData.i2cControllerID);   // I2C network
    pinMode(PIN_CALIBRATE, INPUT_PULLUP);     // Calibration input pin (11).

    // Deal with first run (software has never been run before).
    if (!loadSystemData())     //  || ezyBusDetected())
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
    int  loops    = 0;
    long lastLoop = millis();

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
                if (loops > 7)
                {
                    loops = 0;
                }
                lcd.printAt((loops    ) & 0x7, LCD_ROW_BOT, CHAR_DOT);
                lcd.printAt((loops + 4) & 0x7, LCD_ROW_BOT, CHAR_SPACE);
                loops += 1;

                lastLoop = millis();
            }
        }
    }
}
 
