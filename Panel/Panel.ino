/** 
 *  Controll panel.
 *  
 *  LCD constants and functions.
 *
 * The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
 */

#include <EEPROM.h>
#include <Wire.h>

#include "Messages.h"
#include "Lcd.h"
#include "Panel.h"
#include "Servo.h"
#include "Switch.h"


// Initialize the LCD library with the numbers of the interface pins
// LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
LCD lcd(12, 11, 5, 4, 3, 2);

/** The i2c node IDs. */
const uint8_t controllerID = 0x10;    // Controller ID.
const uint8_t servoBaseID  = 0x50;    // Base ID of the Servo modules 
const uint8_t switchBaseID = 0x20;    // Base ID of the Switch modules.



/** Map the Servo and Switch modules.
 */
void mapHardware()
{
  uint8_t offset = 0;
  
  offset = lcd.printAt(0, 0, M_SCAN_HARDWARE);
  offset = lcd.printAt(0, 1, M_SWITCH);
  
  // Scan for Switch modules.
  for (uint8_t module = 0; module < SWITCH_MODULE_MAX; module++)
  {
    lcd.printAt(offset + 1, 1, module, 2);
    Wire.beginTransmission(switchBaseID + module);
    if (!Wire.endTransmission())   
    {  
        setSwitchModulePresent(module);
    }
  }

  // Scan for Servo modules.
  offset = lcd.printAt(0, 1, M_SERVO);
  for (uint8_t module = 0; module < SWITCH_MODULE_MAX; module++)
  {
    lcd.printAt(offset + 1, 1, module, 2);
    Wire.beginTransmission(servoBaseID + module);
    if (!Wire.endTransmission())   
    {  
        setServoModulePresent(module);  
    }
  }
}


/** Initialise servos.
 */
void initServos()
{
  for (int servo = 0; servo < SERVO_MAX; servo++)
  {
    loadServo(servo);
    saveServo();
    loadSwitch(servo);
    saveSwitch();
    setServoModulePresent(2);
    setSwitchModulePresent(3);
  }
}


/** Setup the Arduino.
 */
void setup()
{
  Serial.begin(115200);         // Serial IO.
  lcd.begin(16, 2);             // LCD panel.
  Wire.begin(controllerID);     // I2C network    

  mapHardware();
  initServos();
}


/** Main loop.
 */
void loop()
{
  lcd.setCursor(10, 1);
  lcd.print(millis());
}
 
