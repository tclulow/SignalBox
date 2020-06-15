/** Control panel.
 */

#include <EEPROM.h>
#include <Wire.h>

#include "Config.h"
#include "EzyBus.h"
#include "Messages.h"
#include "Panel.h"
#include "Lcd.h"
#include "Output.h"
#include "Input.h"
#include "Buttons.h"
#include "System.h"
#include "Debug.h"
#include "ImportExport.h"
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


/** Map the Input and Output nodes.
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
    if (Wire.endTransmission())   
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
    if (Wire.endTransmission())
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

  // Report abscence of hardware (and default to all if necessary).
  if (   (inputNodes  == 0)
      || (outputNodes == 0))
  {
    int row = LCD_ROW_TOP;
    lcd.clear();
    if (inputNodes == 0)
    {
      lcd.printAt(LCD_COL_START, row++, M_NO_INPUTS);
      inputNodes  = INPUT_NODE_ALL_MASK;
    }
    if (outputNodes == 0)
    {
      lcd.printAt(LCD_COL_START, row++, M_NO_OUTPUTS);
      outputNodes = OUTPUT_NODE_ALL_MASK;
    }
    delay(DELAY_READ);
  }
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
}


/** Software hasn't been run before.
 */
void firstRun()
{
  lcd.clear();
  lcd.printAt(LCD_COL_START, LCD_ROW_TOP, M_SETUP);
  delay(DELAY_READ);

  // Initialise SystemData.
  systemData.magic   = MAGIC_NUMBER;
  systemData.version = VERSION;

  systemData.i2cControllerID = DEFAULT_I2C_CONTROLLER_ID;
  systemData.i2cInputBaseID  = DEFAULT_I2C_INPUT_BASE_ID;
  systemData.i2cOutputBaseID = DEFAULT_I2C_OUTPUT_BASE_ID;

  calibrateButtons();

  // Decide if EzyBus conversion required.
  if (ezyBusDetected())
  {
    lcd.clear();
    lcd.printAt(LCD_COL_START, LCD_ROW_TOP, M_EZY_FOUND);
    lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_EZY_UPDATE);

    if (waitForButton() == BUTTON_SELECT)
    {
      convertEzyBus();
    }
    else
    {
      defaultSetup();
    }
  }
  else
  {
    defaultSetup();
  }
    
  saveSystemData();
  delay(DELAY_READ);
}


/** Set the default initial setup
 *  Servos with mid-position and mid-pace.
 */
void defaultSetup()
{
  int input = 0;

  lcd.clear();
  lcd.printAt(LCD_COL_START, LCD_ROW_TOP, M_INITIALISING);
  lcd.setCursor(LCD_COL_START, LCD_ROW_BOT);
  
  for (int node = 0; node < OUTPUT_NODE_MAX; node++)
  {
    lcd.print(HEX_CHARS[node]);
    for (int pin = 0; pin < OUTPUT_NODE_SIZE; pin++)
    {
      // Create the output.
      loadOutput(node, pin);
      outputData.mode = OUTPUT_MODE_SERVO;
      outputData.lo   = OUTPUT_DEFAULT_LO;
      outputData.hi   = OUTPUT_DEFAULT_HI;
      outputData.pace = OUTPUT_DEFAULT_PACE;
      saveOutput();

      // Create an input.
      loadInput(input++);
      inputData.output[0] = (node << OUTPUT_NODE_SHIFT) | pin;
      inputData.output[1] = INPUT_DISABLED_MASK;
      inputData.output[2] = INPUT_DISABLED_MASK;
      saveInput();
    }
  }
}


/** Convert EzyBus configuration.
 */
void convertEzyBus()
{
  int input = 0;
  
  lcd.clear();
  lcd.printAt(LCD_COL_START, LCD_ROW_TOP, M_EZY_UPDATING);
  lcd.setCursor(LCD_COL_START, LCD_ROW_BOT);
  
  for (int node = 0; node < OUTPUT_NODE_MAX; node++)
  {
    lcd.print(HEX_CHARS[node]);
    for (int pin = 0; pin < OUTPUT_NODE_SIZE; pin++)
    {
      // Convert the output.
      loadOutput(node, pin);
      outputData.pace >>= OUTPUT_PACE_SHIFT;  // Pace was in steps of 4 (2-bits), drop one bit
      saveOutput();

      // Create an input.
      loadInput(input++);
      inputData.output[0] = INPUT_TOGGLE_MASK | (node << OUTPUT_NODE_SHIFT) | pin;
      inputData.output[1] = INPUT_DISABLED_MASK;
      inputData.output[2] = INPUT_DISABLED_MASK;
      saveInput();
    }
  }
}


/** Scan all the inputs.
 *  Process any that have changed.
 */
void scanInputs()
{ 
//  #if DEBUG
//  processInput(3, 4, 0x00);
//  processInput(3, 4, 0x80);
//  processInput(3, 5, 0x00);
//  processInput(3, 5, 0x80);
//  #endif 

  // Scan all the nodes. 
  for (int node = 0; node < INPUT_NODE_MAX; node++)
  {
    if (isInputNode(node))                                        
    {
      // Read current state of pins and if successful and there's been a change
      int pins = readInputNode(node);
      if (pins != currentInputState[node])
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
 *  Return current state if there's a communication error, 
 *  this will prevent any actions being performed.
 */
int readInputNode(int node)
{
  int value = 0;
  
  Wire.beginTransmission(systemData.i2cInputBaseID + node);    
  Wire.write(MCP_GPIOA);
  value = Wire.requestFrom(node, 2);  Exoect two bytes
  if (value != 2)
  {
    value = currentInputState[node];     // Pretend no change if wrong byte-count returned
  }
  else
  {
    // Read the two bytes.
    value = Wire.read() << 8
          + Wire.read();
    if (Wire.endTransmission())
    {
      value = currentInputState[node];  // Pretend no change if comms error.
    }
  }

  return value;
}


/** Process the changed input.
 */
void processInput(int aNode, int aPin, int aState)
{
//  #if DEBUG
//  // Report the input
//  Serial.print("Input  ");
//  Serial.print(aState ? "Hi" : "Lo");
//  Serial.print(" ");
//  Serial.print(HEX_CHARS[aNode & 0xf]);
//  Serial.print(" ");
//  Serial.print(HEX_CHARS[aPin & 0xf]);
//  Serial.println();
//  #endif

  if (debugEnabled(DEBUG_LOW))
  {
    lcd.clear();
    lcd.printAt(LCD_COL_START, LCD_ROW_TOP, M_INPUT);
    lcd.printAt(LCD_COL_STATE, LCD_ROW_TOP, (aState ? M_HI : M_LO));
    lcd.printAt(LCD_COL_NODE,  LCD_ROW_TOP, HEX_CHARS[aNode]);
    lcd.printAt(LCD_COL_PIN,   LCD_ROW_TOP, HEX_CHARS[aPin]);
  }

  loadInput(aNode, aPin);
  boolean isToggle = inputData.output[0] & INPUT_TOGGLE_MASK;

  // Process all the Input's outputs (if they're not disabled).
  for (int index = 0; index < INPUT_OUTPUT_MAX; index++)
  {
    if (   (index == 0)
        || (!(inputData.output[index] & INPUT_DISABLED_MASK)))
    {
      int output = inputData.output[index] & INPUT_OUTPUT_MASK;
      if (isToggle)
      {
        loadOutput(output);

        if (aState)
        {
          outputData.mode |= OUTPUT_STATE;    // Set output state
        }
        else
        {
          outputData.mode &= ~OUTPUT_STATE;   // Clear output state
        }
        
        sendOutputCommand((outputData.mode & OUTPUT_STATE ? outputData.hi : outputData.lo), outputData.pace, outputData.mode & OUTPUT_STATE);
        saveOutput();
      }
      else  // button input
      {
        if (!aState)      // Send change state when button pressed (goes low), not when released (goes high).
        {
          loadOutput(output);
          outputData.mode ^= OUTPUT_STATE;    // Toggle the state.
          sendOutputCommand((outputData.mode & OUTPUT_STATE ? outputData.hi : outputData.lo), outputData.pace, outputData.mode & OUTPUT_STATE);
          saveOutput();
        }
      }
    }
  }
}


/** Send a command to an output node.
 *  Return error code if any.
 */
int sendOutputCommand(int aValue, int aPace, int aState)
{
//  #if DEBUG
//  // Report output
//  Serial.print("Output ");
//  Serial.print(aState ? "Hi" : "Lo");
//  Serial.print(" ");
//  Serial.print(HEX_CHARS[(outputNumber >> OUTPUT_NODE_SHIFT) & OUTPUT_NODE_MASK]);
//  Serial.print(" ");
//  Serial.print(HEX_CHARS[(outputNumber                     ) & OUTPUT_PIN_MASK ]);
//  Serial.print(" ");
//  Serial.print(aValue, HEX);
//  Serial.print(" ");
//  Serial.print(HEX_CHARS[pace & OUTPUT_PACE_MASK]);
//  Serial.println();
//  #endif

  if (debugEnabled(DEBUG_LOW))
  {
    lcd.clearRow(LCD_COL_START, LCD_ROW_BOT);
    lcd.printAt(LCD_COL_START,  LCD_ROW_BOT, M_OUTPUT);
    lcd.printAt(LCD_COL_STATE,  LCD_ROW_BOT, (aState ? M_HI : M_LO));
    lcd.printAt(LCD_COL_NODE,   LCD_ROW_BOT, HEX_CHARS[(outputNumber >> OUTPUT_NODE_SHIFT) & OUTPUT_NODE_MASK]);
    lcd.printAt(LCD_COL_PIN,    LCD_ROW_BOT, HEX_CHARS[(outputNumber                     ) & OUTPUT_PIN_MASK ]);
    debugPause();
  }
  
  Wire.beginTransmission(systemData.i2cOutputBaseID + ((outputNumber >> OUTPUT_NODE_SHIFT) & OUTPUT_NODE_MASK));
  Wire.write(outputNumber & OUTPUT_PIN_MASK);
  Wire.write(aValue);
  Wire.write(((aPace & OUTPUT_PACE_MASK) << OUTPUT_PACE_SHIFT) + OUTPUT_PACE_OFFSET);
  Wire.write(aState ? 1 : 0);
  return Wire.endTransmission();
}


//uint8_t * heapPtr, * stackPtr;
//
//void checkMem(char* aMessage)
//{
//  stackPtr = (uint8_t *)malloc(4);  // use stackPtr temporarily
//  heapPtr = stackPtr;               // save value of heap pointer
//  free(stackPtr);                   // free up the memory again (sets stackPtr to 0)
//  stackPtr =  (uint8_t *)(SP);      // save value of stack pointer
//  Serial.print(aMessage);
//  Serial.print(" ");
//  Serial.print((int)heapPtr);
//  Serial.print(" ");
//  Serial.println((int)stackPtr);
//}

  
/** Setup the Arduino.
 */
void setup()
{
  Serial.begin(115200);           // Serial IO.
//  #if DEBUG
//  Serial.print("System  ");
//  Serial.print(SYSTEM_BASE, HEX);
//  Serial.print(" to ");
//  Serial.print(SYSTEM_END, HEX);
//  Serial.println();
//  Serial.print("Outputs ");
//  Serial.print(OUTPUT_BASE, HEX);
//  Serial.print(" to ");
//  Serial.print(OUTPUT_END, HEX);
//  Serial.println();
//  Serial.print("Inputs  ");
//  Serial.print(INPUT_BASE, HEX);
//  Serial.print(" to ");
//  Serial.print(INPUT_END, HEX);
//  Serial.println();
//  #endif

//  #if DEBUG
//  loadInput(3, 4);
//  inputData.output[0] = (0x1 << OUTPUT_NODE_SHIFT) | 7 | INPUT_TOGGLE_MASK; 
//  inputData.output[1] = (0xc << OUTPUT_NODE_SHIFT) | 5;
//  inputData.output[2] = INPUT_DISABLED_MASK;
//  saveInput();
//
//  loadInput(3, 5);
//  inputData.output[0] = (0x9 << OUTPUT_NODE_SHIFT) | 3; 
//  inputData.output[1] = INPUT_DISABLED_MASK;
//  inputData.output[2] = (0x6 << OUTPUT_NODE_SHIFT) | 1;
//  saveInput();
//
//  loadOutput(0x1, 7);
//  outputData.mode = OUTPUT_MODE_NONE;
//  outputData.lo   = 0x17;
//  outputData.hi   = 0x22;
//  outputData.pace = 0x33;
//  saveOutput();
//
//  loadOutput(0xc, 5);
//  outputData.mode = OUTPUT_MODE_SERVO;
//  outputData.lo   = 0xc5;
//  outputData.hi   = 0x44;
//  outputData.pace = 0x55;
//  saveOutput();
//
//  loadOutput(0x9, 3);
//  outputData.mode = OUTPUT_MODE_SIGNAL;
//  outputData.lo   = 0x93;
//  outputData.hi   = 0x66;
//  outputData.pace = 0x77;
//  saveOutput();
//  
//  loadOutput(0x6, 1);
//  outputData.mode = OUTPUT_MODE_LED;
//  outputData.lo   = 0x61;
//  outputData.hi   = 0x88;
//  outputData.pace = 0xaa;
//  saveOutput();
//  #endif


  // Initialise subsystems.
  lcd.begin(LCD_COLS, LCD_ROWS);            // LCD panel.
  Wire.begin(systemData.i2cControllerID);   // I2C network
  pinMode(PIN_CALIBRATE, INPUT_PULLUP);     // Calibration input pin (11).

  // Announce ourselves.
  announce();
  delay(DELAY_READ);

  // Deal with first run (software has never been run before).
  if (!loadSystemData() || ezyBusDetected())
  {
    firstRun();
  }
  else if (digitalRead(PIN_CALIBRATE) == 0)
  {
    // Calibration requested.
    calibrateButtons();
    saveSystemData();
  }

  // Discover and initialise attached hardware.
  mapHardware();                            // Scan for attached hardware.
  initInputs();                             // Initialise all inputs.

  // Announce ourselves.
  announce();
}


/** Main loop.
 */
void loop()
{
  int  loops    = 0;
  long lastLoop = millis();

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
    if (millis() - lastLoop > 200)
    {
      if (loops > 7)
      {
        loops = 0;
      }
      lcd.printAt((loops    ) & 0x7, LCD_ROW_BOT, CHAR_DOT);
      lcd.printAt((loops + 4) & 0x7, LCD_ROW_BOT, CHAR_SPACE);
      loops += 1;

      lastLoop = millis();
    }
  }
}
 
