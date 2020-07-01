/** Common data.
 */
#ifndef _Common_h
#define _Common_h


#define MAGIC_NUMBER 0x50616f6e       // Magic number.
#define VERSION      0x0003           // Version number of software.


// i2c node numbers.
#define DEFAULT_I2C_CONTROLLER_ID   0x10    // Controller ID.
#define DEFAULT_I2C_INPUT_BASE_ID   0x20    // Input nodes' base ID.
#define DEFAULT_I2C_OUTPUT_BASE_ID  0x50    // Output nodes' base ID.


// Output information that's shared with the output module.
#define OUTPUT_TYPE_MASK       0x0f   // Only four bits are used for the type of output.
#define OUTPUT_TYPE_SERVO      0x00   // Output is a servo.
#define OUTPUT_TYPE_SIGNAL     0x01   // Output is a signal.
#define OUTPUT_TYPE_LED        0x02   // Output is a LED or other IO device.
#define OUTPUT_TYPE_MAX        0x03   // Limit of output types.
#define OUTPUT_TYPE_SHIFT         3   // Output type is shifted 3 bits when communicating to the OutputModule.
#define OUTPUT_PIN_MASK        0x07   // Mask to get output pin within a node.


#endif
