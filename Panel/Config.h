/** Config data.
 */
#ifndef _Config_h
#define _Config_h

#define DEBUG true

#define DELAY 2000    // Delay in msecs when the LCD may need to be read by an operator.

#define DEFAULT_I2C_CONTROLLER_ID   0x10    // Controller ID.
#define DEFAULT_I2C_INPUT_BASE_ID   0x20    // Input nodes' base ID.
#define DEFAULT_I2C_OUTPUT_BASE_ID  0x50    // Output nodes' base ID.


// Define some fake nodes (every third one).
#define FAKE_NODE    && (node % 3)


#endif
