/** Control panel.
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
uint16_t currentInputState[INPUT_NODE_MAX];    // Current state of inputs.


/** Announce ourselves.
 */
void announce()
{
  lcd.clear();
  lcd.printAt(LCD_COL_START,                       LCD_ROW_TOP, M_SOFTWARE);
  lcd.printAt(LCD_COLS - strlen_P(M_VERSION),      LCD_ROW_TOP, M_VERSION);
  lcd.printAt(LCD_COLS - strlen_P(M_VERSION_DATE), LCD_ROW_BOT, M_VERSION_DATE);
}


/** Map the Output and Input nodes.
 */
void mapHardware()
{
  lcd.clear();
  lcd.printAt(LCD_COL_START, LCD_ROW_TOP, M_SCAN_NODES);
  lcd.setCursor(LCD_COLS - INPUT_NODE_MAX, 0);
  
  // Scan for Input nodes.
  for (int node = 0; node < INPUT_NODE_MAX; node++)
  {
    Wire.beginTransmission(systemData.i2cInputBaseID + node);
    if (Wire.endTransmission()FAKE_NODE)   
    {
      lcd.print(CHAR_DOT); 
    }
    else
    {  
      lcd.print(HEX_CHARS[node]);
      setInputNodePresent(node);
    }
  }

  // Scan for Output nodes.
  lcd.setCursor(0, 1);
  for (int node = 0; node < OUTPUT_NODE_MAX; node++)
  {
    Wire.beginTransmission(systemData.i2cOutputBaseID + node);
    if (Wire.endTransmission()FAKE_NODE)
    {
      lcd.print(CHAR_DOT); 
    }
    else
    {
      lcd.print(HEX_CHARS[node]);
      setOutputNodePresent(node);  
    }
  }

  delay(DELAY_READ);
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
  for (int node = 0; node < INPUT_NODE_MAX; node++)
  {
    currentInputState[node] = 0xffff;
  }

  // For every Input node, set it's mode of operation.
  for(int node = 0; node < INPUT_NODE_MAX; node++)
  {
    if (isInputNode(node))
    {
      lcd.print(HEX_CHARS[node]);
      Wire.beginTransmission(systemData.i2cInputBaseID + node); 
      Wire.write(MCP_IODIRA);
      Wire.write(MCP_ALL_HIGH);
      Wire.endTransmission();

      Wire.beginTransmission(systemData.i2cInputBaseID + node);  
      Wire.write(MCP_IODIRB);
      Wire.write(MCP_ALL_HIGH);
      Wire.endTransmission();

      Wire.beginTransmission(systemData.i2cInputBaseID + node);
      Wire.write (MCP_GPPUA);
      Wire.write(MCP_ALL_HIGH);
      Wire.endTransmission();  
       
      Wire.beginTransmission(systemData.i2cInputBaseID + node);
      Wire.write(MCP_GPPUB);
      Wire.write(MCP_ALL_HIGH);
      Wire.endTransmission();  
    }
    else
    {
      lcd.print(CHAR_DOT);
    }
  }   
  delay(DELAY_READ);
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

  #if DEBUG
  systemData.buttons[0] = 840;    // Suitable values that should work.
  systemData.buttons[1] = 540;
  systemData.buttons[2] = 340;
  systemData.buttons[3] = 180;
  systemData.buttons[4] =  55;
  #else
  calibrateButtons();
  #endif

  saveSystemData();

//  // Inialise all inputs to have one output with the same number.
//  lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_DEFAULT_INPUTS);
//  for (int input = 0; input < INPUT_MAX; input++)
//  {
//    loadInput(input);
//    inputData.output[0] = (((input % 2) == 0) ? INPUT_TOGGLE_MASK : 0) | input;
//    inputData.output[1] = INPUT_DISABLED_MASK; 
//    inputData.output[2] = INPUT_DISABLED_MASK; 
//    saveInput();
//  }
//
//  // Inialise all outputs.
//  lcd.clearRow(LCD_COL_START, LCD_ROW_BOT);
//  lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_DEFAULT_OUTPUTS);
//  for (int output = 0; output < OUTPUT_MAX; output++)
//  {
//    loadOutput(output);
//    outputData.mode  = output % OUTPUT_MODE_MAX;
//    outputData.lo    = output;
//    outputData.hi    = 180;
//    outputData.pace  = 61;
//    saveOutput();
//  }

  configure.run();
  
  delay(DELAY_READ);
}


/** Scan all the inputs.
 *  Process any that have changed.
 */
void scanInputs()
{ 
  // Scan all the nodes. 
  for (int node = 0; node < INPUT_NODE_MAX; node++)
  {
    if (isInputNode(node))                                        
    {
      // Read current state of pins and if successful and there's been a change
      int pins = readInputNode(systemData.i2cInputBaseID + node);
      if (   (pins >= 0)
          && (pins != currentInputState[node]))
      {
        // Process all the changed pins.
        for (int pin = 0, mask = 1; pin < INPUT_NODE_SIZE; pin++, mask <<= 1)
        {
          int state = pins & mask;
          if (state != (currentInputState[node] & mask))
          {
            processInput(node, pin, state);
          }
        }
      
        // Record new state.
        currentInputState[node] = pins;
      }
    }
  }
}


/** Read the pins of a InputNode.
 *  Return the state of the pins, 16 bits, both ports.
 *  Return negative number if there's a communication error.
 */
long readInputNode(int node)
{
  long value = 0;
  
  Wire.beginTransmission(node);    
  Wire.write(MCP_GPIOA);
  Wire.requestFrom(node, 2);  // read GPIO A & B
  value = Wire.read() << 8
        + Wire.read();
  if (Wire.endTransmission())
  {
    value = -1;   // Pretend all inputs high if comms error.
  }

  #ifdef FAKE_NODE
  if ((millis() & 0x3ff) == 0)    // 1024 millisecs ~= 1 second.
  {
    value = millis() & 0xffff;
  }
  #endif
  
  return value;
}


/** Process the changed input.
 */
void processInput(int node, int pin, int state)
{
  #if DEBUG
  // Report the input
  Serial.print("Input  ");
  Serial.print(HEX_CHARS[node & 0xf]);
  Serial.print(" ");
  Serial.print(HEX_CHARS[pin & 0xf]);
  Serial.print(" ");
  Serial.print(state ? "Hi" : "Lo");
  Serial.println();

//  lcd.clear();
//  lcd.printAt(LCD_COL_INPUT,  LCD_ROW_INPUT, M_INPUT);
//  lcd.printAt(LCD_COL_NODE, LCD_ROW_INPUT, HEX_CHARS[node]);
//  lcd.printAt(LCD_COL_PIN,    LCD_ROW_INPUT, HEX_CHARS[pin]);
//  lcd.printAt(LCD_COL_STATE,  LCD_ROW_INPUT, (state ? M_HI : M_LO));
  #endif
  
  loadInput(node, pin);
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


/** Send a command to an output node.
 *  Return error code if any.
 */
int sendOutputCommand()
{
  #if DEBUG
  // Report output
  Serial.print("Output ");
  Serial.print(HEX_CHARS[(outputNumber >> OUTPUT_NODE_SHIFT) & 0xf]);
  Serial.print(" ");
  Serial.print(HEX_CHARS[outputNumber & OUTPUT_PIN_MASK & 0xf]);
  Serial.print(" ");
  Serial.print((outputData.mode & OUTPUT_STATE) ? "Hi" : "Lo");
  Serial.println();

//  lcd.printAt(LCD_COL_OUTPUT,  LCD_ROW_OUTPUT, M_OUTPUT);
//  lcd.printAt(LCD_COL_NODE,  LCD_ROW_OUTPUT, HEX_CHARS[outputNumber << OUTPUT_NODE_SHIFT]);
//  lcd.printAt(LCD_COL_PIN,     LCD_ROW_OUTPUT, HEX_CHARS[outputNumber & OUTPUT_OUTPUT_MASK]);
//  lcd.printAt(LCD_COL_STATE,   LCD_ROW_OUTPUT, ((outputData.mode & OUTPUT_STATE) ? M_HI : M_LO));
//  delay(DELAY_READ);
  #endif
  
  Wire.beginTransmission(systemData.i2cOutputBaseID + (outputNumber << OUTPUT_NODE_SHIFT));
  Wire.write(outputNumber & OUTPUT_PIN_MASK);
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
  if (   (inputNodes  == 0)
      || (outputNodes == 0))
  {
    int row = LCD_ROW_TOP;
    lcd.clear();
    if (inputNodes == 0)
    {
      lcd.printAt(LCD_COL_START, row++, M_NO_INPUTS);
    }
    if (outputNodes == 0)
    {
      lcd.printAt(LCD_COL_START, row++, M_NO_OUTPUTS);
    }
    delay(DELAY_READ);
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
 
