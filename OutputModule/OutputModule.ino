/** OutputModule.
 *
 *
 *  (c)Copyright Tony Clulow  2021  tony.clulow@pentadtech.com
 *
 *  This work is licensed under the:
 *      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 *      http://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 *  For commercial use, please contact the original copyright holder(s) to agree licensing terms.
 */

#define MASTER false        // The is not the master, it's a nano output module.


#include <Servo.h>
#include "All.h"


// Definitions for paired LEDS
// Other (pin - 1) LED           output is wired Hi=Red,   Lo=Amber
// This  (pin    ) LED_4 or ROAD output is wired Hi=Amber, Lo=Green.
// Phase is calculated using LED state for bit 0, this state for bit 1 => 0 to 3.
//
// Colour   LED   LED_4  On      Off    Phase   Next
// Red       Hi    Lo    LED     LED_4    1      3
// Amber     Hi    Hi    LED_4   LED      3      2
// Amber*2   Lo    Hi    Both    None     2      0
// Green     Lo    Lo    LED_4   LED      0      0
//
// Colour   LED   ROADUK On      Off    Phase   Next
// Red       Hi    Lo    LED     ROAD     1      3
// Red&Amber Hi    Hi    Both    None     3      0
// Green     Lo    Lo    ROADUK  LED      0      2
// Amber     Lo    Hi    ROADUK  LED      2      1
//
// Colour   LED   ROADRW On      Off    Phase   Next
// Red       Hi    Lo    LED     ROAD     1      0
//           Hi    Hi    ROADRW  LED      3      1          // Not used
// Green     Lo    Lo    ROADRW  LED      0      2
// Amber     Lo    Hi    ROADRW  LED      2      1

static const uint8_t LED_4_NEXT_PHASE[]   = { 0, 3, 0, 2 }; // Representation of table above for LED_4.
static const uint8_t ROAD_UK_NEXT_PHASE[] = { 2, 3, 1, 0 }; // Representation of table above for ROADUK.
static const uint8_t ROAD_RW_NEXT_PHASE[] = { 2, 0, 1, 1 }; // Representation of table above for ROADRW.

//                                                                   Other     This
// States where the pin is forced off (both outputs low)    // State 3 2 1 0   3 2 1 0
static const uint8_t LED_4_OFF = 0x92;                      //       1 0 0 1   0 0 1 0 = 0x92.
static const uint8_t ROAD_OFF  = 0x52;                      //       0 1 0 1   0 0 1 0 = 0x52.


// Should changes be persisted?
boolean persisting = true;


// Ticking
long    now       = 0;  // To keep the current time (since boot).
long    tickServo = 0;  // Ticking for Servos.
long    tickLed   = 0;  // Ticking for Leds.
long    tickFlash = 0;  // Ticking for Flashers.
uint8_t tickPwm   = 0;  // Ticking for PWM output of LEDs.


// I2C request command parameters
uint8_t requestCommand = COMMS_CMD_NONE;
uint8_t requestOption  = 0;
uint8_t requestNode    = 0;


// An Array of Output control structures.
struct 
{
    Servo   servo;          // The Servo (if there is one).
    long    delayTo   = 0;  // Start at this time.
    uint8_t steps     = 0;  // The number of steps to take.
    uint8_t step      = 0;  // The current step.
    uint8_t start     = 0;  // The starting value.
    uint8_t value     = 0;  // The value of the output.
    uint8_t target    = 0;  // The target value to aim for.
    uint8_t altStart  = 0;  // The alt starting value.
    uint8_t altValue  = 0;  // The alt value of the output.
    uint8_t altTarget = 0;  // The alt target value to aim for.
} outputs[IO_PINS];


/** Setup the Arduino.
 */
void setup()
{
    randomSeed(analogRead(0));      // Initialise random number generator.
    Serial.begin(SERIAL_SPEED);     // Serial IO.

    // Read jumper pins in case they're needed.
    readJumperPins();

    // Configure the IO pins for output.
    for (uint8_t pin = 0; pin < IO_PINS; pin++)
    {
        pinMode(sigPins[pin], OUTPUT);
        pinMode(ioPins[pin],  OUTPUT);
    }

    // Load SystemData from EEPROM and check it's valid.
    if (!loadSystemData())
    {
        firstRun();
    }
    else
    {
        // Recover state from EEPROM.
        for (uint8_t pin = 0; pin < IO_PINS; pin++)
        {
            loadOutput(pin);
            initOutput(pin, OUTPUT_TYPE_NONE);
            initFlasher(pin);
        }
    }

    // Start I2C communications.
    i2cComms.setId(getModuleId(true));
    i2cComms.onReceive(processReceipt);
    i2cComms.onRequest(processRequest);

    // Flash out version number on the built-in LED,
    // unless that's a Servo - don't want to mess with it's attached base pin.
    if (!outputDefs[OUTPUT_BUILTIN_PIN].isServo())
    {
        flashVersion();
    }

    // Show system data (depending on debug level).
    debugSystemData();
}


/** Initialise data when first run.
 */
void firstRun()
{
    uint8_t moduleId = getModuleId(false);
    
    // Initialise SystemData.
    systemData.magic = MAGIC_NUMBER;

    // Initialise EEPROM with suitable data.
    for (uint8_t pin = 0; pin < IO_PINS; pin++)
    {
        outputDefs[pin].setType(OUTPUT_TYPE_NONE);
        outputDefs[pin].setLo(OUTPUT_DEFAULT_LO);
        outputDefs[pin].setHi(OUTPUT_DEFAULT_HI);
        outputDefs[pin].setPace(OUTPUT_DEFAULT_PACE);
        outputDefs[pin].setReset(OUTPUT_DEFAULT_RESET);

        // Initialise locks all off.
        for (uint8_t pin = 0; pin < OUTPUT_PIN_MAX; pin++)
        {
            for (uint8_t index = 0; index < OUTPUT_LOCK_MAX; index++)
            {
                outputDefs[pin].setLockNode(false, index, moduleId);
                outputDefs[pin].setLockPin (false, index, pin);
                outputDefs[pin].setLockNode(true,  index, moduleId);
                outputDefs[pin].setLockPin (true,  index, pin);
            }
            outputDefs[pin].clearLocks();
        }

        saveOutput(pin);
    }

    saveSystemData();
}


/** Report the status of an output during it's move.
 *  Only when debug level is high enough (at callers discretion).
 */
void reportOutput(PGM_P aHeader, uint8_t aPin)
{
    Serial.print(millis());
    Serial.print(CHAR_TAB);
    Serial.print(PGMT(aHeader));
    Serial.print(aPin, HEX);
    Serial.print(PGMT(M_DEBUG_TYPE));
    Serial.print(PGMT(M_OUTPUT_TYPES[outputDefs[aPin].getType() & OUTPUT_TYPE_MASK]));
    Serial.print(PGMT(M_DEBUG_STATE));
    Serial.print(PGMT(outputDefs[aPin].getState() ? M_HI : M_LO));
    Serial.print(PGMT(M_DEBUG_DELAY_TO));
    Serial.print(outputs[aPin].delayTo);
    Serial.print(PGMT(M_DEBUG_STEPS));
    Serial.print(outputs[aPin].steps, HEX);
    Serial.print(PGMT(M_DEBUG_STEP));
    Serial.print(outputs[aPin].step, HEX);
    Serial.print(PGMT(M_DEBUG_START));
    Serial.print(outputs[aPin].start, HEX);
    Serial.print(PGMT(M_DEBUG_VALUE));
    Serial.print(outputs[aPin].value, HEX);
    Serial.print(PGMT(M_DEBUG_TARGET));
    Serial.print(outputs[aPin].target, HEX);
    Serial.print(PGMT(M_DEBUG_ALT));
    Serial.print(PGMT(M_DEBUG_START));
    Serial.print(outputs[aPin].altStart, HEX);
    Serial.print(PGMT(M_DEBUG_VALUE));
    Serial.print(outputs[aPin].altValue, HEX);
    Serial.print(PGMT(M_DEBUG_TARGET));
    Serial.print(outputs[aPin].altTarget, HEX);
    Serial.println();
}


/** Initialise an Output.
 *  Detach/attach servo as necessary.
 *  Set outputs entry if movement necessary.
 */
void initOutput(uint8_t aPin, uint8_t aOldType)
{
    if (isDebug(DEBUG_BRIEF))
    {
        reportOutput(M_DEBUG_INIT, aPin);
    }
        
    // Detach servo if currently attached and no longer required.
    if (   (isServo(aOldType))
        && (!outputDefs[aPin].isServo()))
    {
        // Detach servo.
        outputs[aPin].servo.detach();
    }

    // Establish new type.
    if (outputDefs[aPin].isServo())
    {
        if (isServo(aOldType))
        {
            // Already attached, move (at correct pace) to new position (immediately).
            actionState(aPin, outputDefs[aPin].getState(), 0, true);
        }
        else
        {
            // Ensure servo is set to correct angle, and attach it.
            if (outputDefs[aPin].getState())
            {
                outputs[aPin].servo.write(outputDefs[aPin].getHi());
            }
            else
            {
                outputs[aPin].servo.write(outputDefs[aPin].getLo());
            }
            digitalWrite(ioPins[aPin], outputDefs[aPin].getState());
            outputs[aPin].servo.attach(sigPins[aPin]);
        }
    }
    else if (   (outputDefs[aPin].getType() == OUTPUT_TYPE_RANDOM)
             && (persisting))
    {
        // Ensure random LEDs are initialised correctly.
        actionState(aPin, outputDefs[aPin].getState(), 0, false);
    }
    else if (   (outputDefs[aPin].isLed())
             || (outputDefs[aPin].isFlasher()))
    {
        // Ensure LEDs glow with correct intensity.
        outputs[aPin].value    = outputDefs[aPin].getState() ? outputDefs[aPin].getHi() : 0;
        outputs[aPin].altValue = outputDefs[aPin].getState() ? 0 : outputDefs[aPin].getLo();

        // No flashing.
        outputs[aPin].steps = 0;

        if (persisting)
        {
            // Handle double-LEDs as special case (if previous output is a LED).
            // See table at top of source for states and colour sequences.
            if (isDoubleLed(aPin))   
            {
                if  (outputDefs[aPin].getType() == OUTPUT_TYPE_LED_4)
                {
                    if (outputDefs[aPin].getState() == outputDefs[aPin - 1].getState())
                    {
                        outputs[aPin - 1].value = 0;        // LED red isn't required.
                        outputs[aPin - 1].altValue = 0;     // LED amber isn't required.
                    }
                    else if (!outputDefs[aPin].getState())
                    {
                        outputs[aPin].altValue = 0;         // 1 case where LED_4 green isn't required.
                    }
                }
                else
                {
                    // ROADs always start at red.
                    outputDefs[aPin - 1].setState(true);
                    outputDefs[aPin    ].setState(false);
                    outputs[aPin - 1].value    = outputDefs[aPin - 1].getHi();
                    outputs[aPin - 1].altValue = 0;
                    outputs[aPin    ].value    = 0;
                    outputs[aPin    ].altValue = 0;
                }
    
                // Make sure auto-reset is actioned (unless pair of ROADs adjacent to each other).
                if (outputDefs[aPin].getReset() > 0)
                {
                    if (   (!isDoubleLed(aPin - 2))
                        || (outputDefs[aPin    ].getType() != OUTPUT_TYPE_ROAD_UK)
                        || (outputDefs[aPin - 2].getType() != OUTPUT_TYPE_ROAD_UK))
                    {
                        actionState(aPin, false, 0, false);
                    }
                }
            }
            else if (isDoubleLed(aPin + 1))
            {
                initOutput(aPin + 1, outputDefs[aPin + 1].getType());
            }
        }
    }
    else
    {
        // All other outputs, turn pins off. 
        digitalWrite(sigPins[aPin], LOW);
        digitalWrite(ioPins[aPin],  LOW);
    }
}


/** Initialise a flasher.
 */
void initFlasher(uint8_t aPin)
{
    if (   (outputDefs[aPin].isFlasher())
        && (outputDefs[aPin].getState())
        && (outputDefs[aPin].getReset() == 0))
    {
        // Indefinite flashers that are high must be started.
        actionState(aPin, outputDefs[aPin].getState(), 0, false);
    }
    else if (outputDefs[aPin].getType() == OUTPUT_TYPE_BLINK)
    {
        // BLINKs must be completely off.
        outputs[aPin].value    = 0;
        outputs[aPin].altValue = 0;
    }
}


/** Report unrecognised command.
 */
void unrecognisedCommand(PGM_P aMessage, uint8_t aCommand, uint8_t aOption)
{
    if  (isDebug(DEBUG_ERRORS))
    {
        Serial.print(millis());
        Serial.print(CHAR_TAB);
        Serial.print(PGMT(aMessage));
        Serial.print(PGMT(M_DEBUG_COMMAND));
        Serial.print(aCommand, HEX);
        Serial.print(PGMT(M_DEBUG_OPTION));
        Serial.print(aOption);
        Serial.println();
    }
}


/** Process a Request (for data).
 *  Send data to master.
 */
void processRequest()
{
    if (isDebug(DEBUG_BRIEF))
    {
        Serial.println();
        Serial.print(millis());
        Serial.print(CHAR_TAB);
        Serial.print(PGMT(M_DEBUG_REQUEST));
        Serial.print(PGMT(M_DEBUG_COMMAND));
        Serial.print(PGMT(M_DEBUG_COMMANDS[requestCommand >> COMMS_COMMAND_SHIFT]));
        Serial.print(PGMT(M_DEBUG_OPTION));
        Serial.print(requestOption, HEX);
        Serial.println();
    }
    
    switch (requestCommand)
    {
        case COMMS_CMD_SYSTEM: returnSystem();
                               break;
        case COMMS_CMD_READ:   returnDef();
                               break;
        default:               unrecognisedCommand(M_DEBUG_REQUEST, requestCommand, requestOption);
                               break;
    }

    // Clear pending command.
    requestCommand = COMMS_CMD_NONE;
}


/** Return responses for system commands.
 */
void returnSystem()
{
    switch (requestOption)
    {
        case COMMS_SYS_STATES:   returnStates();
                                 break;
        case COMMS_SYS_RENUMBER: returnRenumber();
                                 break;
        default:                 unrecognisedCommand(M_DEBUG_SYSTEM, requestCommand, requestOption);
                                 break;
    }
}


/** Return the state of all the node's Outputs.
 */
void returnStates()
{
    uint8_t states = 0;

    // Build a response from all the Output's states.
    for (uint8_t pin = 0, mask = 1; pin < OUTPUT_PIN_MAX; pin++, mask <<= 1)
    {
        if (outputDefs[pin].getState())
        {
            states |= mask;
        }
    }
    
    i2cComms.sendByte(states);

    if (isDebug(DEBUG_BRIEF))
    {
        Serial.print(millis());
        Serial.print(CHAR_TAB);
        Serial.print(PGMT(M_DEBUG_STATES));
        Serial.print(CHAR_SPACE);
        Serial.print(states, HEX);
        Serial.println();
    }
}


/** Return the result of a renumber request.
 */
void returnRenumber()
{
    systemData.i2cModuleID = requestNode;
    saveSystemData();

    i2cComms.sendByte(getModuleId(false));

    // Now change our module ID.
    i2cComms.setId(getModuleId(true));
}


/** Return the requested pin's Output definition.
 */
void returnDef()
{
    if (isDebug(DEBUG_BRIEF))
    {
        outputDefs[requestOption].printDef(M_DEBUG_SEND, getModuleId(false), requestOption);
    }
    outputDefs[requestOption].write();
}


/** Data received.
 *  Process the command.
 */
void processReceipt(int aLen)
{
    if (aLen > 0)
    {
        // Read the command byte.
        uint8_t command = i2cComms.readByte();
        uint8_t option  = command & COMMS_OPTION_MASK;
        uint8_t pin     = option  & OUTPUT_PIN_MASK;
        uint8_t delay   = 0;
    
        command &= COMMS_COMMAND_MASK;

        if (isDebug(DEBUG_BRIEF))
        {
            Serial.println();
            Serial.print(millis());
            Serial.print(CHAR_TAB);
            Serial.print(PGMT(M_DEBUG_RECEIPT));
            Serial.print(PGMT(M_DEBUG_COMMAND));
            Serial.print(PGMT(M_DEBUG_COMMANDS[command >> COMMS_COMMAND_SHIFT]));
            Serial.print(PGMT(M_DEBUG_OPTION));
            Serial.print(option, HEX);
            Serial.print(PGMT(M_DEBUG_LEN));
            Serial.print(aLen, HEX);
            Serial.println();
        }

        switch (command)
        {
            case COMMS_CMD_SYSTEM: processSystem(option);
                                   break;
            case COMMS_CMD_DEBUG:  setDebug(option);                // Option is used for the debug level.
                                   saveSystemData();
                                   break;
            case COMMS_CMD_SET_LO:  
            case COMMS_CMD_SET_HI: if (i2cComms.available())
                                   {
                                       delay = i2cComms.readByte();    // Optional delay value.
                                   }
                                   actionState(pin, command == COMMS_CMD_SET_HI, delay, false);
                                   break;
            case COMMS_CMD_READ:   requestCommand = command;        // Record the command.
                                   requestOption  = option;         // and the pin the master wants to read.
                                   break;
            case COMMS_CMD_WRITE:  processWrite(pin);               // Process the Output's data.
                                   break;
            case COMMS_CMD_SAVE:   processSave(pin);                // Save the Output's data.
                                   break;
            case COMMS_CMD_RESET:  processReset(pin);               // Reset the Output.
                                   break;
            default:               unrecognisedCommand(M_DEBUG_RECEIPT, command, option);
                                   break;
        }
    }
    else
    {
        // Null receipt - Just the master seeing if we exist.
        if (isDebug(DEBUG_BRIEF))
        {
            Serial.println();
            Serial.print(millis());
            Serial.print(CHAR_TAB);
            Serial.print(PGMT(M_DEBUG_RECEIPT));
            Serial.print(CHAR_SPACE);
            Serial.print(PGMT(M_DEBUG_LEN));
            Serial.print(aLen, HEX);
            Serial.println();
        }
    }
    
    // Consume unexpected data.
    if (i2cComms.available())
    {
        if (isDebug(DEBUG_ERRORS))
        {
            Serial.println();
            Serial.print(millis());
            Serial.print(CHAR_TAB);
            Serial.print(PGMT(M_DEBUG_UNEXPECTED));
            Serial.print(PGMT(M_DEBUG_LEN));
            Serial.print(i2cComms.available(), HEX);
            Serial.print(CHAR_COLON);
        }
        
        while (i2cComms.available())
        {
            uint8_t ch = i2cComms.readByte();
            if (isDebug(DEBUG_ERRORS))
            {
                Serial.print(CHAR_SPACE);
                Serial.print(ch, HEX);
            }
        }
        if (isDebug(DEBUG_ERRORS))
        {
            Serial.println();
        }
    }
}


/** Process System command.
 */
void processSystem(uint8_t aOption)
{
    switch (aOption)
    {
        case COMMS_SYS_STATES:     requestCommand = COMMS_CMD_SYSTEM;
                                   requestOption  = aOption;
                                   break;
        case COMMS_SYS_RENUMBER:   requestCommand = COMMS_CMD_SYSTEM;
                                   requestOption  = aOption;
                                   processRenumber();
                                   break;
        case COMMS_SYS_MOVE_LOCKS: processMoveLocks();
                                   break;
        default:                   unrecognisedCommand(M_DEBUG_SYSTEM, COMMS_CMD_SYSTEM, aOption);
                                   break;
    }
}


/** Process a renumber request.
 */
void processRenumber()
{
    if (i2cComms.available())
    {
        requestNode = i2cComms.readByte();      // The desired new node number.
    }
    else
    {
        if (isDebug(DEBUG_ERRORS))
        {
            Serial.print(millis());
            Serial.print(CHAR_TAB);
            Serial.print(PGMT(M_DEBUG_RECEIPT));
            Serial.print(PGMT(M_DEBUG_COMMAND));
            Serial.print(PGMT(M_DEBUG_COMMANDS[requestCommand >> COMMS_COMMAND_SHIFT]));
            Serial.print(PGMT(M_DEBUG_OPTION));
            Serial.print(requestOption);
            Serial.print(PGMT(M_DEBUG_LEN));
            Serial.print(i2cComms.available());
            Serial.println();
        }
        
        // Revoke the request so it can't be actioned.
        requestCommand == COMMS_CMD_NONE;
    }
}


/** Process a move locks request
 */
void processMoveLocks()
{
    if (i2cComms.available() == OUTPUT_MOVE_LOCK_LEN)
    {
        // Read the old and new node numbers.
        uint8_t oldNode = i2cComms.readByte() & OUTPUT_NODE_MASK;
        uint8_t newNode = i2cComms.readByte() & OUTPUT_NODE_MASK;

        if (isDebug(DEBUG_DETAIL))
        {
            Serial.print(millis());
            Serial.print(CHAR_TAB);
            Serial.print(PGMT(M_DEBUG_MOVE));
            Serial.print(PGMT(M_DEBUG_NODE));
            Serial.print(oldNode, HEX);
            Serial.print(PGMT(M_DEBUG_TO));
            Serial.print(newNode, HEX);
            Serial.println();
        }

        // Move all locks referencing the old node number to the new node number.
        for (uint8_t pin = 0; pin < OUTPUT_PIN_MAX; pin++)
        {
            // Process Lo and Hi lock types.
            for (uint8_t hi = 0; hi < 2; hi++)
            {
                // Process all the locks of the type.
                for (uint8_t index = 0; index < OUTPUT_LOCK_MAX; index++)
                {
                    if (outputDefs[pin].getLockNode(hi, index) == oldNode)
                    {
                        outputDefs[pin].setLockNode(hi, index, newNode);
                    }
                    else if (outputDefs[pin].getLockNode(hi, index) == newNode)
                    {
                        outputDefs[pin].setLockNode(hi, index, oldNode);
                    }
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
            Serial.print(PGMT(M_DEBUG_RECEIPT));
            Serial.print(PGMT(M_DEBUG_COMMAND));
            Serial.print(PGMT(M_DEBUG_MOVE));
            Serial.print(PGMT(M_DEBUG_LEN));
            Serial.print(i2cComms.available());
            Serial.println();
        }
    }
}


/** Process write command.
 *  Read the Output
 *  Write the definition to the specified Output.
 */
void processWrite(uint8_t aPin)
{
    uint8_t oldType = outputDefs[aPin].getType();       // Remember old type.
    
    if (i2cComms.available() < OUTPUT_SIZE)
    {
        if (isDebug(DEBUG_ERRORS))
        {
            Serial.print(millis());
            Serial.print(CHAR_TAB);
            Serial.print(PGMT(M_DEBUG_WRITE));
            Serial.print(aPin, HEX);
            Serial.print(PGMT(M_DEBUG_LEN));
            Serial.print(i2cComms.available(), HEX);
            Serial.println();
        }
    }
    else
    {
        persisting = false;             // Stop saving state to EEPROM.
        outputDefs[aPin].read();        // Read the Output definition.
        initOutput(aPin, oldType);      // Initialise the pin.

        if (isDebug(DEBUG_BRIEF))
        {
            outputDefs[aPin].printDef(M_DEBUG_WRITE, getModuleId(false), aPin);
        }
    }
}


/** Process a save command.
 */
void processSave(uint8_t aPin)
{
    uint8_t oldType = outputDefs[aPin].getType();
    
    persisting = true;              // Resume saving output to EEPROM.
    saveOutput(aPin);               // And save the output.
    initOutput(aPin, oldType);      // Ensure output is initialised to new state.
    initFlasher(aPin);              // Ensure flasher is operating (or not).
}


/** Process a reset command.
 */
void processReset(uint8_t aPin)
{
    uint8_t oldType = outputDefs[aPin].getType();
    
    persisting = true;              // Resume saving output to EEPROM.
    loadOutput(aPin);               // And recover the output's definition.
    initOutput(aPin, oldType);      // Ensure output is initialised to new state.
    initFlasher(aPin);              // Ensure flasher is operating (or not).
}


/** Action the state change against the specified pin.
 *  Delay for aDelay seconds.
 *  If a Servo, and aUseValue is set, use its current position rather than Lo-Hi when calculating range of movement.
 */
void actionState(uint8_t aPin, boolean aState, uint8_t aDelay, boolean aUseValue)
{
    boolean newState = aState;      // Might want to change the state (some LED_4 and FLASHERS).
    
    if (isDebug(DEBUG_BRIEF))
    {
        Serial.print(millis());
        Serial.print(CHAR_TAB);
        Serial.print(PGMT(M_DEBUG_ACTION));
        Serial.print(aPin, HEX);    
        Serial.print(PGMT(M_DEBUG_STATE));
        Serial.print(PGMT(aState ? M_HI : M_LO));    
        Serial.print(PGMT(M_DEBUG_DELAY_TO));
        Serial.print(aDelay, HEX);    
        Serial.println();
    }

    // If there's an action pending for a Led, just make it happen now.
    if (   (outputDefs[aPin].isLed())
        && (millis() < outputs[aPin].delayTo)
        && (!aState))
    {
        outputs[aPin].delayTo = 0;
        if (isDoubleLed(aPin))
        {
            outputs[aPin - 1].delayTo = 0;
        }
    }
    else
    {
        // Set common parameters
        outputs[aPin].delayTo = millis() + DELAY_MULTIPLIER * aDelay;
        outputs[aPin].steps   = outputDefs[aPin].getPaceAsSteps() + 1;
        outputs[aPin].step    = 0;
    
        // Output type-specific parameters.
        if (outputDefs[aPin].isServo())
        {
            newState = actionServo(aPin, aState, aUseValue);
        }
        else if (outputDefs[aPin].isLed())
        {
            newState = actionLed(aPin, aState);
        }
        else if (outputDefs[aPin].isFlasher())
        {
            newState = actionFlasher(aPin, aState);
        }
        else
        {
            if (isDebug(DEBUG_ERRORS))
            {
                Serial.print(millis());
                Serial.print(CHAR_TAB);
                Serial.print(PGMT(M_UNKNOWN));
                Serial.print(aPin);
                Serial.print(PGMT(M_DEBUG_TYPE));
                Serial.print(PGMT(M_OUTPUT_TYPES[outputDefs[aPin].getType() & OUTPUT_TYPE_MASK]));
                Serial.println();
            }
        }
    
        // Set the Output to the new state.
        outputDefs[aPin].setState(newState);
        
        // Save the new state if persisting is enabled.
        if (persisting)
        {
            saveOutput(aPin);
        }
    
        if (isDebug(DEBUG_DETAIL))
        {
            reportOutput(M_DEBUG_MOVE, aPin);
        }
    }
}


/** Action a Servo state change.
 *  If aUseValue is set, use Servo's current position rather than Lo-Hi when calculating range of movement.
 */
boolean actionServo(uint8_t aPin, boolean aState, boolean aUseValue)
{
    // Set movement range.
    outputs[aPin].value    = outputs[aPin].servo.read();
    outputs[aPin].start    = (aUseValue ? outputs[aPin].value 
                                        : (aState ? outputDefs[aPin].getLo() 
                                                  : outputDefs[aPin].getHi()));
    outputs[aPin].target   = aState ? outputDefs[aPin].getHi() : outputDefs[aPin].getLo();
    outputs[aPin].altValue = 0;

    long range = abs(outputs[aPin].target - outputs[aPin].start);
    
    // Adjust steps in proportion to the Servo's range.
    outputs[aPin].steps = ((long)outputs[aPin].steps - 1)
                        * (range)
                        / ((long)OUTPUT_SERVO_MAX)
                        + 1;
                        
    // Adjust start step for Servos that are already in mid-travel.
    outputs[aPin].step = ((long)outputs[aPin].steps)
                       * (abs(((long)outputs[aPin].value) - ((long)outputs[aPin].start)))
                       / (range + 1);

    // Add trigger point for SIGNALS, but only if they're ascending the whole range.
    if (   (outputDefs[aPin].getType() == OUTPUT_TYPE_SIGNAL)
        && (aState)
        && (persisting)
        && (outputs[aPin].start == outputDefs[aPin].getLo())
        && (random(100) < SIGNAL_PAUSE_CHANCE))
    {
        outputs[aPin].altValue = (outputs[aPin].steps + random(outputs[aPin].steps)) / 3;
    }

    return aState;
}


/** Action a Led state change.
 */
boolean actionLed(uint8_t aPin, boolean aState)
{
    boolean newState = aState;      // Might want to change the state.

    // Start from current values.
    outputs[aPin].start     = outputs[aPin].value;
    outputs[aPin].altStart  = outputs[aPin].altValue;

    // Set target intensities.
    if (   (isDoubleLed(aPin))          // Handle LED_4/ROAD as special case (if preceding output is a LED).
        && (persisting))
    {
        newState = actionDoubleLed(aPin, aState);
    }
    else if (   (outputDefs[aPin].getType() == OUTPUT_TYPE_RANDOM)
             && (persisting))
    {
        if (aState)
        {
            // Set outputs on randomly.
            outputs[aPin].target    = (random(100) < RANDOM_HI_CHANCE) ? outputDefs[aPin].getHi() : 0;
            outputs[aPin].altTarget = (random(100) < RANDOM_LO_CHANCE) ? outputDefs[aPin].getLo() : 0;
        }
        else
        {
            outputs[aPin].target    = 0;
            outputs[aPin].altTarget = 0;
        }
    }
    else
    {
        // Ordinary LED
        outputs[aPin].target    = aState ? outputDefs[aPin].getHi() : 0;
        outputs[aPin].altTarget = aState ? 0 : outputDefs[aPin].getLo();
    }

    return newState;
}


/** Action a DoubleLed state change.
 */
boolean actionDoubleLed(uint8_t aPin, boolean aState)
{
    boolean newState = aState;      // Might want to change the state.
    boolean ledState = false;
    uint8_t ledPin   = aPin - 1;
    uint8_t offFlag  = (outputDefs[aPin].getType() == OUTPUT_TYPE_LED_4) ? LED_4_OFF : ROAD_OFF;
    uint8_t oldPhase = (outputDefs[ledPin].getState()     )     // Convert state of both outputs to a phase.
                     | (outputDefs[aPin]  .getState() << 1);
    uint8_t newPhase = oldPhase;

    // Set to phase=1 if Hi, otherwise next phase in sequence
    if (aState)
    {
        newPhase = 1;
    }
    else if (outputDefs[aPin].getType() == OUTPUT_TYPE_LED_4)
    {
        newPhase = LED_4_NEXT_PHASE[oldPhase];
    }
    else if (outputDefs[aPin].getType() == OUTPUT_TYPE_ROAD_UK)
    {
        newPhase = ROAD_UK_NEXT_PHASE[oldPhase];
    }
    else
    {
        newPhase = ROAD_RW_NEXT_PHASE[oldPhase];
    }

    // Set states according to new phase.
    // See table at top of source for states and colour sequences.
    ledState = newPhase & 1;
    newState = newPhase & 2;
    outputDefs[ledPin].setState(ledState);

    if (newPhase == oldPhase)
    {
        outputs[aPin].steps   = 0;                                  // Nothing to do.
        outputs[aPin].delayTo = 0;                                  // And nothing scheduled either.
    }
    else
    {
        // Set common parameters for both outputs.
        outputs[ledPin].step     = outputs[aPin].step;              // Both outputs move with the same steps.
        outputs[ledPin].steps    = outputs[aPin].steps;
        outputs[ledPin].delayTo  = outputs[aPin].delayTo;           // and at the same time.

        outputs[ledPin].start    = outputs[ledPin].value;           // LED movement common values.
        outputs[ledPin].altStart = outputs[ledPin].altValue;
             
        // Set targets according to new states.
        outputs[aPin].target      = newState ? outputDefs[aPin].getHi() : 0;
        outputs[aPin].altTarget   = newState ? 0 : outputDefs[aPin].getLo();
        outputs[ledPin].target    = ledState ? outputDefs[ledPin].getHi() : 0;
        outputs[ledPin].altTarget = ledState ? 0 : outputDefs[ledPin].getLo();

        // Force off if states demand it.
        if (offFlag & (0x10 << newPhase))
        {
            outputs[ledPin].target    = 0;
            outputs[ledPin].altTarget = 0;
        }
        if (offFlag & (0x01 << newPhase))
        {
            outputs[aPin].target    = 0;
            outputs[aPin].altTarget = 0;
        }
        
        // Save the new state if persisting is enabled.
        if (persisting)
        {
            saveOutput(ledPin);
        }
    }

    return newState;
}


/** Action a Flasher state change.
 */
boolean actionFlasher(uint8_t aPin, boolean aState)
{
    boolean newState = aState;      // Might want to change the state.
    
    // For a BLINK that's not running, force state on.
    if (   (outputDefs[aPin].getType() == OUTPUT_TYPE_BLINK)
        && (outputs[aPin].value    == 0)
        && (outputs[aPin].altValue == 0))
    {
        newState = true;
    }

    // Turn Flashers off immediately if state = Lo and an indefinite time (delay = 0).
    if (   (!aState)
        && (outputDefs[aPin].getReset() == 0))
    {
        newState = false;
        outputs[aPin].steps = 0;
        outputs[aPin].value = 0;
        if (outputDefs[aPin].getType() == OUTPUT_TYPE_BLINK)
        {
            outputs[aPin].altValue = 0;
        }
        else
        {
            outputs[aPin].altValue = outputDefs[aPin].getLo();
        }
    }
    else
    {
        // Flash as required.
        outputs[aPin].delayTo = outputDefs[aPin].getReset() == 0 ? 0 : millis() + DELAY_MULTIPLIER * outputDefs[aPin].getReset();
        outputs[aPin].step    = outputs[aPin].steps;
    }

    return newState;
}


/** Step all the Servos if necessary.
 */
void stepServos()
{
    // Move any Outputs that need moving.
    for (uint8_t pin = 0; pin < IO_PINS; pin++)
    {
        if (   (outputDefs[pin].isServo())
            && (outputs[pin].steps > 0))
        {
            if (   (outputs[pin].delayTo == 0)
                || (outputs[pin].delayTo <= now))
            {
                stepServo(pin);
            }
        }
    }
}


/** Step a Servo to its next position.
 */
void stepServo(uint8_t aPin)
{
//    // Report initial position if debug level high enough.
//    if (   (isDebug(DEBUG_FULL))
//        && (outputs[aPin].step == 0))
//    {
//        reportOutput(M_DEBUG_MOVE, aPin);
//    }

    // Handle SIGNAL triggers (if set).
    if (outputs[aPin].altValue)
    {
        if (outputs[aPin].step < outputs[aPin].altValue)            // Ascending to trigger step.
        {
            outputs[aPin].step += 1;
            if (outputs[aPin].step >= outputs[aPin].altValue)       // Reached the trigger step.
            {
                // Set new trigger back down a bit (up to 1/3).
                outputs[aPin].altValue -= 1 + random(outputs[aPin].step)
                                              * SIGNAL_PAUSE_PERCENTAGE
                                              / 100;
                outputs[aPin].delayTo = millis() + random(SIGNAL_PAUSE_DELAY);
                
                if (isDebug(DEBUG_DETAIL))
                {
                    reportOutput(M_DEBUG_TRIGGER, aPin);
                }
            }
        }
        else                                                        // Descending to trigger step.
        {
            outputs[aPin].step -= 1;
            if (outputs[aPin].step <= outputs[aPin].altValue)       // Have descended to trigger step.
            {
                outputs[aPin].altValue = 0;                         // Remove the trigger step.
                if (outputDefs[aPin].getState())
                {
                    outputs[aPin].delayTo = millis() + random(SIGNAL_PAUSE_RESTART);
                }
                
                if (isDebug(DEBUG_DETAIL))
                {
                    reportOutput(M_DEBUG_TRIGGER, aPin);
                }
            }
        }
    }
    else
    {
        // Normal non-triggered movement.
        outputs[aPin].step += 1;
    }

    // Calculate Servo's new position.
    if (outputs[aPin].step >= outputs[aPin].steps)
    {
        // Last step, make sure to hit the target bang-on.
        outputs[aPin].value = outputs[aPin].target;
        
        // No need to set pin, actioned later in the function.
        // digitalWrite(ioPins[aPin], outputDefs[aPin].getState());

        // Signals might "bounce" if descending
        if (   (persisting)
            && (outputDefs[aPin].getType() == OUTPUT_TYPE_SIGNAL)
            && (!outputDefs[aPin].getState())
            && (outputs[aPin].steps > 1)
            && (random(100) < SIGNAL_BOUNCE_CHANCE))
        {
            // Go back a little.
            outputs[aPin].altValue = outputs[aPin].steps - random(outputs[aPin].steps)
                                                           * SIGNAL_BOUNCE_PERCENTAGE
                                                           / 100;
            if (isDebug(DEBUG_DETAIL))
            {
                reportOutput(M_DEBUG_TRIGGER, aPin);
            }
        }
        else
        {
            // Stop further movement.
            outputs[aPin].steps = 0;
            
            // If there's a reset, reset the servo after the specified delay.
            if (   (persisting)
                && (outputDefs[aPin].getState())
                && (outputDefs[aPin].getReset() > 0))
            {
                actionState(aPin, false, outputDefs[aPin].getReset(), false);
            }
        }
    }
    else
    {
        // Intermediate step, move proportionately (step/steps) along the range (start to target).
        // Use long arithmetic to avoid overflow problems.
        outputs[aPin].value = ((long)outputs[aPin].start) 
                            +   (((long)outputs[aPin].target) - ((long)outputs[aPin].start))
                              * ((long)outputs[aPin].step)
                              / ((long)outputs[aPin].steps);
    }

    // Set (or unset) Servo's digital pad when we're over halfway
    if (outputDefs[aPin].getState())
    {
        // Only set pad when > half-way AND trigger has been handled.
        digitalWrite(ioPins[aPin],    (outputs[aPin].step > (outputs[aPin].steps >> 1))
                                   && (outputs[aPin].altValue == 0));
    }
    else
    {
        digitalWrite(ioPins[aPin], outputs[aPin].step <= (outputs[aPin].steps >> 1));
    }

    // Move Servo to new state.
    outputs[aPin].servo.write(outputs[aPin].value);

    // Report activity if debug level high enough.
    if (   (isDebug(DEBUG_FULL))
        || (   (isDebug(DEBUG_DETAIL))
            && (   (outputs[aPin].step == 1)
                || (outputs[aPin].step == outputs[aPin].steps))))
    {
        reportOutput(M_DEBUG_MOVE, aPin);
    }
}


/** Step all the Leds if necessary.
 */
void stepLeds()
{
    // Move any Leds that need moving.
    for (uint8_t pin = 0; pin < IO_PINS; pin++)
    {
        if (outputDefs[pin].isLed())
        {
            if (   (outputs[pin].delayTo == 0)
                || (outputs[pin].delayTo <= now))
            {
                stepLed(pin);
            }
        }
    }
}


/** Step a Led to its next intensity.
 */
void stepLed(uint8_t aPin)
{
    if (outputs[aPin].step < outputs[aPin].steps)
    {
        // Move to next step.
        outputs[aPin].step += 1;

        if (outputs[aPin].step >= outputs[aPin].steps)
        {
            // Last step, make sure to hit the target bang-on.
            outputs[aPin].value    = outputs[aPin].target;
            outputs[aPin].altValue = outputs[aPin].altTarget;

            // If there's a reset, reset the LED after the specified delay.
            if (   (persisting)
                && (outputDefs[aPin].getReset() > 0))
            {
                if (isDoubleLed(aPin))
                {
                    stepDoubleLed(aPin);
                }
                else if (outputDefs[aPin].getType() == OUTPUT_TYPE_RANDOM)              // Random.
                {
                    if (outputDefs[aPin].getState())                                    // If Hi, set Hi again (which may or may not illuminate LEDs).
                    {
                        actionState(aPin, true,   outputDefs[aPin].getReset() / 2       // Delay for reset +/- 1/2 reset.
                                                + random(outputDefs[aPin].getReset()),
                                                false);
                    }
                }
                else if (outputDefs[aPin].getState())                                   // Ordinary LED, currently Hi.
                {
                    if (!isDoubleLed(aPin + 1))                                         // And not part of a doubleLed.
                    {
                        actionState(aPin, false, outputDefs[aPin].getReset(), false);   // Move to Lo.
                    }
                }
            }
        }
        else
        {
            // Intermediate step, move proportionately (step/steps) along the range (start to target and altStart to altTarget).
            // Use long arithmetic to avoid overflow problems.
            outputs[aPin].value    = ((long)outputs[aPin].start) 
                                   +   (((long)outputs[aPin].target) - ((long)outputs[aPin].start))
                                     * ((long)outputs[aPin].step)
                                     / ((long)outputs[aPin].steps);
            outputs[aPin].altValue = ((long)outputs[aPin].altStart) 
                                   +   (((long)outputs[aPin].altTarget) - ((long)outputs[aPin].altStart))
                                     * ((long)outputs[aPin].step)
                                     / ((long)outputs[aPin].steps);
        }

        // Report activity if debug level high enough.
        if (   (isDebug(DEBUG_FULL))
            || (   (isDebug(DEBUG_DETAIL))
                && (   (outputs[aPin].step == 1)
                    || (outputs[aPin].step == outputs[aPin].steps))))
        {
            reportOutput(M_DEBUG_MOVE, aPin);
        }
    }
}


/** Step a DoubleLed (LED_4 or ROAD).
 */
void stepDoubleLed(uint8_t aPin)
{
    int     pin   = aPin;                                       // Pin (signed) to fire next (normally the same pin).
    uint8_t reset = outputDefs[aPin].getReset();                // Interval before next firing.
    uint8_t type  = outputDefs[aPin].getType();                 // Type of this pin.
    
    if (   (type == OUTPUT_TYPE_ROAD_UK)                        // ROAD outputs are a special case.
        || (type == OUTPUT_TYPE_ROAD_RW))
    {
        if (   (outputDefs[aPin    ].getState())                // at Red & Amber or Amber.
            && (outputDefs[aPin - 1].getReset() > 0))           // and has an alternate reset specified
        {
            reset = outputDefs[aPin - 1].getReset();            // Use that (normally shorter) reset instead.
        }
        else if (   ( outputDefs[aPin - 1].getState())          // Red is Hi and Lo
                 && (!outputDefs[aPin    ].getState()))
        {
            // See if next pair of pins are also a ROAD, or if previous pins are ROADs.
            pin = aPin + 2;
            if (   (!isDoubleLed(pin))
                || (outputDefs[pin].getType() != type))
            {
                // Next output isn't a ROAD, look for "first" one.
                for (pin = aPin - 2; pin > 0; pin -= 2)
                {
                    if (outputDefs[pin].getType() != type)
                    {
                        break;
                    }
                }
                pin += 2;
            }

            // If we found an adjacent ROAD output.
            if (pin != aPin)
            {
                // Fire that output next using its own reset interval(s).
                reset = outputDefs[pin - 1].getReset();
                if (reset == 0)
                {
                    reset = outputDefs[pin].getReset();
                }
            }
        }
    }
    
    // Move desired pin to next state after correct interval.
    actionState(pin, false, reset, false);
}


/** Step all the active FLASH/BLINK outputs.
 */
void stepFlashes()
{
    // Flash any Outputs that need flashing.
    for (uint8_t pin = 0; pin < IO_PINS; pin++)
    {
        if (outputDefs[pin].isFlasher())
        {
            if (outputs[pin].steps > 0)
            {
                stepFlash(pin);
            }
        }
    }
}


//// DEBUG - checking flickering
//long switches = 0;
//long stays = 0;


/** Flash the given output.
 */
void stepFlash(uint8_t aPin)
{
    outputs[aPin].step += 1;
    if (outputs[aPin].step >= outputs[aPin].steps)          // Time to switch to other LED.
    {
        if (   (   (outputs[aPin].delayTo > 0)
                && (outputs[aPin].delayTo < now))
            || (   (outputDefs[aPin].getType() == OUTPUT_TYPE_BLINK)
                && (outputDefs[aPin].getState() == 0)))
        {
            // Stop flashing.
            outputs[aPin].steps = 0;

            // Decide if to finish with output Hi or Lo
            if (   (outputDefs[aPin].getType() == OUTPUT_TYPE_FLASH)
                && (outputDefs[aPin].getState()))
            {
                outputs[aPin].value    = outputDefs[aPin].getHi();      // FLASHes finish Hi.
                outputs[aPin].altValue = 0;
            }
            else
            {
                outputs[aPin].value = 0;
                if (outputDefs[aPin].getType() == OUTPUT_TYPE_BLINK)
                {
                    outputs[aPin].altValue = 0;                         // BLINKs finish off.
                }
                else
                {
                    outputs[aPin].altValue = outputDefs[aPin].getLo();  // FLASHes finish Lo.
                }
            }

//            // DEBUG - report flickering metrics.
//            Serial.print("Flickering: switches=");
//            Serial.print(switches);
//            Serial.print(", stays=");
//            Serial.print(stays);
//            Serial.println();
//            switches = 0;
//            stays = 0;
        }
        else
        {
            boolean doSwitch = true;
            if (outputs[aPin].steps == 1)               // Fastest possible flash = flicker.
            {
                doSwitch = random(100) < LED_FLICKER_CHANCE;
                
//                // DEBUG - metrics for flickering
//                if (doSwitch)
//                {
//                    switches += 1;
//                }
//                else
//                {
//                    stays += 1;
//                }
            }

            if (doSwitch)
            {
                // Flash opposite way.
                if (outputs[aPin].value)
                {
                    outputs[aPin].value    = 0;
                    outputs[aPin].altValue = outputDefs[aPin].getLo();
                }
                else
                {
                    outputs[aPin].value    = outputDefs[aPin].getHi();
                    outputs[aPin].altValue = 0;
                }
            }

            outputs[aPin].step = 0;
        }

        // Report activity if debug level high enough.
        if (   (isDebug(DEBUG_FULL))
            || (   (isDebug(DEBUG_DETAIL))
                && (   (outputs[aPin].step == 1)
                    || (outputs[aPin].step == outputs[aPin].steps))))
        {
            reportOutput(M_DEBUG_MOVE, aPin);
        }
    }
}


//// Metrics.
//long start = 0;
//long count = 0;

//// DEBUG test marker
//int testRun = 0;

/** Main loop.
 */
void loop()
{
    // Record the time now
    now = millis();
    
//  // Metrics
//  count += 1;
//  if ((now - start) > 1000L)
//  {
//    Serial.println();
//    Serial.print(now);
//    Serial.print(": ");
//    Serial.print(count);
//    Serial.println();
//
//    count = 0;
//    start = now;
//  }
//
//    // DEBUG tests.
//    if (   (now > 5000)
//        && (testRun == 0))
//    {
//        test1();
//        testRun += 1;
//    }
//    if (   (now > 15000)
//        && (testRun == 1))
//    {
//        test2();
//        testRun += 1;
//    }

    // Every STEP_SERVO msecs, step the servos if necessary
    if (now > tickServo)
    {
        tickServo = now + STEP_SERVO;
        stepServos();
    }

    // Every STEP_LED msecs, step the LEDs if necessary
    if (now > tickLed)
    {
        tickLed = now + STEP_LED;
        stepLeds();
    }
    
    // Every STEP_FLASH msecs, step the FLASH/BLINKs if necessary
    if (now > tickFlash)
    {
        tickFlash = now + STEP_FLASH;
        stepFlashes();
    }

    // Set LED Outputs based on their intensity value/alt, using the ticker to generate a PWM signal.
    tickPwm += PWM_TICK;
    for (uint8_t pin = 0; pin < IO_PINS; pin++)
    {
        tickPwm += PWM_INC;
        if (   (outputDefs[pin].isLed())
            || (outputDefs[pin].isFlasher()))
        {
            // Use compliment of tickPwm for alt pin to remove the chance of both being on at once.
            digitalWrite(sigPins[pin],    (outputs[pin].value    >  0)
                                       && (outputs[pin].value    >= ( tickPwm & 0xff)));
            digitalWrite(ioPins[pin],     (outputs[pin].altValue >  0)
                                       && (outputs[pin].altValue >= (~tickPwm & 0xff)));
        }
//        // Example how to use PORTS to toggle pins directly.
//        // See OUTPUT_BASE_PIN and ioPins for output => pin mapping.
//        // PORTD - Arduino digital pins 0 -  7
//        // PORTB - Arduino digital pins 8 - 13
//        // PORTC - Arduino analog  pins 0 -  7.
//        // Servo pin 3 is Arduino pin 7 which is PORTD mask 0x80
//        if (pin == 3)
//        {
//            if (   outputs[pin].value    >  0 
//                && outputs[pin].value    >= ( nowMicros & 0xff))
//            {
//                PORTD |= 0x80;
//            }
//            else
//            {
//                PORTD &= 0x7f;
//            }
//        }
    }
}
