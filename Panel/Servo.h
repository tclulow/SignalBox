/** Servo data.
 */
#ifndef _Servo_h
#define _Servo_h

// Servo modules.
#define SERVO_MODULE_SIZE   8   // 8 servos to each module.
#define SERVO_MODULE_MAX    16  // Maximum modules.
#define SERVO_MODULE_SHIFT  4   // Shift servo number this amount to get a module number.

// ServoData saved in EEPROM
#define SERVO_BASE  0                                         // EEPROM base of Servo data.
#define SERVO_SIZE  sizeof(ServoData)                         // Size of ServoData entry.
#define SERVO_MAX   (SERVO_MODULE_SIZE * SERVO_MODULE_MAX)    // Maximum servos (up to 128).
#define SERVO_END   (SERVO_BASE + SERVO_SIZE * SERVO_MAX)     // End of Switch EEPROM.


/** Data describing a Servo's operation.
 */
struct ServoData
{
  uint8_t mode  = 0;
  uint8_t left  = 0;
  uint8_t right = 0;
  uint8_t speed = 0;
};

// Variables for working with a Servo.
uint16_t   servoModules = 0;    // Bit map of Servo modules present.
uint8_t    servoNumber  = 0;    // Current Servo number.
ServoData  servoData;           // Data describing current Servo.


/** Load a Servo's data from EEPROM.
 */
void loadServo(uint8_t aServo)
{
  if (aServo < SERVO_MAX)
  {
    servoNumber = aServo;
    EEPROM.get(SERVO_BASE + servoNumber * SERVO_SIZE, servoData); 
  }
}


/** Save a Servo's data to EEPROM.
 *  Data in servoNumber and servoData.
 */
void saveServo()
{
  if (servoNumber < SERVO_MAX)
  {
    EEPROM.put(SERVO_BASE + servoNumber * SERVO_SIZE, servoData);
  }
}


/** Record the presence of a ServoModule in the map.
 */
void setServoModulePresent(uint8_t aModule)
{
  servoModules |= (1 << aModule); 
}


/** Is a Servo module present?
 *  Look for servo's module in servoModules.
 */
boolean isServoModule(uint8_t aModule)
{
  return servoModules & (1 << aModule);
}


/** Is a Servo present?
 *  Look for servo's module in servoModules.
 */
boolean isServo(uint8_t aServo)
{
  return isServoModule(aServo >> SERVO_MODULE_SHIFT);
}

#endif
