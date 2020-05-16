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
#include "Buttons.h"


// Initialize the LCD library with the numbers of the interface pins
// LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
LCD lcd(12, 11, 5, 4, 3, 2);

/** The i2c node IDs. */
const uint8_t controllerID = 0x10;    // Controller ID.
const uint8_t servoBaseID  = 0x50;    // Base ID of the Servo modules 
const uint8_t switchBaseID = 0x20;    // Base ID of the Switch modules.

int currentSwitchState[SWITCH_MODULE_MAX];    // Current state of switches


/** Map the Servo and Switch modules.
 */
void mapHardware()
{
  lcd.clear();
  lcd.printAt(0, 0, M_SCAN_HARDWARE);
  lcd.print(CHAR_SPACE);
  
  // Scan for Servo modules.
  for (int module = 0; module < SWITCH_MODULE_MAX; module++)
  {
    Wire.beginTransmission(servoBaseID + module);
    if (Wire.endTransmission())
    {
      lcd.print(CHAR_DOT); 
    }
    else
    {
      lcd.print(module, HEX);
      setServoModulePresent(module);  
    }
  }

  // Scan for Switch modules.
  lcd.setCursor(0, 1);
  for (int module = 0; module < SWITCH_MODULE_MAX; module++)
  {
    Wire.beginTransmission(switchBaseID + module);
    if (Wire.endTransmission())   
    {
      lcd.print(CHAR_DOT); 
    }
    else
    {  
      lcd.print(module, HEX);
      setSwitchModulePresent(module);
    }
  }
  delay(1000);
  // lcd.clear();
}


/** Configure all switches for input.
 */
void initSwitches()
{ 
  int offset = 0;
  
  lcd.clear();
  lcd.printAt(0, 0, M_INIT_SWITCHES);

  for(int module = 0; module < SWITCH_MODULE_MAX; module++)
  {
    if (isSwitchModule(module))
    {
      lcd.print(module, HEX);
      Wire.beginTransmission(switchBaseID + module); 
      Wire.write(SWITCH_PORTA_DIRECTION);
      Wire.write(0xFF);
      Wire.endTransmission();

      Wire.beginTransmission(switchBaseID + module);  
      Wire.write(SWITCH_PORTB_DIRECTION);
      Wire.write(0xFF);
      Wire.endTransmission();

      Wire.beginTransmission(switchBaseID + module);
      Wire.write (SWITCH_PORTA_PULLUPS);
      Wire.write(0xFF);
      Wire.endTransmission();  
       
      Wire.beginTransmission(switchBaseID + module);
      Wire.write(SWITCH_PORTB_PULLUPS);
      Wire.write(0xFF);
      Wire.endTransmission();  
    }
    else
    {
      lcd.print(CHAR_DOT);
    }
  }   
  delay(1000);
  // lcd.clear();  
}


/** Configure the system.
 */
void configure()
{
  lcd.clear();
  lcd.printAt(0, 0, M_CONFIG);

  // Wait for button to be released.
  while (readButton());

  // TODO - configuration here
}


/** Scan all the switches.
 *  Process any that have changed.
 */
void scanSwitches()
{ 
  // Scan all the modules. 
  for (int module=0; module < SWITCH_MODULE_MAX; module++)
  {
    if (isSwitchModule(module))                                        
    {
      int pins = readSwitchModule(module);
      for (int pin = 0, mask = 1; pin < SWITCH_MODULE_SIZE; pin++, mask <<= 1)
      {
        int state = pins & mask;
        if (state != currentSwitchState[module] & mask)
        {
          processSwitch(module, pin, state);
        }
      }

      // Record new state.
      currentSwitchState[module] = pins;
    }
  }
}


/** Read the pins of a SwitchModule.
 *  Return the state of the pins, 16 bits, both ports.
 */
int readSwitchModule(int module)
{
  int value = 0;
  
  Wire.beginTransmission(module);    
  Wire.write(SWITCH_READ_DATA);
  Wire.requestFrom(module, 2);  // read the current GPIO output latches
  value = Wire.read() << 8;
        + Wire.read();
  Wire.endTransmission();    

  return value;
}


/** Process the changed switch.
 */
void processSwitch(int module, int pin, int state)
{
  
}


/** Send a coomand to an output module.
 *  Return error code if any.
 */
int sendCommand(uint8_t module, uint8_t pin, uint8_t value, uint8_t pace, uint8_t state)
{
  Wire.beginTransmission(module);
  Wire.write(pin);        
  Wire.write(value);  
  Wire.write(pace);   
  Wire.write(state);  
  return Wire.endTransmission();
}


///** Initialise servos.
// */
//void initServos()
//{
//  for (int servo = 0; servo < SERVO_MAX; servo++)
//  {
//    loadServo(servo);
//    saveServo();
//    loadSwitch(servo);
//    saveSwitch();
//    setServoModulePresent(2);
//    setSwitchModulePresent(3);
//  }
//}


/** Setup the Arduino.
 */
void setup()
{
  Serial.begin(115200);         // Serial IO.
  lcd.begin(16, 2);             // LCD panel.
  Wire.begin(controllerID);     // I2C network    

  mapHardware();                // Scan for attached hardware.
  initSwitches();               // Initialise all switches.
}


/** Main loop.
 */
void loop()
{
  if (readButton())       // Press any button to configure.
  {
    configure();
  }

  scanSwitches();
}
 
