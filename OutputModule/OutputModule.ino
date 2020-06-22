/** OutputModule.
 */


#include <EEPROM.h>
#include <Servo.h>
#include <Wire.h>


// Servo state saved in EEPROM
#define SERVO_BASE  0                    // EEPROM base of Servo data.
#define SERVO_SIZE  sizeof(servoState)   // Size of Servo data

#define I2C_BASE_ID       0x50   // Output nodes' base ID.
#define DELAY_STEP          50   // Delay (msecs) between steps of a Servo.

#define SERVO_BASE_PIN       4   // Servos attached to this pin and the next 7 more.

#define JUMPER_PINS          4   // Four jumpers.
#define IO_PINS              8   // Eight IO pins.

#define DELAY_MULTIPLIER  1000   // Multiply delay values by this amount (convert to seconds).


// The i2c ID of the module.
uint8_t moduleID = 0;

// The module jumpers
uint8_t jumpers[JUMPER_PINS] = { 1, 0, A6, A7 };

// The digital IO pins.
uint8_t ioPins[IO_PINS]      = { 3, 2, A3, A2, A1, A0, 13, 12 };

// An Array of Servo control structures.
struct 
{
  Servo   servo;            // The Servo.
  uint8_t start  = 0;       // The angle we started at.
  uint8_t target = 0;       // The angle we want to reach.
  uint8_t steps  = 0;       // The number of steps to take.
  uint8_t step   = 0;       // The current step.
  uint8_t state  = 0;       // The state to set the output to.
  long    delay  = 0;       // Delay start to this time.
} outputs[IO_PINS];

// EEPROM persistance of Servo values
uint8_t servoState[IO_PINS] = { 90, 90, 90, 90, 90, 90, 90, 90 };


/** Setup the Arduino.
 */
void setup()
{
  Serial.begin(115200);           // Serial IO.

  // Configure the Jumper pins for input.
  for (int pin = 0; pin < JUMPER_PINS; pin++)
  {
    pinMode(pin, INPUT_PULLUP);
  }

  // Configure the IO pins for output.
  for (int pin = 0; pin < IO_PINS; pin++)
  {
    pinMode(pin, OUTPUT);
  }

  // Configure the on-board LED pin for output
  pinMode(LED_BUILTIN, OUTPUT);


  // Configure i2c from jumpers.
  for (int pin = 0; pin < JUMPER_PINS; pin++)
  {
    moduleID |= digitalRead(ioPins[pin]) << pin;
  }
  moduleID |= I2C_BASE_ID;

  // Recover Servo state from EEPROM
  EEPROM.get(SERVO_BASE, servoState);
  for (int servo = 0; servo < IO_PINS; servo++)
  {
    Serial.print("Init servo ");
    Serial.print(servo);
    Serial.print(" to ");
    Serial.print(servoState[servo]);
    Serial.println();
  }

  // Attach all servos to their pins.
  for (int i = 0, pin = SERVO_BASE_PIN; i < IO_PINS; i++, pin++)
  {
    pinMode(pin, OUTPUT);
    outputs[i].servo.write(servoState[i]);
    outputs[i].servo.attach(pin);
    delay(20);
  }

  // Start i2c communications.
  Wire.begin(moduleID);
  Wire.onReceive(processRequest);

  Serial.print("Module ID: 0x");
  Serial.println(moduleID, HEX);

//  // Test-move a Servo
//  delay(2000);
//  moveServo(0, 0, 100, 1);
}


/** Upon receipt of a request, store it in the corresponding Servo's state.
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
    uint8_t angle = Wire.read();
    uint8_t pace  = Wire.read();
    uint8_t state = Wire.read();  
    uint8_t delay = 0;
    if (Wire.available())
    {
      delay = Wire.read();
    }
  
    moveServo(pin, angle, pace, state, delay);
  }

  // Consume unexpected data.
  while (Wire.available())
  {
    Wire.read();
  }
}


/** Move a Servo from its current position to the desired one
 *  at the pace indicated.
 *  Steps to move the whole range adjusted by the partial range required
 *  and for the pace at which to run (faster = fewer steps).
 */
void moveServo(uint8_t aServo, uint8_t aTarget, uint8_t aPace, uint8_t aState, uint8_t aDelay)
{
  // Set the Servo's movement.
  outputs[aServo].start  = outputs[aServo].servo.read();
  outputs[aServo].target = aTarget;
  outputs[aServo].steps  = (128 - aPace) * abs((aTarget - outputs[aServo].start)) / 128 + 1;
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

//// Metrics.
//long start = 0;
//long count = 0;

// Ticking
long tick  = 0;
long now   = 0;

/** Main loop.
 */
void loop()
{
  boolean stateChanged = false;

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
    for (int servo = 0; servo < IO_PINS; servo++)
    {
      if (   (outputs[servo].delay == 0)
          || (outputs[servo].delay <= now))
      {    
        if (outputs[servo].step < outputs[servo].steps)
        {
          // Indicate work in progress
          digitalWrite(LED_BUILTIN, HIGH);
  
          outputs[servo].step += 1;
          
          if (outputs[servo].step == outputs[servo].steps)
          {
            // Last step, make sure to hit the target bang-on.
            outputs[servo].servo.write(outputs[servo].target);
    
            // Indicate work complete
            digitalWrite(LED_BUILTIN, LOW);
    
            // Record Servo's state.
            stateChanged = true;
            servoState[servo] = outputs[servo].servo.read();
          }
          else
          {
            // Intermediate step, move proportionately (step/steps) along the range (start to target).
            outputs[servo].servo.write(outputs[servo].start + (outputs[servo].target - outputs[servo].start) * outputs[servo].step / outputs[servo].steps);
          }
    
//          // Test code to report activity.
//          if (   (outputs[servo].step == 1)
//              || (outputs[servo].step == outputs[servo].steps))
//          {
//            Serial.print(now);
//            Serial.print("\tStep: servo=");
//            Serial.print(servo);
//            Serial.print(", start=");
//            Serial.print(outputs[servo].start);
//            Serial.print(", angle=");
//            Serial.print(outputs[servo].servo.read());
//            Serial.print(", target=");
//            Serial.print(outputs[servo].target);
//            Serial.print(", step=");
//            Serial.print(outputs[servo].step);
//            Serial.print(", steps=");
//            Serial.print(outputs[servo].steps);
//            Serial.print(", state=");
//            Serial.print(outputs[servo].state);
//            Serial.print(", delay=");
//            Serial.print(outputs[servo].delay);
//            Serial.println();
//          }
        }
      }
    }

    // If any Servos moved, record their new position
    if (stateChanged)
    {
      EEPROM.put(SERVO_BASE, servoState);
    }
  }
  
  // Set IO Outputs based on their intensity state, using the clock to generate a PWM signal.
  for (int output = 0; output < IO_PINS; output++)
  {
    digitalWrite(ioPins[output], outputs[output].state > (now & 0xff));
  }
}
