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
#include "System.h"
#include "Output.h"
#include "Input.h"
#include "Buttons.h"
#include "Configure.h"


// Record state of inputs.
uint16_t currentInputState[INPUT_MODULE_MAX];    // Current state of inputs.


/** Announce ourselves.
 */
void announce()
{
  lcd.clear();
  lcd.printAt(LCD_COL_START,                       LCD_ROW_TOP, M_SOFTWARE);
  lcd.printAt(LCD_COLS - strlen_P(M_VERSION),      LCD_ROW_TOP, M_VERSION);
  lcd.printAt(LCD_COLS - strlen_P(M_VERSION_DATE), LCD_ROW_BOT, M_VERSION_DATE);
}


/** Map the Output and Input modules.
 */
void mapHardware()
{
  lcd.clear();
  lcd.printAt(LCD_COL_START, LCD_ROW_TOP, M_SCAN_MODULES);
  lcd.setCursor(LCD_COLS - INPUT_MODULE_MAX, 0);
  
  // Scan for Input modules.
  for (int module = 0; module < INPUT_MODULE_MAX; module++)
  {
    Wire.beginTransmission(systemData.i2cInputBaseID + module);
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
    Wire.beginTransmission(systemData.i2cOutputBaseID + module);
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
  lcd.printAt(LCD_COL_START, LCD_ROW_TOP, M_INIT_INPUTS);
  lcd.setCursor(LCD_COL_START, LCD_ROW_BOT);

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
      Wire.beginTransmission(systemData.i2cInputBaseID + module); 
      Wire.write(INPUT_IODIRA);
      Wire.write(0xFF);
      Wire.endTransmission();

      Wire.beginTransmission(systemData.i2cInputBaseID + module);  
      Wire.write(INPUT_IODIRB);
      Wire.write(0xFF);
      Wire.endTransmission();

      Wire.beginTransmission(systemData.i2cInputBaseID + module);
      Wire.write (INPUT_GPPUA);
      Wire.write(INPUT_ALL_HIGH);
      Wire.endTransmission();  
       
      Wire.beginTransmission(systemData.i2cInputBaseID + module);
      Wire.write(INPUT_GPPUB);
      Wire.write(INPUT_ALL_HIGH);
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


/** Software hasn't been run before.
 */
void firstRun()
{
  lcd.clear();
  lcd.printAt(LCD_COL_START, LCD_ROW_TOP, M_FIRST_RUN);

  // Initialise SystemData.
  systemData.magic   = MAGIC_NUMBER;
  systemData.version = VERSION;

  systemData.i2cControllerID = DEFAULT_I2C_CONTROLLER_ID;
  systemData.i2cInputBaseID  = DEFAULT_I2C_INPUT_BASE_ID;
  systemData.i2cOutputBaseID = DEFAULT_I2C_OUTPUT_BASE_ID;

  saveSystemData();

  // Inialise all inputs to have one output with the same number.
  lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_DEFAULT_INPUTS);
  for (int input = 0; input < INPUT_MAX; input++)
  {
    loadInput(input);
    inputData.output[0] = (((input % 2) == 0) ? INPUT_TOGGLE_MASK : 0) | input;
    inputData.output[1] = INPUT_DISABLED_MASK; 
    inputData.output[2] = INPUT_DISABLED_MASK; 
    saveInput();
  }

  // Inialise all outputs.
  lcd.clearRow(LCD_COL_START, LCD_ROW_BOT);
  lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_DEFAULT_OUTPUTS);
  for (int output = 0; output < OUTPUT_MAX; output++)
  {
    loadOutput(output);
    outputData.mode  = output % OUTPUT_MODE_MAX;
    outputData.lo    = output;
    outputData.hi    = 180;
    outputData.pace  = 61;
    saveOutput();
  }

  configure.run();
  
  delay(DELAY);
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
      int pins = readInputModule(systemData.i2cInputBaseID + module);
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
  Wire.write(INPUT_GPIOA);
  Wire.requestFrom(module, 2);  // read GPIO A & B
  value = Wire.read() << 8
        + Wire.read();
  if (Wire.endTransmission())
  {
    value = -1;   // Pretend all inputs high if comms error.
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
  Serial.print(HEX_CHARS[module & 0xf]);
  Serial.print(" ");
  Serial.print(HEX_CHARS[pin & 0xf]);
  Serial.print(" ");
  Serial.print(state ? "Hi" : "Lo");
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
  
  if (inputData.output[0] & INPUT_TOGGLE_MASK)
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
  else  // button input
  {
    if (state)      // Send change state when button pressed, not when released.
    {
      outputData.mode ^= OUTPUT_STATE;    // Toggle the state.
      sendOutputCommand();                // Send to Output.
      saveOutput();
    }
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
  Serial.print(HEX_CHARS[(outputNumber >> OUTPUT_MODULE_SHIFT) & 0xf]);
  Serial.print(" ");
  Serial.print(HEX_CHARS[outputNumber & OUTPUT_OUTPUT_MASK & 0xf]);
  Serial.print(" ");
  Serial.print((outputData.mode & OUTPUT_STATE) ? "Hi" : "Lo");
  Serial.println();

//  lcd.printAt(LCD_COL_OUTPUT,  LCD_ROW_OUTPUT, M_OUTPUT);
//  lcd.printAt(LCD_COL_MODULE,  LCD_ROW_OUTPUT, HEX_CHARS[outputNumber << OUTPUT_MODULE_SHIFT]);
//  lcd.printAt(LCD_COL_PIN,     LCD_ROW_OUTPUT, HEX_CHARS[outputNumber & OUTPUT_OUTPUT_MASK]);
//  lcd.printAt(LCD_COL_STATE,   LCD_ROW_OUTPUT, ((outputData.mode & OUTPUT_STATE) ? M_HI : M_LO));
//  delay(DELAY);
  #endif
  
  Wire.beginTransmission(systemData.i2cOutputBaseID + (outputNumber << OUTPUT_MODULE_SHIFT));
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
  Serial.print("System  ");
  Serial.print(SYSTEM_BASE, HEX);
  Serial.print(" to ");
  Serial.print(SYSTEM_END, HEX);
  Serial.println();
  Serial.print("Outputs ");
  Serial.print(OUTPUT_BASE, HEX);
  Serial.print(" to ");
  Serial.print(OUTPUT_END, HEX);
  Serial.println();
  Serial.print("Inputs  ");
  Serial.print(INPUT_BASE, HEX);
  Serial.print(" to ");
  Serial.print(INPUT_END, HEX);
  Serial.println();
  #endif

  // Initialise subsystems.
  lcd.begin(LCD_COLS, LCD_ROWS);            // LCD panel.
  Wire.begin(systemData.i2cControllerID);   // I2C network    

  // Discover and initialise attached hardware.
  mapHardware();                            // Scan for attached hardware.
  initInputs();                             // Initialise all inputs.

  // Deal with first run (software has never been run before).
  if (!loadSystemData())
  {
    firstRun();
  }

  // Report abscence of hardware.
  if (   (inputModules  == 0)
      || (outputModules == 0))
  {
    int row = LCD_ROW_TOP;
    lcd.clear();
    if (inputModules == 0)
    {
      lcd.printAt(LCD_COL_START, row++, M_NO_INPUTS);
    }
    if (outputModules == 0)
    {
      lcd.printAt(LCD_COL_START, row++, M_NO_OUTPUTS);
    }
    delay(DELAY);
  }

  // Announce ourselves.
  announce();
}


/** Main loop.
 */
void loop()
{
  int  loops    = 0;
  char active[] = "-\\|/";

  // Loop forever
  while (true)
  {
    // Press any button to configure.
    if (readButton())
    {
      configure.run();
      announce();
    }

    // Process any inputs
    scanInputs();           

    // Show activity.
    lcd.printAt(LCD_COL_START, LCD_ROW_BOT, active[loops++ & 0x3]);
  }
}
 
