/** Input data.
 */
#ifndef _Input_h
#define _Input_h

// Input modules.
#define INPUT_MODULE_SIZE    16     // 16 inputs to each module.
#define INPUT_MODULE_MAX     8      // Maximum modules.
#define INPUT_MODULE_MASK    0x07   // 3 bits for 8 modules.
#define INPUT_MODULE_SHIFT   4      // Shift input number this amount to get a module number.
#define INPUT_INPUT_MASK     0x0f   // Mask to get input number within a module.
#define INPUT_OUTPUT_MAX     3      // Number of outputs each input can control.

// InputData saved in EEPROM
#define INPUT_BASE   OUTPUT_END                                 // EEPROM base of Input data.
#define INPUT_SIZE   sizeof(InputData)                          // Size of InputData entry.
#define INPUT_MAX    (INPUT_MODULE_SIZE * INPUT_MODULE_MAX)     // Maximum inputs (up to 128).
#define INPUT_END    (INPUT_BASE + INPUT_SIZE * INPUT_MAX)      // End of Input EEPROM.

// Mask for Input options
#define INPUT_TOGGLE_MASK      0x80     // The input is a toggle rather than a button.
#define INPUT_DISABLED_MASK    0x80     // The Input's output is disabled.
#define INPUT_OUTPUT_MASK      0x7f     // mask to get the Input's output without the flags above.

// Input message commands.
#define INPUT_IODIRA    0x00    // IO direction, High = input.
#define INPUT_IODIRB    0x01
#define INPUT_IPOLA     0x02    // Polarity, High = GPIO reversed.
#define INPUT_IPOLB     0x03
#define INPUT_GPINTENA  0x04    // Interupt enabled.
#define INPUT_GPINTENB  0x05
#define INPUT_DEFVALA   0x06    // Interupt compare value. Used if INTCON set.
#define INPUT_DEFVALB   0x07
#define INPUT_INTCONA   0x08    // Interup control, High = use DEFVAL, low = use previous value.
#define INPUT_INTCONB   0x09
#define INPUT_IOCON     0x0A    // Control register. Not used. See datasheet.
#define INPUT_IOCON_DUP 0x0B
#define INPUT_GPPUA     0x0C    // Pull-ups. High = pull-up resistor enabled.
#define INPUT_GPPUB     0x0D
#define INPUT_INTFA     0x0E    // Interupt occurred on these pins (read-only).
#define INPUT_INTFB     0x0F
#define INPUT_INTCAPA   0x10    // Interupt capture. Copy of GPIO when interups occurred. 
#define INPUT_INTCAPB   0x11    // Cleared when read (or when GPIO read).
#define INPUT_GPIOA     0x12    // GPIO pins.
#define INPUT_GPIOB     0x13
#define INPUT_OLATA     0x14    // Output latches (connected to GPIO pins).
#define INPUT_OLATB     0x15

#define INPUT_ALL_LOW   0x00
#define INPUT_ALL_HIGH  0xFF



/** Data describing an Input's operation.
 */
struct InputData
{
  uint8_t output[INPUT_OUTPUT_MAX];   // The outputs conrolled by this input.
};


/** Variables for working with an Input.
 */
int        inputModules = 0;    // Bit map of Input modules present.
int        inputNumber  = 0;    // Current Input number.
InputData  inputData;           // Data describing current Servo.


/** Load an Input's data from EEPROM.
 */
void loadInput(int aInput)
{
  if (aInput < INPUT_MAX)
  {
    inputNumber = aInput;
    EEPROM.get(INPUT_BASE + inputNumber * INPUT_SIZE, inputData);
  }
}


/** Load an Input's data from EEPROM.
 */
void loadInput(int aModule, int aInput)
{
  loadInput((aModule << INPUT_MODULE_SHIFT) + (aInput & INPUT_INPUT_MASK));
}


/** Save an Input's data to EEPROM.
 *  Data in inputNumber and inputData.
 */
void saveInput()
{
  if (inputNumber < INPUT_MAX)
  {
    EEPROM.put(INPUT_BASE + inputNumber * INPUT_SIZE, inputData);
  }
}


/** Record the presence of an InputModule in the map.
 */
void setInputModulePresent(int aModule)
{
  inputModules |= (1 << aModule); 
}


/** Is an Input module present?
 *  Look for input's module in inputModules.
 */
boolean isInputModule(int aModule)
{
  return (aModule < INPUT_MODULE_MAX) && (inputModules & (1 << aModule));
}


/** Is an Input present?
 *  Look for input's module in inputModules.
 */
boolean isInput(int aInput)
{
  return isInputModule(aInput >> INPUT_MODULE_SHIFT);
}

#endif
