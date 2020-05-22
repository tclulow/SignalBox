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
#include "Output.h"
#include "Input.h"
#include "Buttons.h"


// Initialize the LCD library with the numbers of the interface pins
// LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
LCD lcd(12, 11, 5, 4, 3, 2);

/** The i2c node IDs. */
const uint8_t controllerID = 0x10;    // Controller ID.
const uint8_t outputBaseID = 0x50;    // Base ID of the Output modules.
const uint8_t inputBaseID  = 0x20;    // Base ID of the Input modules.

// Record state of inputs.
int currentInputState[INPUT_MODULE_MAX];    // Current state of inputs.


/** Map the Output and Input modules.
 */
void mapHardware()
{
  lcd.clear();
  lcd.printAt(0, 0, M_SCAN_HARDWARE);
  lcd.print(CHAR_SPACE);
  
  // Scan for Output modules.
  for (int module = 0; module < INPUT_MODULE_MAX; module++)
  {
    Wire.beginTransmission(outputBaseID + module);
    if (Wire.endTransmission())
    {
      lcd.print(CHAR_DOT); 
    }
    else
    {
      lcd.print(module, HEX);
      setOutputModulePresent(module);  
    }
  }

  // Scan for Input modules.
  lcd.setCursor(0, 1);
  for (int module = 0; module < INPUT_MODULE_MAX; module++)
  {
    Wire.beginTransmission(inputBaseID + module);
    if (Wire.endTransmission())   
    {
      lcd.print(CHAR_DOT); 
    }
    else
    {  
      lcd.print(module, HEX);
      setInputModulePresent(module);
    }
  }
  delay(1000);
  // lcd.clear();
}


/** Configure all inputs for input.
 */
void initInputs()
{ 
  lcd.clear();
  lcd.printAt(0, 0, M_INIT_INPUTS);

  // Clear state of Inputs.
  for (int module = 0; module < INPUT_MODULE_MAX; module++)
  {
    currentInputState[module] = 0;
  }

  // For every Input module, set it's mode of operation.
  for(int module = 0; module < INPUT_MODULE_MAX; module++)
  {
    if (isInputModule(module))
    {
      lcd.print(module, HEX);
      Wire.beginTransmission(inputBaseID + module); 
      Wire.write(INPUT_PORTA_DIRECTION);
      Wire.write(0xFF);
      Wire.endTransmission();

      Wire.beginTransmission(inputBaseID + module);  
      Wire.write(INPUT_PORTB_DIRECTION);
      Wire.write(0xFF);
      Wire.endTransmission();

      Wire.beginTransmission(inputBaseID + module);
      Wire.write (INPUT_PORTA_PULLUPS);
      Wire.write(0xFF);
      Wire.endTransmission();  
       
      Wire.beginTransmission(inputBaseID + module);
      Wire.write(INPUT_PORTB_PULLUPS);
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


/** Scan all the inputs.
 *  Process any that have changed.
 */
void scanInputs()
{ 
  // Scan all the modules. 
  for (int module = 0; module < INPUT_MODULE_MAX; module++)
  {
    if (isInputModule(module))                                        
    {
      int pins = readInputModule(module);
      for (int pin = 0, mask = 1; pin < INPUT_MODULE_SIZE; pin++, mask <<= 1)
      {
        int state = pins & mask;
        if (state != currentInputState[module] & mask)
        {
          processInput(module, pin, state);
        }
      }

      // Record new state.
      currentInputState[module] = pins;
    }
  }
}


/** Read the pins of a InputModule.
 *  Return the state of the pins, 16 bits, both ports.
 */
int readInputModule(int module)
{
  int value = 0;
  
  Wire.beginTransmission(module);    
  Wire.write(INPUT_READ_DATA);
  Wire.requestFrom(module, 2);  // read the current GPIO output latches
  value = Wire.read() << 8;
        + Wire.read();
  Wire.endTransmission();    

  return value;
}


/** Process the changed input.
 */
void processInput(int module, int pin, int state)
{
  loadInput(module, pin);
  int output = inputData.output1 & INPUT_OUTPUT_MASK;
  loadOutput(output);
  
  if (inputData.output1 & INPUT_PUSH_TO_MAKE)
  {
    if (state)      // Send change state when button pressed, not when released.
    {
      outputData.mode ^= OUTPUT_STATE;    // Toggle the state.
      sendOutputCommand();                // Send to Output.
      saveOutput();
    }
  }
  else    // Toggle switch
  {
    if (state)
    {
      outputData.mode |= OUTPUT_STATE;    // Set output state
    }
    else
    {
      outputData.mode &= ~OUTPUT_STATE;   // Clear output state
    }
    
    sendOutputCommand();                  // Send change state to match that of the input.
    saveOutput();
  }
}


/** Send a command to an output module.
 *  Return error code if any.
 */
int sendOutputCommand()
{
  Wire.beginTransmission(outputNumber << OUTPUT_MODULE_SHIFT);
  Wire.write(outputNumber & OUTPUT_OUTPUT_MASK);
  if (outputData.mode & OUTPUT_STATE)
  {
    Wire.write(outputData.hi);
  }
  else
  {
    Wire.write(outputData.lo);
  }
  Wire.write(outputData.pace);   
  Wire.write((outputData.mode & OUTPUT_STATE) ? 1 : 0);  
  return Wire.endTransmission();
}


/** Setup the Arduino.
 */
void setup()
{
  Serial.begin(115200);         // Serial IO.
  lcd.begin(16, 2);             // LCD panel.
  Wire.begin(controllerID);     // I2C network    

  mapHardware();                // Scan for attached hardware.
  initInputs();                 // Initialise all inputs.
}


/** Main loop.
 */
void loop()
{
  if (readButton())       // Press any button to configure.
  {
    configure();
  }

  scanInputs();
}
 
