/** OutputModule.
 */

#include <EEPROM.h>
#include <Servo.h>
#include <Wire.h>

#include "Config.h"
#include "Messages.h"
#include "Debug.h"
#include "System.h"
#include "Common.h"
#include "Memory.h"
#include "Comms.h"
#include "Output.h"


// Should changes be persisted?
boolean persisting = true;


// Ticking
long now       = 0;
long nowMicros = micros();
long tickServo = 0;
long tickLed   = 0;
long tickFlash = 0;

// i2c request command.
uint8_t requestCmd = COMMS_CMD_NONE;
uint8_t requestPin = 0;


// An Array of Output control structures.
struct 
{
    Servo   servo;          // The Servo (if there is one).
    uint8_t steps  = 0;     // The number of steps to take.
    uint8_t step   = 0;     // The current step.
    uint8_t value  = 0;     // The value of the output.
    uint8_t alt    = 0;     // The value of the alternate output.
    long    delay  = 0;     // Delay start to this time.
} outputs[IO_PINS];


/** Setup the Arduino.
 */
void setup()
{
    initialise();

    // Configure the Jumper pins for input.
    for (int pin = 0; pin < JUMPER_PINS; pin++)
    {
        pinMode(jumperPins[pin], INPUT_PULLUP);
    }

    // Configure the IO pins for output.
    for (int pin = 0; pin < IO_PINS; pin++)
    {
        pinMode(OUTPUT_BASE_PIN + pin, OUTPUT);
        pinMode(ioPins[pin], OUTPUT);
    }

    // Load SystemData from EEPROM and check it's valid.
    if (loadSystemData())
    {
        firstRun();
    }
    else
    {
//        systemData.i2cModuleID = systemData.i2cOutputBaseID + 8;    // Hard-code module ID.
//        saveSystemData();

        // Recover state from EEPROM.
        for (uint8_t pin = 0; pin < IO_PINS; pin++)
        {
            loadOutput(pin);
            initOutput(pin, OUTPUT_TYPE_NONE);
        }
    }

    // Start i2c communications.
    Wire.begin(systemData.i2cModuleID);
    Wire.onReceive(processReceipt);
    Wire.onRequest(processRequest);

    if (isDebug(DEBUG_NONE))
    {
        Serial.print("Module ID: ");
        Serial.println(systemData.i2cModuleID, HEX);
    }
}


/** Initialise data when first run.
 */
void firstRun()
{
    Serial.println("FirstRun");
    
    // Initialise SystemData.
    systemData.magic   = MAGIC_NUMBER;
    systemData.version = VERSION;

    systemData.i2cControllerID = DEFAULT_I2C_CONTROLLER_ID;
    systemData.i2cInputBaseID  = DEFAULT_I2C_INPUT_BASE_ID;
    systemData.i2cOutputBaseID = DEFAULT_I2C_OUTPUT_BASE_ID;
    systemData.i2cModuleID     = systemData.i2cOutputBaseID;

    // Configure i2c from jumpers.
    for (int pin = 0, mask=1; pin < JUMPER_PINS; pin++, mask <<= 1)
    {
        if (   (   (jumperPins[pin] >= ANALOG_PIN_FIRST)
                && (analogRead(jumperPins[pin]) > ANALOG_PIN_CUTOFF))
            || (   (jumperPins[pin] <  ANALOG_PIN_FIRST)
                && (false)      // TODO - handle digital pins on TxRx
                && (digitalRead(jumperPins[pin]))))
        {
            systemData.i2cModuleID |= mask;
        }
//        Serial.print("Jumper ");
//        Serial.print(jumperPins[pin], HEX);
//        Serial.print(": digital=");
//        Serial.print(digitalRead(jumperPins[pin]), HEX);
//        Serial.print(", analog=");
//        Serial.print(analogRead(jumperPins[pin]), HEX);
//        Serial.print(". ID=");
//        Serial.print(systemData.i2cModuleID, HEX);
//        Serial.println();
    }

    
    // Initialise EEPROM with suitable data.
    for (uint8_t pin = 0; pin < IO_PINS; pin++)
    {
        outputDefs[pin].setType(OUTPUT_TYPE_NONE);
        outputDefs[pin].setLo(OUTPUT_DEFAULT_LO);
        outputDefs[pin].setHi(OUTPUT_DEFAULT_HI);
        outputDefs[pin].setPace(OUTPUT_DEFAULT_PACE);
        outputDefs[pin].setDelay(OUTPUT_DEFAULT_DELAY);
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
    Serial.print(outputDefs[aPin].getType(), HEX);
    Serial.print(PGMT(M_DEBUG_STATE));
    Serial.print(outputDefs[aPin].getState(), HEX);
    Serial.print(PGMT(M_DEBUG_TARGET));
    Serial.print(outputDefs[aPin].getTarget(), HEX);
    Serial.print(PGMT(M_DEBUG_ALT_TARGET));
    Serial.print(outputDefs[aPin].getAltTarget(), HEX);
    Serial.print(PGMT(M_DEBUG_STEPS));
    Serial.print(outputs[aPin].steps, HEX);
    Serial.print(PGMT(M_DEBUG_STEP));
    Serial.print(outputs[aPin].step, HEX);
    Serial.print(PGMT(M_DEBUG_VALUE));
    Serial.print(outputs[aPin].value, HEX);
    Serial.print(PGMT(M_DEBUG_ALT));
    Serial.print(outputs[aPin].alt, HEX);
    Serial.print(PGMT(M_DEBUG_DELAY));
    Serial.print(outputs[aPin].delay);
    Serial.println();
}


/** Initialise an Output.
 *  Detach/attach servo as necessary.
 *  Set outputs entry if movement necessary.
 */
void initOutput(int aPin, uint8_t aOldType)
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
            actionState(aPin, outputDefs[aPin].getState(), 0);
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
            outputs[aPin].servo.attach(OUTPUT_BASE_PIN + aPin);
        }
    }
    else if (   (outputDefs[aPin].isLed())
             || (outputDefs[aPin].isFlasher()))
    {
        // Ensure LEDs glow with correct intensity.
        if (outputDefs[aPin].getState())
        {
            outputs[aPin].value = outputDefs[aPin].getHi();
            outputs[aPin].alt   = 0;
        }
        else
        {
            outputs[aPin].value = 0;
            outputs[aPin].alt   = outputDefs[aPin].getLo();
        }
        outputs[aPin].steps = 0;            // Ensure there's no flashing

        // TODO - Ensure BLINK Outputs blink if they're Hi and have delay-0 (indefinite).
        // But only at start-up, not when configuring.
    }
    else
    {
        // All other outputs, turn pins off. 
        digitalWrite(OUTPUT_BASE_PIN + aPin, LOW);
        digitalWrite(ioPins[aPin], LOW);
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
        Serial.print(requestPin, HEX);
        Serial.print(PGMT(M_DEBUG_CMD));
        Serial.print(requestCmd, HEX);
        Serial.println();
    }
    
    switch (requestCmd)
    {
        case COMMS_CMD_STATES: returnStates();
                               break;
        case COMMS_CMD_READ:   returnDef();
                               break;
        default:               Serial.print("\tUnknown command: ");
                               Serial.println(requestCmd);

    }

    // Clear pending command.
    requestCmd = COMMS_CMD_NONE;
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
    
    Wire.write(states);

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


/** Return the requested pin's Output definition.
 */
void returnDef()
{
    if (isDebug(DEBUG_FULL))
    {
        outputDefs[requestPin].printDef(M_DEBUG_SEND, requestPin);
    }
    outputDefs[requestPin].write();
}


/** Data received.
 *  Process the command.
 */
void processReceipt(int aLen)
{
    if (aLen > 0)
    {
        uint8_t command = Wire.read();
        uint8_t pin     = command & COMMS_PIN_MASK;
    
        command &= COMMS_CMD_MASK;

        if (isDebug(DEBUG_BRIEF))
        {
            Serial.println();
            Serial.print(millis());
            Serial.print(CHAR_TAB);
            Serial.print(PGMT(M_DEBUG_RECEIPT));
            Serial.print(pin, HEX);
            Serial.print(PGMT(M_DEBUG_LEN));
            Serial.print(aLen, HEX);
            Serial.print(PGMT(M_DEBUG_CMD));
            Serial.print(command, HEX);
            Serial.println();
        }
            
        switch (command)
        {
            case COMMS_CMD_STATES: requestCmd = command;    // Record the command.
                                   // requestPin = pin;     // Not interested in the pin.
                                   break;
            case COMMS_CMD_DEBUG:  setDebug(pin);           // Pin is used for the debug level.
                                   saveSystemData();
                                   break;
            case COMMS_CMD_SET_LO:  
            case COMMS_CMD_SET_HI: if (Wire.available())
                                   {
                                       // Use delay sent with request.
                                       actionState(pin, command == COMMS_CMD_SET_HI, Wire.read());
                                   }
                                   else
                                   {
                                       // Use delay from Output's definition.
                                       actionState(pin, command == COMMS_CMD_SET_HI, outputDefs[pin].getDelay());
                                   }
                                   break;
            case COMMS_CMD_READ:   requestCmd = command;        // Record the command.
                                   requestPin = pin;            // and the pin the master wants to read.
                                   break;
            case COMMS_CMD_WRITE:  processWrite(pin, false);    // Process the Output's data.
                                   break;
            case COMMS_CMD_SAVE:   processWrite(pin, true);     // Process the Output's data and save it.
                                   break;
            case COMMS_CMD_RESET:  processReset(pin);
                                   break;
            default:               if (isDebug(DEBUG_NONE))
                                   {
                                       Serial.print(millis());
                                       Serial.print("\tUnrecognised command: ");
                                       Serial.println(command, HEX);
                                   }
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
    if (Wire.available())
    {
        if (isDebug(DEBUG_NONE))
        {
            Serial.println();
            Serial.print(millis());
            Serial.print(CHAR_TAB);
            Serial.print(PGMT(M_DEBUG_UNEXPECTED));
            Serial.print(PGMT(M_DEBUG_LEN));
            Serial.print(Wire.available(), HEX);
            Serial.print(CHAR_COLON);
        }
        
        while (Wire.available())
        {
            uint8_t ch = Wire.read();
            if (isDebug(DEBUG_NONE))
            {
                Serial.print(CHAR_SPACE);
                Serial.print(ch, HEX);
            }
        }
        if (isDebug(DEBUG_NONE))
        {
            Serial.println();
        }
    }
}


/** Process write command.
 *  Read the Output
 *  Write the definition to the specified Output.
 */
void processWrite(uint8_t aPin, boolean aSave)
{
    uint8_t oldType = outputDefs[aPin].getType();       // Remember old type.
    
    if (Wire.available() < COMMS_LEN_WRITE)
    {
        if (isDebug(DEBUG_NONE))
        {
            Serial.print(millis());
            Serial.print(CHAR_TAB);
            Serial.print(aSave ? PGMT(M_DEBUG_SAVE) : PGMT(M_DEBUG_WRITE));
            Serial.print(aPin, HEX);
            Serial.print(PGMT(M_DEBUG_LEN));
            Serial.print(Wire.available(), HEX);
            Serial.println();
        }
    }
    else
    {
        persisting = aSave;         // Change persisting state based on Save vs Write.

        // Read the Output definition and save it.
        outputDefs[aPin].read();
        initOutput(aPin, oldType);

        if (aSave)
        {
            saveOutput(aPin);
        }
        else
        {
            outputDefs[aPin].printDef(M_DEBUG_WRITE, aPin);
        }
    }
}


/** Process a reset command.
 */
void processReset(uint8_t aPin)
{
    uint8_t oldType = outputDefs[aPin].getType();
    
    persisting = true;    
    loadOutput(aPin);
    initOutput(aPin, oldType);
}


/** Action the state change against the specified pin.
 */
void actionState(uint8_t aPin, uint8_t aState, uint8_t aDelay)
{
    if (isDebug(DEBUG_BRIEF))
    {
        Serial.print(millis());
        Serial.print(CHAR_TAB);
        Serial.print(PGMT(M_DEBUG_ACTION));
        Serial.print(aPin, HEX);    
        Serial.print(PGMT(M_DEBUG_STATE));
        Serial.print(aState, HEX);    
        Serial.print(PGMT(M_DEBUG_DELAY));
        Serial.print(aDelay, HEX);    
        Serial.println();
    }
    
    outputDefs[aPin].setState(aState);
    
    // Calculate steps and starting step.
    outputs[aPin].step = 0;
    if (outputDefs[aPin].isServo())
    {
        outputs[aPin].value = outputs[aPin].servo.read();
        uint32_t steps = abs(outputDefs[aPin].getTarget() - outputs[aPin].value);
        if (steps > OUTPUT_SERVO_MAX)
        {
            steps = OUTPUT_SERVO_MAX;
        }
        steps = steps * outputDefs[aPin].getPaceAsSteps()
                      / OUTPUT_SERVO_MAX;
        outputs[aPin].steps = steps + 1;
    }
    else if (outputDefs[aPin].isLed())
    {
        outputs[aPin].steps = outputDefs[aPin].getPaceAsSteps() + 1;
    }
    else if (outputDefs[aPin].isFlasher())
    {
        // For a BLINK that's not running, force state on.
        if (   (outputDefs[aPin].getType() == OUTPUT_TYPE_BLINK)
            && (outputs[aPin].steps == 0))
        {
            outputDefs[aPin].setState(true);
        }

        // Turn Flashers off immediately if state = Lo and an indefinite time (delay = 0).
        if (   (!outputDefs[aPin].getState())
            && (aDelay == 0))
        {
            outputs[aPin].steps = 0;
        }
        else
        {
            // Flash as required.
            outputs[aPin].steps = outputDefs[aPin].getPaceAsSteps() + 1;
            outputs[aPin].step  = outputs[aPin].steps;
        }
    }
    else
    {
        if (isDebug(DEBUG_NONE))
        {
            Serial.print(millis());
            Serial.print(CHAR_TAB);
            Serial.print(PGMT(M_DEBUG_UNKNOWN));
            Serial.print(aPin);
            Serial.print(PGMT(M_DEBUG_TYPE));
            Serial.print(outputDefs[aPin].getType());
            Serial.println();
        }
    }

    // Set the Output's movement characteristics.
    outputs[aPin].delay  = (aDelay == 0 ? 0 : millis() + DELAY_MULTIPLIER * aDelay);

    // Save the new state if persisting is enabled.
    if (persisting)
    {
        saveOutput(aPin);
    }

    if (isDebug(DEBUG_FULL))
    {
        reportOutput(M_DEBUG_MOVE, aPin);
    }
}


/** Step all the Servos if necessary.
 */
void stepServos()
{
    // Move any Outputs that need moving.
    for (int pin = 0; pin < IO_PINS; pin++)
    {
        if (outputDefs[pin].isServo())
        {
            if (   (outputs[pin].delay == 0)
                || (outputs[pin].delay <= now))
            {
                stepServo(pin);
            }
        }
    }
}


/** Step a Servo to its next position.
 */
void stepServo(int aPin)
{
    if (outputs[aPin].step < outputs[aPin].steps)
    {
//        // Report initial position if debug level high enough.
//        if (   (isDebug(DEBUG_FULL))
//            && (outputs[aPin].step == 0))
//        {
//            reportOutput(M_DEBUG_MOVE, aPin);
//        }

        outputs[aPin].step += 1;

        // Calculate Servo's new position.
        if (outputs[aPin].step == outputs[aPin].steps)
        {
            // Last step, make sure to hit the target bang-on.
            outputs[aPin].value = outputDefs[aPin].getTarget();
            digitalWrite(ioPins[aPin], outputDefs[aPin].getState());
        }
        else
        {
            // Intermediate step, move proportionately (step/steps) along the range (start to target).
//            outputs[aPin].value = outputDefs[aPin].getAltTarget() 
//                                +   (outputDefs[aPin].getTarget() - outputDefs[aPin].getAltTarget())
//                                  * outputs[aPin].step
//                                  / outputs[aPin].steps;
            outputs[aPin].value += (outputDefs[aPin].getTarget() - outputs[aPin].value)
                                 / (outputs[aPin].steps + 1 - outputs[aPin].step);
        }

        // Set (or unset) Servo's digital pad when we're over halfway
        if (outputDefs[aPin].getState())
        {
            digitalWrite(ioPins[aPin], outputs[aPin].step >  (outputs[aPin].steps >> 1));
        }
        else
        {
            digitalWrite(ioPins[aPin], outputs[aPin].step <= (outputs[aPin].steps >> 1));
        }

        // Move Servo to new state.
        outputs[aPin].servo.write(outputs[aPin].value);
        digitalWrite(LED_BUILTIN, HIGH);                    // Indicate work in progress;

        // Report activity if debug level high enough.
        if (   (isDebug(DEBUG_DETAIL))
            || (   (isDebug(DEBUG_FULL))
                && (   (outputs[aPin].step == 1)
                    || (outputs[aPin].step == outputs[aPin].steps))))
        {
            reportOutput(M_DEBUG_MOVE, aPin);
        }
    }
}


/** Step all the Leds if necessary.
 */
void stepLeds()
{
    // Move any Leds that need moving.
    for (int pin = 0; pin < IO_PINS; pin++)
    {
        if (outputDefs[pin].getType() == OUTPUT_TYPE_LED)
        {
            if (   (outputs[pin].delay == 0)
                || (outputs[pin].delay <= now))
            {
                stepLed(pin);
            }
        }
    }
}


/** Step a Led to its next intensity.
 */
void stepLed(int aPin)
{
    if (outputs[aPin].step < outputs[aPin].steps)
    {
        outputs[aPin].step += 1;

        if (outputs[aPin].step >= outputs[aPin].steps)
        {
            // Last step, make sure to hit the target bang-on.
            if (outputDefs[aPin].getState())
            {
                outputs[aPin].value = outputDefs[aPin].getTarget();
                outputs[aPin].alt   = 0;
            }
            else
            {
                outputs[aPin].value = 0;
                outputs[aPin].alt   = outputDefs[aPin].getTarget();
            }
        }
        else
        {
            // Intermediate step, move proportionately (step/steps) along the range (start to target).
            if (outputDefs[aPin].getState())
            {
                outputs[aPin].value  = outputDefs[aPin].getTarget() * outputs[aPin].step / outputs[aPin].steps;
                outputs[aPin].alt   -= outputs[aPin].alt   / (outputs[aPin].steps + 1 - outputs[aPin].step);
            }
            else
            {
                outputs[aPin].value -= outputs[aPin].value / (outputs[aPin].steps + 1 - outputs[aPin].step);
                outputs[aPin].alt    = outputDefs[aPin].getTarget() * outputs[aPin].step / outputs[aPin].steps;
            }
        }

        // Report activity if debug level high enough.
        if (   (isDebug(DEBUG_DETAIL))
            || (   (isDebug(DEBUG_FULL))
                && (   (outputs[aPin].step == 1)
                    || (outputs[aPin].step == outputs[aPin].steps))))
        {
            reportOutput(M_DEBUG_MOVE, aPin);
        }
    }
}


/** Step all the active FLASH/BLINK outputs.
 */
void stepFlashes()
{
    // Flash any Outputs that need flashing.
    for (int pin = 0; pin < IO_PINS; pin++)
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
    if (outputs[aPin].step >= outputs[aPin].steps)
    {
        if (   (   (outputs[aPin].delay > 0)
                && (outputs[aPin].delay < now))
            || (   (outputDefs[aPin].getType() == OUTPUT_TYPE_BLINK)
                && (outputDefs[aPin].getState() == 0)))
        {
            // Stop flashing.
            outputs[aPin].steps = 0;

            // Decide if to finish with output Hi or Lo
            if (   (outputDefs[aPin].getType() == OUTPUT_TYPE_FLASH)
                && (outputDefs[aPin].getState()))
            {
                outputs[aPin].value = outputDefs[aPin].getHi();
                outputs[aPin].alt   = 0;
            }
            else
            {
                outputs[aPin].value = 0;
                outputs[aPin].alt   = outputDefs[aPin].getLo();
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
            if (outputs[aPin].steps == 1)           // Fastest possible flash = flicker.
            {
                doSwitch = (micros() & 0xc) == 0;   // One chance in four
                
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
                    outputs[aPin].value = 0;
                    outputs[aPin].alt   = outputDefs[aPin].getLo();
                }
                else
                {
                    outputs[aPin].value = outputDefs[aPin].getHi();
                    outputs[aPin].alt   = 0;
                }
            }

            outputs[aPin].step = 0;
        }

        // Report activity if debug level high enough.
        if (   (isDebug(DEBUG_DETAIL))
            || (   (isDebug(DEBUG_FULL))
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
    now       = millis();
    nowMicros = micros();
    
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
    if ((now - tickServo) > STEP_SERVO)
    {
        tickServo = now;
        digitalWrite(LED_BUILTIN, LOW);       // Assume no work in progress.
        stepServos();
    }

    // Every STEP_LED msecs, step the LEDs if necessary
    if ((now - tickLed) > STEP_LED)
    {
        tickLed = now;
        stepLeds();
    }
    
    // Every STEP_FLASH msecs, step the FLASH/BLINKs if necessary
    if ((now - tickFlash) > STEP_FLASH)
    {
        tickFlash = now;
        stepFlashes();
    }
    
    // Set LED Outputs based on their intensity value/alt, using the clock to generate a PWM signal.
    for (int pin = 0; pin < IO_PINS; pin++)
    {
        if (   (outputDefs[pin].isLed())
            || (outputDefs[pin].isFlasher()))
        {
            digitalWrite(OUTPUT_BASE_PIN + pin,    outputs[pin].value >  0 
                                                && outputs[pin].value >= (nowMicros & 0xff));
            digitalWrite(ioPins[pin],              outputs[pin].alt   >  0 
                                                && outputs[pin].alt   >= (nowMicros & 0xff));
//            // DEBUG
//            if (pin == 1)
//            {
//                digitalWrite(LED_BUILTIN,    outputs[pin].value >  0 
//                                          && outputs[pin].value >= (nowMicros & 0xff));
//            }
        }
    }
}


void test1()
{
//    Serial.println();

    // outputDef.set(aType, aState, aLo, aHi, aPace, aDelay)
//    outputDefs[0].set(OUTPUT_TYPE_SERVO, false,   3, 180,  0xd, 1);   saveOutput(0);
//    outputDefs[1].set(OUTPUT_TYPE_SERVO, false,   4, 179,  0xc, 2);   saveOutput(1);
//
//    actionReceipt(0, true, 0);
//    actionReceipt(1, true, 3);
    
//    outputDefs[pin].set(OUTPUT_TYPE_LED,   false, 100, 100,  0xc, 1);   saveOutput(pin);   actionReceipt(pin++, true, 1);
//    outputDefs[pin].set(OUTPUT_TYPE_FLASH, false, 255,   4,  0xf, 4);   saveOutput(pin);   actionReceipt(pin++, true, 4);
//    outputDefs[pin].set(OUTPUT_TYPE_BLINK, false,  23, 183,  0xe, 4);   saveOutput(pin);   actionReceipt(pin++, true, 4);
    
//    Serial.println();
}


void test2()
{
//    Serial.println();
//
//    actionReceipt(0, false, outputDefs[0].getDelay());
//    actionReceipt(1, false, outputDefs[1].getDelay());
//    
//    Serial.println();   
}
