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
#define OUTPUT_NODE_PIN_MASK      7   // 3 bits for 8 pins withing an output node.

// Output options maxima.
#define OUTPUT_SERVO_MAX        180   // Maximum value an angle output parameter can take.
#define OUTPUT_LED_MAX          255   // Maximum value a LED can take.
#define OUTPUT_HI_LO_SIZE         3   // Maximum digits in a Hi/Lo display.

// Masks for type, state, pace and delay options withing the outputData element.
#define OUTPUT_TYPE_MASK       0x0f   // Output type mask (4 bits).
#define OUTPUT_STATE           0x80   // On or off, switched or not switched, 0 = lo, 1 = hi.
#define OUTPUT_PACE_SHIFT         4   // Pace is in the left-most nibble.
#define OUTPUT_PACE_MASK       0x0f   // Pace is 4 bits.
#define OUTPUT_PACE_MULT          3   // Pace is shifted by this amount (multiplied by 8).
#define OUTPUT_PACE_OFFSET        4   // Pace is offset by this amount (add 4).
#define OUTPUT_DELAY_MASK      0x0f   // Delay will be right-most nibble of output.pace.

// Defaults when initialising
#define OUTPUT_DEFAULT_LO        89   // Default low  position is 90 degrees - 1.
#define OUTPUT_DEFAULT_HI        91   // Default high position is 90 degrees + 1.
#define OUTPUT_DEFAULT_PACE    0x80   // Default pace is mid-range, no delay.


/** Data describing an Output's operation.
 */
struct OutputData
{
    uint8_t type  = 0;
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
void loadOutput(int aNode, int aPin);


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
