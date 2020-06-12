/** Output data.
 */
#ifndef _Output_h
#define _Output_h

// Output nodes.
#define OUTPUT_NODE_SIZE          8   // 8 outputs to each node.
#define OUTPUT_NODE_MAX          16   // Maximum nodes.
#define OUTPUT_NODE_ALL_MASK 0xffff   // All output nodes present.
#define OUTPUT_NODE_MASK       0x0f   // 4 bits for 16 nodes.
#define OUTPUT_NODE_SHIFT         3   // Shift output number this amount to get a node number.
#define OUTPUT_PIN_MASK        0x07   // Mask to get output pin within a node.

// OutputData saved in EEPROM
#define OUTPUT_BASE  0                                          // EEPROM base of OutputData.
#define OUTPUT_SIZE  sizeof(OutputData)                         // Size of OutputData entry.
#define OUTPUT_MAX   (OUTPUT_NODE_SIZE * OUTPUT_NODE_MAX)       // Maximum outputs (up to 128).
#define OUTPUT_END   (OUTPUT_BASE + OUTPUT_SIZE * OUTPUT_MAX)   // End of OutputData EEPROM.

// Mask for OUTPUT options
#define OUTPUT_STATE           0x80   // On or off, switched or not switched, 0 = lo, 1 = hi.
#define OUTPUT_MODE_MASK       0x0f   // Only four bits are used for the type of output.
#define OUTPUT_MODE_SERVO      0x00   // Output is a servo.
#define OUTPUT_MODE_SIGNAL     0x01   // Output is a signal.
#define OUTPUT_MODE_LED        0x02   // Output is a LED or other IO device.
#define OUTPUT_MODE_MAX        0x03   // Limit of output types.

#define OUTPUT_PACE_MASK       0x0f   // Pace is in right-most nibble of output.pace.
#define OUTPUT_PACE_SHIFT         3   // Pace is shifted by this amount (multiplied by 8).
#define OUTPUT_PACE_OFFSET        4   // Pace is offset by this amount (add 4).
#define OUTPUT_PACE_INDEX         2   // Index of the Pace parameter
#define OUTPUT_DELAY_MASK       0xf   // Delay will be a nibble of output.pace.
#define OUTPUT_DELAY_SHIFT        4   // Delay will be in left-most nibble of output.pace.

#define OUTPUT_DEFAULT_LO        89   // Default low  position is 90 degrees - 1.
#define OUTPUT_DEFAULT_HI        91   // Default high position is 90 degrees + 1.
#define OUTPUT_DEFAULT_PACE       8   // Default pace is mid-range.


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
int        outputNodes  = 0;   // Bit map of Output nodes present.
int        outputNumber = 0;   // Current Output number.
OutputData outputData;         // Data describing current Output.


/** Load an Output's data from EEPROM.
 */
void loadOutput(int aOutput);


/** Load an Output's data from EEPROM.
 */
void loadOutput(int aNode, int aOutput);


/** Save an Output's data to EEPROM.
 *  Data in outputNumber and outputData.
 */
void saveOutput();


/** Record the presence of an OutputNode in the map.
 */
void setOutputNodePresent(int aNode);


/** Is an Output node present?
 *  Look for Output's node in outputNodes.
 */
boolean isOutputNode(int aNode);


/** Is an Output present?
 *  Look for output's node in outputNodes.
 */
boolean isOutput(int aOutput);

#endif
