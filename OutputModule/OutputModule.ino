
/** OutputModule.
 */


#include <EEPROM.h>
#include <Servo.h>
#include <Wire.h>

#include "Messages.h"
#include "Common.h"
#include "System.h"


// Output state saved in EEPROM
#define SYSTEM_BASE   0                                         // SystemData goes here
#define TYPE_BASE     SYSTEM_BASE + sizeof(systemData)          // Base of Output type data.
#define VALUE_BASE    TYPE_BASE   + IO_PINS * sizeof(uint8_t)   // Base of Output value data.
#define EEPROM_END    VALUE_BASE  + IO_PINS * sizeof(uint8_t)   // Size of EEPROM


#define STEP_SERVO          50   // Delay (msecs) between steps of a Servo.
#define STEP_LED             5   // delay (msecs) between steps of a LED.
#define STEP_FLASH         100   // Delay (msecs) between flashes of a FLASH or BLINK.
#define MAX_PACE           124   // Maximum pace value.
#define PACE_STEPS         128   // Pace adjustment when converting to steps.

#define OUTPUT_BASE_PIN      4   // Output attached to this pin and the next 7 more.

#define JUMPER_PINS          4   // Four jumpers.
#define IO_PINS              8   // Eight IO pins.

#define DELAY_MULTIPLIER  1000   // Multiply delay values by this amount (convert to seconds).


// The module jumper pins
const uint8_t jumperPins[JUMPER_PINS] = { 1, 0, A6, A7 };

// The digital IO pins.
const uint8_t ioPins[IO_PINS]         = { 3, 2, A3, A2, A1, A0, 13, 12 };


// The i2c ID of the module.
uint8_t moduleID  = 0;

// Ticking
long    now       = 0;
long    tickServo = 0;
long    tickLed   = 0;
long    tickFlash = 0;


// An Array of Output control structures.
struct 
{
    Servo   servo;          // The Servo (if there is one).
    uint8_t type   = 0;     // The type of the output.
    uint8_t start  = 0;     // The value we started at.
    uint8_t target = 0;     // The value we want to reach.
    uint8_t steps  = 0;     // The number of steps to take.
    uint8_t step   = 0;     // The current step.
    uint8_t state  = 0;     // The state of the ouput, Hi or Lo.
    uint8_t alt    = 0;     // The value of the alternate output.
    uint8_t value  = 0;     // The value of the output.
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
        for (uint8_t pin = 0; pin < IO_PINS; pin++)
        {
            EEPROM.get(TYPE_BASE  + pin, outputs[pin].type);
            EEPROM.get(VALUE_BASE + pin, outputs[pin].value);
        }
    }

    // Report state from EEPROM
    for (int pin = 0; pin < IO_PINS; pin++)
    {
        Serial.print("Init ");
        Serial.print(pin);
        Serial.print(" type 0x");
        Serial.print(outputs[pin].type, HEX);
        Serial.print(" value 0x");
        Serial.print(outputs[pin].value, HEX);
        Serial.println();
    }

    // DEBUG - move LED 0 and servo 1
    int pin = 0;

    setOutputType(pin, OUTPUT_TYPE_SERVO);
    outputs[pin].start  = 0;
    outputs[pin].target = 180;
    outputs[pin].steps  = 10;
    outputs[pin].step   = 0;
    outputs[pin].state  = 1;
    outputs[pin].alt    = 180;
    outputs[pin].value  = 0;
    outputs[pin].delay  = millis() + DELAY_MULTIPLIER * 1; // aDelay;
    pin += 1;

//    setOutputType(pin, OUTPUT_TYPE_LED);
//    outputs[pin].start  = 0;
//    outputs[pin].target = 180;
//    outputs[pin].steps  = 100;
//    outputs[pin].step   = 0;
//    outputs[pin].state  = 1;
//    outputs[pin].alt    = 180;
//    outputs[pin].value  = 0;
//    outputs[pin].delay  = millis() + DELAY_MULTIPLIER * 1; // aDelay;
//    pin += 1;
//
//    setOutputType(pin, OUTPUT_TYPE_LED);
//    outputs[pin].start  = 0;
//    outputs[pin].target = 180;
//    outputs[pin].steps  = 10;
//    outputs[pin].step   = 0;
//    outputs[pin].state  = 0;
//    outputs[pin].alt    = 180;
//    outputs[pin].value  = 0;
//    outputs[pin].delay  = millis() + DELAY_MULTIPLIER * 2; // aDelay;
//    pin += 1;
//
//    setOutputType(pin, OUTPUT_TYPE_BLINK);
//    outputs[pin].start  = 0;
//    outputs[pin].target = 180;
//    outputs[pin].steps  = 10;
//    outputs[pin].step   = 0;
//    outputs[pin].state  = 1;
//    outputs[pin].alt    = 0;
//    outputs[pin].value  = 180;
//    outputs[pin].delay  = millis() + DELAY_MULTIPLIER * 3; // aDelay;
//    pin += 1;
//
//    setOutputType(pin, OUTPUT_TYPE_BLINK);
//    outputs[pin].start  = 0;
//    outputs[pin].target = 180;
//    outputs[pin].steps  = 10;
//    outputs[pin].step   = 0;
//    outputs[pin].state  = 0;
//    outputs[pin].alt    = 180;
//    outputs[pin].value  = 0;
//    outputs[pin].delay  = millis() + DELAY_MULTIPLIER * 4; // aDelay;
//    pin += 1;
//
//    setOutputType(pin, OUTPUT_TYPE_LED);
//    outputs[pin].start  = 0;
//    outputs[pin].target = 180;
//    outputs[pin].steps  = 10;
//    outputs[pin].step   = 0;
//    outputs[pin].state  = 1;
//    outputs[pin].alt    = 0;
//    outputs[pin].value  = 0;
//    outputs[pin].delay  = millis() + DELAY_MULTIPLIER * 0; // aDelay;
//    pin += 1;

    while (pin < IO_PINS)
    {
        setOutputType(pin++, OUTPUT_TYPE_NONE);
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
        setOutputType(pin, outputs[pin].type);  
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
    for (uint8_t pin = 0; pin < IO_PINS; pin++)
    {
        EEPROM.put(TYPE_BASE  + pin, OUTPUT_TYPE_NONE);
        EEPROM.put(VALUE_BASE + pin, 90);
    }
    
    EEPROM.put(SYSTEM_BASE, systemData);
}


/** Set the Type of a pin.
 *  Remove/disconnect previous type if necessary.
 */
void setOutputType(int aPin, uint8_t aType)
{
    if (aType != outputs[aPin].type)
    {
        // Remove/disable old type.
        if (   (outputs[aPin].type == OUTPUT_TYPE_SERVO)
            || (outputs[aPin].type == OUTPUT_TYPE_SIGNAL))
        {
            // Detach servo.
            outputs[aPin].servo.detach();
        }

        // Record the change.
        outputs[aPin].type = aType;
        EEPROM.put(TYPE_BASE + aPin, aType);
    }

    // Establish new type.
    if (   (aType == OUTPUT_TYPE_SERVO)
        || (aType == OUTPUT_TYPE_SIGNAL))
    {
        outputs[aPin].servo.write(outputs[aPin].value);
        outputs[aPin].servo.attach(OUTPUT_BASE_PIN + aPin);
    }
}


/** Upon receipt of a request, store it in the corresponding Output's controller.
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
        if (type != outputs[pin].type)
        {
            setOutputType(pin, type);
        }

        // Decide upon starting position.
        if (   (type == OUTPUT_TYPE_SERVO)
            || (type == OUTPUT_TYPE_SIGNAL))
        {
            start = outputs[pin].value;   // start = outputs[pin].servo.read();
            digitalWrite(ioPins[pin], state);
        }
        else if (type == OUTPUT_TYPE_LED)
        {
            // start = 0;
        }
        else if (   (type == OUTPUT_TYPE_FLASH)
                 || (type == OUTPUT_TYPE_BLINK))
        {
            start = outputs[pin].value;
        }
        else
        {
            Serial.print("Unknown outputType ");
            Serial.print(type);
            Serial.println();
        }

        // Set the Output's movement characteristics.
        outputs[pin].start  = start;
        outputs[pin].target = target;
        outputs[pin].steps  = (MAX_PACE - pace) * abs((target - start)) / PACE_STEPS + 1;
        outputs[pin].step   = 1;
        outputs[pin].alt    = outputs[pin].value;
        outputs[pin].value  = start;
        outputs[pin].delay  = millis() + DELAY_MULTIPLIER * delay;

        reportOutput(pin);
    }

    // Consume unexpected data.
    while (Wire.available())
    {
        Wire.read();
    }
}


/** Report the status of an output during it's move
 */
void reportOutput(uint8_t aPin)
{
    // Report Output movement.
    Serial.print(millis());
    Serial.print("\tOutput: pin=");
    Serial.print(aPin);
    Serial.print(", type=");
    Serial.print(outputs[aPin].type, HEX);
    Serial.print(", start=");
    Serial.print(outputs[aPin].start);
    Serial.print(", target=");
    Serial.print(outputs[aPin].target);
    Serial.print(", steps=");
    Serial.print(outputs[aPin].steps);
    Serial.print(", step=");
    Serial.print(outputs[aPin].step);
    Serial.print(", alt=");
    Serial.print(outputs[aPin].alt);
    Serial.print(", value=");
    Serial.print(outputs[aPin].value);
    Serial.print(", state=");
    Serial.print(outputs[aPin].state);
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
        if (outputs[pin].type == aType)
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
    if (outputs[aPin].step < outputs[aPin].steps)
    {
        outputs[aPin].step += 1;

        if (outputs[aPin].step == outputs[aPin].steps)
        {
            // Last step, make sure to hit the target bang-on.
            outputs[aPin].value = outputs[aPin].target;
            outputs[aPin].alt   = 0;
            
            // Record Output's value.
            EEPROM.put(VALUE_BASE + aPin, outputs[aPin].value);
        }
        else
        {
            // Intermediate step, move proportionately (step/steps) along the range (start to target).
            outputs[aPin].value = outputs[aPin].start + (outputs[aPin].target - outputs[aPin].start) * outputs[aPin].step / outputs[aPin].steps;
            outputs[aPin].alt -= outputs[aPin].alt / (outputs[aPin].steps + 1 - outputs[aPin].step);
        }

        // Ensure Servos move to new state.
        if (isServo)
        {
            outputs[aPin].servo.write(outputs[aPin].value);
            digitalWrite(LED_BUILTIN, HIGH);                    // Indicate work in progress;
        }

        // Test code to report activity.
        if (   (outputs[aPin].step == 1)
            || (outputs[aPin].step == outputs[aPin].steps))
        {
            reportOutput(aPin);
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
        if (   (outputs[pin].type == OUTPUT_TYPE_FLASH)
            || (outputs[pin].type == OUTPUT_TYPE_BLINK))
        {
            if (outputs[pin].steps > 0)
            {
                stepFlash(pin);
            }
        }
    }
}


/** Flash the given output.
 */
void stepFlash(uint8_t aPin)
{
    outputs[aPin].step += 1;
    if (outputs[aPin].step > outputs[aPin].steps)
    {
        if (   (outputs[aPin].delay > 0)
            && (outputs[aPin].delay < now))
        {
            // Stop flashing.
            outputs[aPin].steps = 0;

            // Decide if to finish with output Hi or Lo
            if (   (outputs[aPin].type ==    OUTPUT_TYPE_FLASH)
                && (outputs[aPin].state))
            {
                outputs[aPin].value = outputs[aPin].target;
                outputs[aPin].alt   = 0;
            }
            else
            {
                outputs[aPin].value = 0;
                outputs[aPin].alt   = outputs[aPin].target;
            }
        }
        else
        {
            // Flash opposite way.
            outputs[aPin].step = 0;
            if (outputs[aPin].value)
            {
                outputs[aPin].value = 0;
                outputs[aPin].alt   = outputs[aPin].target;
            }
            else
            {
                outputs[aPin].value = outputs[aPin].target;
                outputs[aPin].alt   = 0;
            }
        }
        
        digitalWrite(OUTPUT_BASE_PIN + aPin, outputs[aPin].value);
        digitalWrite(ioPins[aPin],           outputs[aPin].alt);
    }
     
    reportOutput(aPin);
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
        digitalWrite(LED_BUILTIN, LOW);       // Assume no work in progress.
        stepOutputs(true, OUTPUT_TYPE_SERVO);
        stepOutputs(true, OUTPUT_TYPE_SIGNAL);
    }

    // Every STEP_LED msecs, step the LEDs if necessary
    if ((now - tickLed) > STEP_LED)
    {
        tickLed = now;
        stepOutputs(false, OUTPUT_TYPE_LED);
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
        if (outputs[pin].type == OUTPUT_TYPE_LED)
        {
            boolean on  =    outputs[pin].value >  0 
                          && outputs[pin].value >= (now & 0xff);
            boolean alt =    outputs[pin].alt   >  0 
                          && outputs[pin].alt   >= (now & 0xff);
            if (outputs[pin].state)
            {
                digitalWrite(OUTPUT_BASE_PIN + pin,  on);
                digitalWrite(ioPins[pin],           alt);
            }
            else
            {
                digitalWrite(OUTPUT_BASE_PIN + pin, alt);
                digitalWrite(ioPins[pin],            on);
            }
            
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
