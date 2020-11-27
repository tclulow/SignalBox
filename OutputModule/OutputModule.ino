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
#define ALT_BASE      VALUE_BASE  + IO_PINS * sizeof(uint8_t)   // Base of Output alt data.
#define EEPROM_END    ALT_BASE    + IO_PINS * sizeof(uint8_t)   // Size of EEPROM


#define STEP_SERVO          50   // Delay (msecs) between steps of a Servo.
#define STEP_LED            10   // Delay (msecs) between steps of a LED.
#define STEP_FLASH          20   // Delay (msecs) between steps of flashes of a FLASH or BLINK.
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
    uint8_t value  = 0;     // The value of the output.
    uint8_t alt    = 0;     // The value of the alternate output.
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
        firstRun();
    }
    else
    {
        // Recover state from EEPROM.
        for (uint8_t pin = 0; pin < IO_PINS; pin++)
        {
            EEPROM.get(TYPE_BASE  + pin, outputs[pin].type);
            EEPROM.get(VALUE_BASE + pin, outputs[pin].value);
            EEPROM.get(ALT_BASE   + pin, outputs[pin].alt);
            
            if (   (outputs[pin].type == OUTPUT_TYPE_LED)
                || (outputs[pin].type == OUTPUT_TYPE_FLASH)
                || (outputs[pin].type == OUTPUT_TYPE_BLINK))
            {
                // LEDs need their last value/alt in target/start
                outputs[pin].target = outputs[pin].value;
                outputs[pin].start  = outputs[pin].alt;

                if (outputs[pin].state || true)         // TODO - persist the state so this can be set.
                {
                    outputs[pin].alt = 0;
                }
                else
                {
                    outputs[pin].value  = 0;
                }
            }
        }
    }

    // Report state from EEPROM
    for (int pin = 0; pin < IO_PINS; pin++)
    {
        Serial.print("Init ");
        Serial.print(pin);
        Serial.print(", type 0x");
        Serial.print(outputs[pin].type,  HEX);
        Serial.print(", value ");
        Serial.print(outputs[pin].value);
        Serial.print(", alt ");
        Serial.print(outputs[pin].alt);
        Serial.print(", target ");
        Serial.print(outputs[pin].target);
        Serial.print(", start ");
        Serial.print(outputs[pin].start);
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
        setOutputType(pin, outputs[pin].type);  
    }
    
    // Start i2c communications.
    Wire.begin(moduleID);
    Wire.onReceive(processRequest);

    Serial.print("Module ID: 0x");
    Serial.println(moduleID, HEX);

    // DEBUG - move LED 0 and servo 1
    uint8_t pin     = 0;
    uint8_t pace    = 117;  // 10 steps.
    uint8_t delayBy = 1;
    
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

    // Initialise EEPROM with suitable data.
    for (uint8_t pin = 0; pin < IO_PINS; pin++)
    {
        EEPROM.put(TYPE_BASE  + pin, OUTPUT_TYPE_NONE);
        EEPROM.put(VALUE_BASE + pin, 90);
        EEPROM.put(ALT_BASE   + pin, 90);
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

        actionRequest(pin, type, target, pace, state, delay);
    }

    // Consume unexpected data.
    while (Wire.available())
    {
        Wire.read();
    }
}


/** Action a request.
 */
void actionRequest(uint8_t aPin, uint8_t aType, uint8_t aTarget, uint8_t aPace, uint8_t aState, uint8_t aDelay)
{
    Serial.print(millis());
    Serial.print("\tAction: ");
    Serial.print("pin=");
    Serial.print(aPin, HEX);    
    Serial.print(", type=");
    Serial.print(aType, HEX);    
    Serial.print(", target=");
    Serial.print(aTarget);    
    Serial.print(", pace=");
    Serial.print(aPace, HEX);    
    Serial.print(", state=");
    Serial.print(aState, HEX);    
    Serial.print(", delay=");
    Serial.print(aDelay, HEX);
    Serial.println();

    // If the pin's type has changed, action it.
    if (aType != outputs[aPin].type)
    {
        setOutputType(aPin, aType);
    }

    // Set the Output's movement characteristics.
    outputs[aPin].step   = 0;
    outputs[aPin].state  = aState;
    outputs[aPin].delay  = (aDelay == 0 ? 0 : millis() + DELAY_MULTIPLIER * aDelay);

    if (   (aType == OUTPUT_TYPE_SERVO)
        || (aType == OUTPUT_TYPE_SIGNAL))
    {
        outputs[aPin].start  = outputs[aPin].value;    // start = outputs[aPin].servo.read();
        outputs[aPin].target = aTarget;
        outputs[aPin].steps  = (MAX_PACE - aPace) * abs((aTarget - outputs[aPin].start)) / PACE_STEPS + 1;
        digitalWrite(ioPins[aPin], aState);
        EEPROM.put(VALUE_BASE + aPin, aTarget);
    }
    else if (   (aType == OUTPUT_TYPE_LED)
             || (aType == OUTPUT_TYPE_FLASH)
             || (aType == OUTPUT_TYPE_BLINK))
    {
        if (aState)
        {
            outputs[aPin].target = aTarget;
            EEPROM.put(VALUE_BASE + aPin, aTarget);
        }
        else
        {
            outputs[aPin].start = aTarget; 
            EEPROM.put(ALT_BASE + aPin, aTarget);
        }

        // For a BLINK that's not running, force state on.
        if (   (aType == OUTPUT_TYPE_BLINK)
            && (outputs[aPin].steps == 0))
        {
            outputs[aPin].state = 1;
        }

        outputs[aPin].steps = (MAX_PACE - aPace) * 2 + 1;
        if (outputs[aPin].type != OUTPUT_TYPE_LED)
        {
            outputs[aPin].step  = outputs[aPin].steps;
        }
    }
    else
    {
        Serial.print("Unknown outputType ");
        Serial.print(aType);
        Serial.println();
    }

    reportOutput(aPin);
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
    Serial.print(", value=");
    Serial.print(outputs[aPin].value);
    Serial.print(", alt=");
    Serial.print(outputs[aPin].alt);
    Serial.print(", state=");
    Serial.print(outputs[aPin].state);
    Serial.print(", delay=");
    Serial.print(outputs[aPin].delay);
    Serial.println();
}


/** Step all the Servos if necessary.
 */
void stepServos()
{
    // Move any Outputs that need moving.
    for (int pin = 0; pin < IO_PINS; pin++)
    {
        if (   (outputs[pin].type == OUTPUT_TYPE_SERVO)
            || (outputs[pin].type == OUTPUT_TYPE_SIGNAL))
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
        outputs[aPin].step += 1;

        if (outputs[aPin].step == outputs[aPin].steps)
        {
            // Last step, make sure to hit the target bang-on.
            outputs[aPin].value = outputs[aPin].target;
        }
        else
        {
            // Intermediate step, move proportionately (step/steps) along the range (start to target).
            outputs[aPin].value = outputs[aPin].start + (outputs[aPin].target - outputs[aPin].start) * outputs[aPin].step / outputs[aPin].steps;
        }

        // Ensure Servos move to new state.
        outputs[aPin].servo.write(outputs[aPin].value);
        digitalWrite(LED_BUILTIN, HIGH);                    // Indicate work in progress;

        // Test code to report activity.
//        if (   (outputs[aPin].step == 1)
//            || (outputs[aPin].step == outputs[aPin].steps))
        {
            reportOutput(aPin);
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
        if (outputs[pin].type == OUTPUT_TYPE_LED)
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

        if (outputs[aPin].step == outputs[aPin].steps)
        {
            // Last step, make sure to hit the target bang-on.
            if (outputs[aPin].state)
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
            // Intermediate step, move proportionately (step/steps) along the range (start to target).
            if (outputs[aPin].state)
            {
                outputs[aPin].value  = outputs[aPin].target * outputs[aPin].step / outputs[aPin].steps;
                outputs[aPin].alt   -= outputs[aPin].alt   / (outputs[aPin].steps + 1 - outputs[aPin].step);
            }
            else
            {
                outputs[aPin].value -= outputs[aPin].value / (outputs[aPin].steps + 1 - outputs[aPin].step);
                outputs[aPin].alt    = outputs[aPin].target * outputs[aPin].step / outputs[aPin].steps;
            }
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
        if (   (   (outputs[aPin].delay > 0)
                && (outputs[aPin].delay < now))
            || (   (outputs[aPin].type  == OUTPUT_TYPE_BLINK)
                && (outputs[aPin].state == 0)))
        {
            // Stop flashing.
            outputs[aPin].steps = 0;

            // Decide if to finish with output Hi or Lo
            if (   (outputs[aPin].type == OUTPUT_TYPE_FLASH)
                && (outputs[aPin].state))
            {
                outputs[aPin].value = outputs[aPin].target;
                outputs[aPin].alt   = 0;
            }
            else
            {
                outputs[aPin].value = 0;
                outputs[aPin].alt   = outputs[aPin].start;
            }
        }
        else
        {
            // Flash opposite way.
            outputs[aPin].step = 1;
            if (outputs[aPin].value)
            {
                outputs[aPin].value = 0;
                outputs[aPin].alt   = outputs[aPin].start;
            }
            else
            {
                outputs[aPin].value = outputs[aPin].target;
                outputs[aPin].alt   = 0;
            }
        }

        // Test code to report activity.
        if (   (outputs[aPin].step == 1)
            || (outputs[aPin].step == outputs[aPin].steps))
        {
            reportOutput(aPin);
        }
    }

//    // Test code to report activity.
//    if (   (outputs[aPin].step == 1)
//        || (outputs[aPin].step == outputs[aPin].steps))
//    {
//        reportOutput(aPin);
//    }
}


//// Metrics.
//long start = 0;
//long count = 0;

// DEBUG test marker
int testRun = 0;

/** Main loop.
 */
void loop()
{
    long nowMicros = micros();
            
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

    // DEBUG tests.
    if (   (now > 5000)
        && (testRun == 0))
    {
        test1();
        testRun += 1;
    }
    if (   (now > 10000)
        && (testRun == 1))
    {
        test2();
        testRun += 1;
    }

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
        if (   (outputs[pin].type == OUTPUT_TYPE_LED)
            || (outputs[pin].type == OUTPUT_TYPE_FLASH)
            || (outputs[pin].type == OUTPUT_TYPE_BLINK))
        {
            digitalWrite(OUTPUT_BASE_PIN + pin,    outputs[pin].value >  0 
                                                && outputs[pin].value >= (nowMicros & 0xff));
            digitalWrite(ioPins[pin],              outputs[pin].alt   >  0 
                                                && outputs[pin].alt   >= (nowMicros & 0xff));
            // DEBUG
            if (pin == 0)
            {
                digitalWrite(LED_BUILTIN,    outputs[pin].value >  0 
                                          && outputs[pin].value >= (nowMicros & 0xff));
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


void test1()
{
    int pin      = 0;
    int pace     = 117;
    int delayBy  = 0;

    Serial.println();
    
    // actionRequest(pin, type, target, pace, state, delay);
//    actionRequest(pin++, OUTPUT_TYPE_SERVO, 180, pace, 1, delayBy++);
    actionRequest(pin++, OUTPUT_TYPE_LED,   181,    0, 1, delayBy++);
//    actionRequest(pin++, OUTPUT_TYPE_FLASH,  20, 124, 1, 4);
//    actionRequest(pin++, OUTPUT_TYPE_BLINK,  20, pace, 1, 4);

    while (pin < IO_PINS)
    {
        setOutputType(pin++, OUTPUT_TYPE_NONE);
    }
    Serial.println();
}


void test2()
{
    int pin      = 0;
    int pace     = 117;
    int delayBy  = 0;

    Serial.println();
    
    // actionRequest(pin, type, target, pace, state, delay);
//    actionRequest(pin++, OUTPUT_TYPE_SERVO,   0, pace, 0, delayBy++);
    actionRequest(pin++, OUTPUT_TYPE_LED,   179,    0, 0, delayBy++);
//    actionRequest(pin++, OUTPUT_TYPE_FLASH, 178,  0, 0, 0);
//    actionRequest(pin++, OUTPUT_TYPE_BLINK, 102, pace, 0, 4);
    
    Serial.println();
}
