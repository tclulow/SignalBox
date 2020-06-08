/** Input data.
 */
#ifndef _Input_h
#define _Input_h

// Input nodes.
#define INPUT_NODE_SIZE      16     // 16 inputs to each node.
#define INPUT_NODE_MAX       8      // Maximum nodes.
#define INPUT_NODE_ALL_MASK  0xff   // All input nodes present.
#define INPUT_NODE_MASK      0x07   // 3 bits for 8 nodes.
#define INPUT_NODE_SHIFT     4      // Shift input number this amount to get a node number.
#define INPUT_PIN_MASK       0x0f   // Mask to get input pin within a node.

// InputData saved in EEPROM
#define INPUT_BASE   OUTPUT_END                                 // EEPROM base of Input data.
#define INPUT_SIZE   sizeof(InputData)                          // Size of InputData entry.
#define INPUT_MAX    (INPUT_NODE_SIZE * INPUT_NODE_MAX)     // Maximum inputs (up to 128).
#define INPUT_END    (INPUT_BASE + INPUT_SIZE * INPUT_MAX)      // End of Input EEPROM.

// Mask for Input options
#define INPUT_OUTPUT_MAX       3        // Number of outputs each input can control.
#define INPUT_TOGGLE_MASK      0x80     // The input is a toggle rather than a button.
#define INPUT_DISABLED_MASK    0x80     // The Input's output is disabled.
#define INPUT_OUTPUT_MASK      0x7f     // mask to get the Input's output without the flags above.

// Mask for none or all bits.
#define MCP_ALL_LOW   0x00
#define MCP_ALL_HIGH  0xFF

// Input message commands.
#define MCP_IODIRA    0x00    // IO direction, High = input.
#define MCP_IODIRB    0x01
#define MCP_IPOLA     0x02    // Polarity, High = GPIO reversed.
#define MCP_IPOLB     0x03
#define MCP_GPINTENA  0x04    // Interupt enabled.
#define MCP_GPINTENB  0x05
#define MCP_DEFVALA   0x06    // Interupt compare value. Used if INTCON set.
#define MCP_DEFVALB   0x07
#define MCP_INTCONA   0x08    // Interup control, High = use DEFVAL, low = use previous value.
#define MCP_INTCONB   0x09
#define MCP_IOCON     0x0A    // Control register. Not used. See datasheet.
#define MCP_IOCON_DUP 0x0B
#define MCP_GPPUA     0x0C    // Pull-ups. High = pull-up resistor enabled.
#define MCP_GPPUB     0x0D
#define MCP_INTFA     0x0E    // Interupt occurred on these pins (read-only).
#define MCP_INTFB     0x0F
#define MCP_INTCAPA   0x10    // Interupt capture. Copy of GPIO when interups occurred. 
#define MCP_INTCAPB   0x11    // Cleared when read (or when GPIO read).
#define MCP_GPIOA     0x12    // GPIO pins.
#define MCP_GPIOB     0x13
#define MCP_OLATA     0x14    // Output latches (connected to GPIO pins).
#define MCP_OLATB     0x15


/** Data describing an Input's operation.
 */
struct InputData
{
  uint8_t output[INPUT_OUTPUT_MAX];   // The outputs conrolled by this input.
};


/** Variables for working with an Input.
 */
int        inputNodes  = 0;   // Bit map of Input nodes present.
int        inputNumber = 0;   // Current Input number.
InputData  inputData;         // Data describing current Servo.


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
void loadInput(int aNode, int aInput)
{
  loadInput((aNode << INPUT_NODE_SHIFT) + (aInput & INPUT_PIN_MASK));
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


/** Record the presence of an InputNode in the map.
 */
void setInputNodePresent(int aNode)
{
  inputNodes |= (1 << aNode); 
}


/** Is an Input node present?
 *  Look for input's node in inputNodes.
 */
boolean isInputNode(int aNode)
{
  return (aNode < INPUT_NODE_MAX) && (inputNodes & (1 << aNode));
}


/** Is an Input present?
 *  Look for input's node in inputNodes.
 */
boolean isInput(int aInput)
{
  return isInputNode(aInput >> INPUT_NODE_SHIFT);
}

#endif
