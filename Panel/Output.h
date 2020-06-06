/** Output data.
 */
#ifndef _Output_h
#define _Output_h

// Output nodes.
#define OUTPUT_NODE_SIZE   8      // 8 outputs to each node.
#define OUTPUT_NODE_MAX    16     // Maximum nodes.
#define OUTPUT_NODE_MASK   0x0f   // 4 bits for 16 nodes.
#define OUTPUT_NODE_SHIFT  3      // Shift output number this amount to get a node number.
#define OUTPUT_PIN_MASK    0x07   // Mask to get output pin within a node.

// OutputData saved in EEPROM
#define OUTPUT_BASE  SYSTEM_END                                 // EEPROM base of OutputData.
#define OUTPUT_SIZE  sizeof(OutputData)                         // Size of OutputData entry.
#define OUTPUT_MAX   (OUTPUT_NODE_SIZE * OUTPUT_NODE_MAX)   // Maximum outputs (up to 128).
#define OUTPUT_END   (OUTPUT_BASE + OUTPUT_SIZE * OUTPUT_MAX)   // End of OutputData EEPROM.

// Mask for OUTPUT options
#define OUTPUT_STATE        0x80
#define OUTPUT_MODE_MASK    0x0f
#define OUTPUT_MODE_NONE    0x00
#define OUTPUT_MODE_SERVO   0x01
#define OUTPUT_MODE_SIGNAL  0x02
#define OUTPUT_MODE_LED     0x03
#define OUTPUT_MODE_MAX     0x04


/** Data describing an Output's operation.
 */
struct OutputData
{
  uint8_t mode  = 0;
  uint8_t lo    = 0;
  uint8_t hi    = 0;
  uint8_t pace  = 0;
};


/** Variables for working with an Output.
 */
int        outputNodes = 0;   // Bit map of Output nodes present.
int        outputNumber  = 0;   // Current Output number.
OutputData outputData;          // Data describing current Output.


/** Load an Output's data from EEPROM.
 */
void loadOutput(int aOutput)
{
  if (aOutput < OUTPUT_MAX)
  {
    outputNumber = aOutput;
    EEPROM.get(OUTPUT_BASE + outputNumber * OUTPUT_SIZE, outputData); 
  }
}


/** Load an Output's data from EEPROM.
 */
void loadOutput(int aNode, int aOutput)
{
  loadOutput((aNode << OUTPUT_NODE_SHIFT) + (aOutput & OUTPUT_PIN_MASK));
}


/** Save an Output's data to EEPROM.
 *  Data in outputNumber and outputData.
 */
void saveOutput()
{
  if (outputNumber < OUTPUT_MAX)
  {
    EEPROM.put(OUTPUT_BASE + outputNumber * OUTPUT_SIZE, outputData);
  }
}


/** Record the presence of an OutputNode in the map.
 */
void setOutputNodePresent(int aNode)
{
  outputNodes |= (1 << aNode); 
}


/** Is an Output node present?
 *  Look for Output's node in outputNodes.
 */
boolean isOutputNode(int aNode)
{
  return (aNode < OUTPUT_NODE_MAX) && (outputNodes & (1 << aNode));
}


/** Is an Output present?
 *  Look for output's node in outputNodes.
 */
boolean isOutput(int aOutput)
{
  return isOutputNode(aOutput >> OUTPUT_NODE_SHIFT);
}

#endif
