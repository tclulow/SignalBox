/** 
 *  Control panel.
 *  
 *  LCD constants and functions.
 *
 * The circuit:
 * LCD RS pin to digital pin 12m
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

#include "Config.h"
#include "Panel.h"
#include "Messages.h"
#include "Lcd.h"
#include "Output.h"
#include "Input.h"
#include "Buttons.h"
#include "Configure.h"


// Record state of inputs.
uint16_t currentInputState[INPUT_MODULE_MAX];    // Current state of inputs.


/** Map the Output and Input modules.
 */
void mapHardware()
{
  lcd.clear();
  lcd.printAt(0, 0, M_SCAN_HARDWARE);
  lcd.setCursor(LCD_COLS - INPUT_MODULE_MAX, 0);
  
  // Scan for Input modules.
  for (int module = 0; module < INPUT_MODULE_MAX; module++)
  {
    Wire.beginTransmission(inputBaseID + module);
    if (Wire.endTransmission()FAKE_MODULE)   
    {
      lcd.print(CHAR_DOT); 
    }
    else
    {  
      lcd.print(HEX_CHARS[module]);
      setInputModulePresent(module);
    }
  }

  // Scan for Output modules.
  lcd.setCursor(0, 1);
  for (int module = 0; module < OUTPUT_MODULE_MAX; module++)
  {
    Wire.beginTransmission(outputBaseID + module);
    if (Wire.endTransmission()FAKE_MODULE)
    {
      lcd.print(CHAR_DOT); 
    }
    else
    {
      lcd.print(HEX_CHARS[module]);
      setOutputModulePresent(module);  
    }
  }

  delay(DELAY);
  // lcd.clear();
}


/** Configure all inputs for input.
 */
void initInputs()
{ 
  lcd.clear();
  lcd.printAt(0, 0, M_INIT_INPUTS);
  lcd.setCursor(0, 1);

  // Clear state of Inputs, all high.
  for (int module = 0; module < INPUT_MODULE_MAX; module++)
  {
    currentInputState[module] = 0xffff;
  }

  // For every Input module, set it's mode of operation.
  for(int module = 0; module < INPUT_MODULE_MAX; module++)
  {
    if (isInputModule(module))
    {
      lcd.print(HEX_CHARS[module]);
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
  delay(DELAY);
  // lcd.clear();  
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
      // Read current state of pins and if successful and there's been a change
      int pins = readInputModule(inputBaseID + module);
      if (   (pins >= 0)
          && (pins != currentInputState[module]))
      {
        // Process all the changed pins.
        for (int pin = 0, mask = 1; pin < INPUT_MODULE_SIZE; pin++, mask <<= 1)
        {
          int state = pins & mask;
          if (state != (currentInputState[module] & mask))
          {
            processInput(module, pin, state);
          }
        }
      
        // Record new state.
        currentInputState[module] = pins;
      }
    }
  }
}


/** Read the pins of a InputModule.
 *  Return the state of the pins, 16 bits, both ports.
 *  Return negative number if there's a communication error.
 */
long readInputModule(int module)
{
  long value = 0;
  
  Wire.beginTransmission(module);    
  Wire.write(INPUT_READ_DATA);
  Wire.requestFrom(module, 2);  // read the current GPIO output latches
  value = Wire.read() << 8
        + Wire.read();
  if (Wire.endTransmission())
  {
    value = -1;
  }

  #ifdef FAKE_MODULE
  if ((millis() & 0x3ff) == 0)    // 1024 millisecs ~= 1 second.
  {
    value = millis() & 0xffff;
  }
  #endif
  
  return value;
}


/** Process the changed input.
 */
void processInput(int module, int pin, int state)
{
  #if DEBUG
  // Report the input
  Serial.print("Input  ");
  Serial.print(HEX_CHARS[module]);
  Serial.print(" ");
  Serial.print(HEX_CHARS[pin]);
  Serial.print(" ");
  Serial.print(state ? M_HI : M_LO);
  Serial.println();

//  lcd.clear();
//  lcd.printAt(LCD_COL_INPUT,  LCD_ROW_INPUT, M_INPUT);
//  lcd.printAt(LCD_COL_MODULE, LCD_ROW_INPUT, HEX_CHARS[module]);
//  lcd.printAt(LCD_COL_PIN,    LCD_ROW_INPUT, HEX_CHARS[pin]);
//  lcd.printAt(LCD_COL_STATE,  LCD_ROW_INPUT, (state ? M_HI : M_LO));
  #endif
  
  loadInput(module, pin);
  int output = inputData.output[0] & INPUT_OUTPUT_MASK;
  loadOutput(output);
  
  if (inputData.output[0] & INPUT_BUTTON_MASK)
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
  #if DEBUG
  // Report output
  Serial.print("Output ");
  Serial.print(HEX_CHARS[outputNumber << OUTPUT_MODULE_SHIFT]);
  Serial.print(" ");
  Serial.print(HEX_CHARS[outputNumber & OUTPUT_OUTPUT_MASK]);
  Serial.print(" ");
  Serial.print((outputData.mode & OUTPUT_STATE) ? M_HI : M_LO);
  Serial.println();

//  lcd.printAt(LCD_COL_OUTPUT,  LCD_ROW_OUTPUT, M_OUTPUT);
//  lcd.printAt(LCD_COL_MODULE,  LCD_ROW_OUTPUT, HEX_CHARS[outputNumber << OUTPUT_MODULE_SHIFT]);
//  lcd.printAt(LCD_COL_PIN,     LCD_ROW_OUTPUT, HEX_CHARS[outputNumber & OUTPUT_OUTPUT_MASK]);
//  lcd.printAt(LCD_COL_STATE,   LCD_ROW_OUTPUT, ((outputData.mode & OUTPUT_STATE) ? M_HI : M_LO));
//  delay(DELAY);
  #endif
  
  Wire.beginTransmission(outputBaseID + (outputNumber << OUTPUT_MODULE_SHIFT));
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
  #if DEBUG
  Serial.begin(115200);           // Serial IO.
  #endif
  
  lcd.begin(LCD_COLS, LCD_ROWS);  // LCD panel.
  Wire.begin(controllerID);       // I2C network    

  mapHardware();                  // Scan for attached hardware.
  initInputs();                   // Initialise all inputs.

  lcd.clear();
  lcd.printAt(0, 0, M_SOFTWARE);
  lcd.printAt(0, 1, M_VERSION);
}


/** Main loop.
 */
void loop()
{
  if (readButton())       // Press any button to configure.
  {
    configure.run();
  }

  scanInputs();
}
 
