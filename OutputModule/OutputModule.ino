
/** OutputModule.
 */


#include <EEPROM.h>
#include <Servo.h>
#include <Wire.h>

#include "Messages.h"
#include "Common.h"
#include "System.h"


// Output state saved in EEPROM
#define SYSTEM_BASE   0                                   // SystemData goes here
#define TYPE_BASE     SYSTEM_BASE + sizeof(systemData)    // Base of Output type data.
#define STATE_BASE    TYPE_BASE   + sizeof(outputTypes)   // Base of Output state data.
#define IO_BASE       STATE_BASE  + sizeof(outputStates)  // Base of IO states data. 
#define EEPROM_END    IO_BASE     + sizeof(ioStates)      // Size of EEPROM


#define STEP_SERVO          50   // Delay (msecs) between steps of a Servo.
#define STEP_LED             5   // delay (msecs) between steps of a LED.
#define MAX_PACE           124   // Maximum pace value.
#define PACE_STEPS         128   // Pace adjustment when converting to steps.

#define OUTPUT_BASE_PIN      4   // Output attached to this pin and the next 7 more.

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

// Ticking
long now       = 0;
long tickServo = 0;
long tickLed   = 0;


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

    // DEBUG - move LED 0 and servo 1
    int pin = 0;

    outputTypes[pin]    = OUTPUT_TYPE_LED;
    outputStates[pin]   = 0x0;
    outputs[pin].start  = 0;
    outputs[pin].target = 180;
    outputs[pin].steps  = 10;
    outputs[pin].step   = 0;
    outputs[pin].state  = 0;
    outputs[pin].delay  = millis() + DELAY_MULTIPLIER * 0; // aDelay;

    pin += 1;
    outputTypes[pin]    = OUTPUT_TYPE_SERVO;
    outputStates[pin]   = 0x0;
    outputs[pin].start  = 0;
    outputs[pin].target = 180;
    outputs[pin].steps  = 10;
    outputs[pin].step   = 0;
    outputs[pin].state  = 0;
    outputs[pin].delay  = millis() + DELAY_MULTIPLIER * 0; // aDelay;
    
    // Report state from EEPROM
    for (int pin = 0; pin < IO_PINS; pin++)
    {
        Serial.print("Init ");
        Serial.print(pin);
        Serial.print(" type 0x");
        Serial.print(outputTypes[pin], HEX);
        Serial.print(" state 0x");
        Serial.print(outputStates[pin], HEX);
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
        pinMode(OUTPUT_BASE_PIN + pin, OUTPUT);
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
        outputs[aPin].servo.attach(OUTPUT_BASE_PIN + aPin);
    }
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
    
        uint8_t pin    = Wire.read();
        uint8_t type   = 0;
        uint8_t start  = 0;
        uint8_t target = Wire.read();
        uint8_t pace   = Wire.read();
        uint8_t state  = Wire.read();  
        uint8_t delay  = 0;
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
        Serial.print(", target=");
        Serial.print(target);    
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

        // Recover starting position for output.
        if (   (type == OUTPUT_TYPE_SERVO)
            || (type == OUTPUT_TYPE_SIGNAL))
        {
            start = outputs[pin].servo.read();
        }
        else if (type == OUTPUT_TYPE_LED)
        {
            // start = 0;
        }
        else
        {
//             || (type == OUTPUT_TYPE_FLASH)
//             || (type == OUTPUT_TYPE_BLINK))
            Serial.print("Unknown outputType ");
            Serial.print(type);
            Serial.println();
        }

        // Set the Output's movement characteristics.
        outputs[pin].start  = start;
        outputs[pin].target = target;
        outputs[pin].steps  = (MAX_PACE - pace) * abs((target - start)) / PACE_STEPS + 1;
        outputs[pin].step   = 0;
        outputs[pin].state  = start;
        outputs[pin].delay  = millis() + DELAY_MULTIPLIER * delay;

        reportMovement(pin);
    }

    // Consume unexpected data.
    while (Wire.available())
    {
        Wire.read();
    }
}


/** Report the status of an output during it's move
 */
void reportMovement(uint8_t aPin)
{
    // Report Output movement.
    Serial.print(millis());
    Serial.print("\tMove: pin=");
    Serial.print(aPin);
    Serial.print(", start=");
    Serial.print(outputs[aPin].start);
    Serial.print(", target=");
    Serial.print(outputs[aPin].target);
    Serial.print(", step=");
    Serial.print(outputs[aPin].step);
    Serial.print(", steps=");
    Serial.print(outputs[aPin].steps);
    Serial.print(", state=");
    Serial.print(outputs[aPin].state);
    Serial.print(", alt=");
    Serial.print(outputs[aPin].alt);
    Serial.print(", delay=");
    Serial.print(outputs[aPin].delay);
    Serial.println();
}


/** Step all the outputs of the particular type if necessary.
 */
void stepOutputs(boolean isServo, uint8_t aType)
{
    // Move any Outputs that need moving.
    for (int pin = 0; pin < IO_PINS; pin++)
    {
        if (outputTypes[pin] == aType)
        {
            if (   (outputs[pin].delay == 0)
                || (outputs[pin].delay <= now))
            {
                stepOutput(pin, isServo);
            }
        }
    }
}


/** Step an Output to its next position.
 */
void stepOutput(int aPin, boolean isServo)
{
    boolean working = false;
    
    if (outputs[aPin].step < outputs[aPin].steps)
    {
        outputs[aPin].step += 1;
        
        if (outputs[aPin].step == outputs[aPin].steps)
        {
            // Last step, make sure to hit the target bang-on.
            outputStates[aPin] = outputs[aPin].target;

            // Record Output's state.
            EEPROM.put(STATE_BASE + aPin, outputStates[aPin]);
        }
        else
        {
            // Intermediate step, move proportionately (step/steps) along the range (start to target).
            outputStates[aPin] = outputs[aPin].start + (outputs[aPin].target - outputs[aPin].start) * outputs[aPin].step / outputs[aPin].steps;

            working = true;
        }

        outputs[aPin].state = outputStates[aPin];

        // Ensure Servos move to new state.
        if (isServo)
        {
            outputs[aPin].servo.write(outputStates[aPin]);
        }

        // Test code to report activity.
//        if (   (outputs[aPin].step == 1)
//            || (outputs[aPin].step == outputs[aPin].steps))
        {
            reportMovement(aPin);
        }
    }

    // Indicate working or not.
    if (isServo)
    {
        digitalWrite(LED_BUILTIN, working);
    }
}


//// Metrics.
//long start = 0;
//long count = 0;


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

    // Every STEP_SERVO msecs, step the servos if necessary
    if ((now - tickServo) > STEP_SERVO)
    {
        tickServo = now;
        stepOutputs(true, OUTPUT_TYPE_SERVO);
        stepOutputs(true, OUTPUT_TYPE_SIGNAL);
    }

    // Every STEP_LED msecs, step the LEDs if necessary
    if ((now - tickLed) > STEP_LED)
    {
        tickLed = now;
        stepOutputs(false, OUTPUT_TYPE_LED);
    }
    
    // Set IO Outputs based on their intensity state, using the clock to generate a PWM signal.
    for (int pin = 0; pin < IO_PINS; pin++)
    {
        if (outputTypes[pin] == OUTPUT_TYPE_LED)
        {
            boolean on =    outputs[pin].state > 0 
                         && outputs[pin].state >= (now & 0xff);
            digitalWrite(OUTPUT_BASE_PIN + pin,  on);
            digitalWrite(ioPins[pin],           !on);

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
