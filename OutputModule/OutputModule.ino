
/** OutputModule.
 */


#include <EEPROM.h>
#include <Servo.h>
#include <Wire.h>

#include "Messages.h"
#include "Common.h"
#include "System.h"


// Servo state saved in EEPROM
#define SYSTEM_BASE   0                                   // SystemData goes here
#define TYPE_BASE     SYSTEM_BASE + sizeof(systemData)    // Base of Output type data.
#define STATE_BASE    TYPE_BASE   + sizeof(outputTypes)   // Base of Output state data.
#define IO_BASE       STATE_BASE  + sizeof(outputStates)  // Base of IO states data. 
#define EEPROM_END    IO_BASE     + sizeof(ioStates)      // Size of EEPROM


#define DELAY_STEP          50   // Delay (msecs) between steps of a Servo.
#define MAX_PACE           124   // Maximum pace value.
#define PACE_STEPS         128   // Pace adjustment when converting to steps.

#define SERVO_BASE_PIN       4   // Servos attached to this pin and the next 7 more.

#define JUMPER_PINS          4   // Four jumpers.
#define IO_PINS              8   // Eight IO pins.

#define DELAY_MULTIPLIER  1000   // Multiply delay values by this amount (convert to seconds).


// The module jumper pins
const uint8_t jumperPins[JUMPER_PINS] = { 1, 0, A6, A7 };

// The digital IO pins.
const uint8_t ioPins[IO_PINS]      = { 3, 2, A3, A2, A1, A0, 13, 12 };


// EEPROM persistance of output pin types.
uint8_t outputTypes[IO_PINS]  = { OUTPUT_TYPE_NONE, OUTPUT_TYPE_NONE, OUTPUT_TYPE_NONE, OUTPUT_TYPE_NONE, 
                                  OUTPUT_TYPE_NONE, OUTPUT_TYPE_NONE, OUTPUT_TYPE_NONE, OUTPUT_TYPE_NONE };

// EEPROM persistance of Output states.
uint8_t outputStates[IO_PINS] = { 90, 90, 90, 90, 90, 90, 90, 90 };

// EEPROM persistance of IO states.
uint8_t ioStates[IO_PINS]     = { 0, 0, 0, 0, 0, 0, 0, 0 }; 


// The i2c ID of the module.
uint8_t moduleID = 0;

// An Array of Output control structures.
struct 
{
    Servo   servo;          // The Servo.
    uint8_t start  = 0;     // The angle we started at.
    uint8_t target = 0;     // The angle we want to reach.
    uint8_t steps  = 0;     // The number of steps to take.
    uint8_t step   = 0;     // The current step.
    uint8_t state  = 0;     // The state to set the output to.
    uint8_t alt    = 0;     // The state to set the alternate output to.
    long    delay  = 0;     // Delay start to this time.
} outputs[IO_PINS];


/** Initialise data when first run.
 */
void firstRun()
{
    // Initialise SystemData.
    systemData.magic   = MAGIC_NUMBER;
    systemData.version = VERSION;

    systemData.i2cControllerID = DEFAULT_I2C_CONTROLLER_ID;
    systemData.i2cInputBaseID  = DEFAULT_I2C_INPUT_BASE_ID;
    systemData.i2cOutputBaseID = DEFAULT_I2C_OUTPUT_BASE_ID;

    // Initialise EEPROM with suitable data.
    EEPROM.put(TYPE_BASE,   outputTypes);
    EEPROM.put(STATE_BASE,  outputStates);
    EEPROM.put(IO_BASE,     ioStates);
    
    EEPROM.put(SYSTEM_BASE, systemData);
}


/** Set the Type of a pin.
 *  Remove/disconnect previous type if necessary.
 */
void setPinType(int aPin, uint8_t aType)
{
    if (aType != outputTypes[aPin])
    {
        // Remove/disable old type.
        if (   (outputTypes[aPin] == OUTPUT_TYPE_SERVO)
            || (outputTypes[aPin] == OUTPUT_TYPE_SIGNAL))
        {
            // Detach servo.
            outputs[aPin].servo.detach();
        }

        // Record the change.
        outputTypes[aPin] = aType;
        EEPROM.put(TYPE_BASE, outputTypes);
    }

    // Establish new type.
    if (   (aType == OUTPUT_TYPE_SERVO)
        || (aType == OUTPUT_TYPE_SIGNAL))
    {
        outputs[aPin].servo.write(outputStates[aPin]);
        outputs[aPin].servo.attach(SERVO_BASE_PIN + aPin);
    }
}


/** Setup the Arduino.
 */
void setup()
{
    initialise();

    // Load SystemData from EEPROM and check it's valid.
    EEPROM.get(SYSTEM_BASE, systemData);
    if (systemData.magic != MAGIC_NUMBER)
    {
        Serial.println("FirstRun");
        firstRun();
    }
    else
    {
        // Recover state from EEPROM.
        EEPROM.get(TYPE_BASE,  outputTypes);
        EEPROM.get(STATE_BASE, outputStates);
        EEPROM.get(IO_BASE,    ioStates);
    }

    // DEBUG - move a servo
    outputTypes[1]   = OUTPUT_TYPE_SERVO;
    outputStates[1]  = 0x0;
    outputs[1].state = 0x0;
    ioStates[1]      = 0x0;
    // moveServo(pin, angle, pace, state, delay);
    moveServo(1, 180, 0xc, 1, 0);
    
    // Report state from EEPROM
    for (int pin = 0; pin < IO_PINS; pin++)
    {
        Serial.print("Init ");
        Serial.print(pin);
        Serial.print(" type 0x");
        Serial.print(outputTypes[pin], HEX);
        Serial.print(" state ");
        Serial.print(outputStates[pin]);
        Serial.print(" io ");
        Serial.print(ioStates[pin] ? "Hi" : "Lo");
        Serial.println();
    }

    // Configure the Jumper pins for input.
    for (int pin = 0; pin < JUMPER_PINS; pin++)
    {
        pinMode(jumperPins[pin], INPUT_PULLUP);
    }

    // Configure the IO pins for output.
    for (int pin = 0; pin < IO_PINS; pin++)
    {
        pinMode(SERVO_BASE_PIN + pin, OUTPUT);
        pinMode(ioPins[pin], OUTPUT);
    }

    // Configure i2c from jumpers.
    for (int pin = 0; pin < JUMPER_PINS; pin++)
    {
        moduleID |= digitalRead(jumperPins[pin]) << pin;
    }
    moduleID |= systemData.i2cOutputBaseID;

    // Initialise all the outputs (from state saved in EEPROM).
    for (int pin = 0; pin < IO_PINS; pin++)
    {
        setPinType(pin, outputTypes[pin]);  
    }
    
    // Start i2c communications.
    Wire.begin(moduleID);
    Wire.onReceive(processRequest);

    Serial.print("Module ID: 0x");
    Serial.println(moduleID, HEX);
}


/** Upon receipt of a request, store it in the corresponding Output's state.
 */
void processRequest(int aLen)
{
    if (aLen < 4)
    {
        Serial.print("Len: ");
        Serial.print(aLen);
    }
    else if (Wire.available() < 4)
    {
        Serial.print("Avail: ");
        Serial.print(Wire.available());
    }
    else
    {
        Serial.print("Req: ");
        Serial.println(aLen);
    
        uint8_t pin   = Wire.read();
        uint8_t type  = 0;
        uint8_t angle = Wire.read();
        uint8_t pace  = Wire.read();
        uint8_t state = Wire.read();  
        uint8_t delay = 0;
        if (Wire.available())
        {
            delay = Wire.read();
        }

        // Extract type and pin from the combined type/pin byte.
        type = (pin >> OUTPUT_TYPE_SHIFT) & OUTPUT_TYPE_MASK;
        pin &= OUTPUT_PIN_MASK;

        Serial.print("pin=");
        Serial.print(pin, HEX);    
        Serial.print(", type=0x");
        Serial.print(type, HEX);    
        Serial.print(", angle=");
        Serial.print(angle);    
        Serial.print(", pace=");
        Serial.print(pace, HEX);    
        Serial.print(", state=");
        Serial.print(state, HEX);    
        Serial.print(", delay=");
        Serial.print(delay, HEX);
        Serial.println();

        // If the pin's type has changed, action it.
        if (type != outputTypes[pin])
        {
            setPinType(pin, type);
        }

        // Arrange for Servos to move to requested angle.
        if (   (type == OUTPUT_TYPE_SERVO)
            || (type == OUTPUT_TYPE_SIGNAL))
        {
            moveServo(pin, angle, pace, state, delay);
        }

        // TODO - handle LEDs.
        else if (   (type == OUTPUT_TYPE_LED)
                 || (type == OUTPUT_TYPE_FLASH)
                 || (type == OUTPUT_TYPE_BLINK))
        {
            Serial.println("TODO - LED");
        }
        else
        {
            Serial.print("Unknown outputType ");
            Serial.print(type);
            Serial.println();
        }
    }

    // Consume unexpected data.
    while (Wire.available())
    {
        Wire.read();
    }
}


/** Move a Servo from its current position to the desired one at the pace indicated.
 *  Steps to move the whole range adjusted by the partial range required
 *  and for the pace at which to run (faster = fewer steps).
 */
void moveServo(uint8_t aServo, uint8_t aTarget, uint8_t aPace, uint8_t aState, uint8_t aDelay)
{
    // Set the Servo's movement.
    outputs[aServo].start  = outputs[aServo].servo.read();
    outputs[aServo].target = aTarget;
    outputs[aServo].steps  = (MAX_PACE - (aPace > MAX_PACE ? MAX_PACE : aPace)) * abs((aTarget - outputs[aServo].start)) / PACE_STEPS + 1;
    outputs[aServo].step   = 0;
    outputs[aServo].state  = aState;
    outputs[aServo].delay  = millis() + DELAY_MULTIPLIER * aDelay;
    
    // Report action request
    Serial.print(millis());
    Serial.print("\tMove: servo=");
    Serial.print(aServo);
    Serial.print(", start=");
    Serial.print(outputs[aServo].start);
    Serial.print(", angle=");
    Serial.print(outputs[aServo].servo.read());
    Serial.print(", target=");
    Serial.print(outputs[aServo].target);
    Serial.print(", pace=");
    Serial.print(aPace);
    Serial.print(", steps=");
    Serial.print(outputs[aServo].steps);
    Serial.print(", state=");
    Serial.print(outputs[aServo].state);
    Serial.print(", delay=");
    Serial.print(outputs[aServo].delay);
    Serial.println();
}


/** Step a servo to its next angle.
 */
void stepServo(int aPin)
{
    boolean workInProgress = false;
    
    if (outputs[aPin].step < outputs[aPin].steps)
    {
        outputs[aPin].step += 1;
        
        if (outputs[aPin].step == outputs[aPin].steps)
        {
            // Last step, make sure to hit the target bang-on.
            outputs[aPin].servo.write(outputs[aPin].target);

            // Record Servo's state.
            outputStates[aPin] = outputs[aPin].servo.read();
            EEPROM.put(STATE_BASE, outputStates);
        }
        else
        {
            // Intermediate step, move proportionately (step/steps) along the range (start to target).
            outputs[aPin].servo.write(outputs[aPin].start + (outputs[aPin].target - outputs[aPin].start) * outputs[aPin].step / outputs[aPin].steps);
            workInProgress = true;
        }

        // Test code to report activity.
        if (   (outputs[aPin].step == 1)
            || (outputs[aPin].step == outputs[aPin].steps))
        {
            Serial.print(millis());
            Serial.print("\tStep: pin=");
            Serial.print(aPin);
            Serial.print(", start=");
            Serial.print(outputs[aPin].start);
            Serial.print(", angle=");
            Serial.print(outputs[aPin].servo.read());
            Serial.print(", target=");
            Serial.print(outputs[aPin].target);
            Serial.print(", step=");
            Serial.print(outputs[aPin].step);
            Serial.print(", steps=");
            Serial.print(outputs[aPin].steps);
            Serial.print(", state=");
            Serial.print(outputs[aPin].state);
            Serial.print(", delay=");
            Serial.print(outputs[aPin].delay);
            Serial.println();
        }
    }

    // Indicate work complete
    digitalWrite(LED_BUILTIN, workInProgress);
}


//// Metrics.
//long start = 0;
//long count = 0;

// Ticking
long tick = 0;
long now  = 0;


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
    
    // Every DELAY_STEP msecs, step all the servos if necessary
    if ((now - tick) > DELAY_STEP)
    {
        tick = now;
        
        // Move any Servos that need moving.
        for (int pin = 0; pin < IO_PINS; pin++)
        {
            if (   (outputTypes[pin] == OUTPUT_TYPE_SERVO)
                || (outputTypes[pin] == OUTPUT_TYPE_SIGNAL))
            {
                if (   (outputs[pin].delay == 0)
                    || (outputs[pin].delay <= now))
                {
                    stepServo(pin);
                }
            }
        }
    }
    
    // Set IO Outputs based on their intensity state, using the clock to generate a PWM signal.
    for (int pin = 0; pin < IO_PINS; pin++)
    {
        if (outputTypes[pin] == OUTPUT_TYPE_LED)
        {
            boolean on = outputs[pin].state > 0 && outputs[pin].state >= (now & 0xff);
            digitalWrite(SERVO_BASE_PIN + pin,  on);
            digitalWrite(ioPins[pin],          !on);

//            // DEBUG
//            digitalWrite(LED_BUILTIN, on);
//            Serial.print(now);
//            Serial.print(" ");
//            Serial.print(now & 0xff, HEX);
//            Serial.print(on ? " 1" : " 0");
//            Serial.println();
//            delay(1001);
        }
    }
}
