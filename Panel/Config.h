/** Config data.
 */
#ifndef _Config_h
#define _Config_h

#define DEBUG true

#define DELAY 2000    // Delay in msecs when the LCD may need to be read by an operator.

#define DEFAULT_I2C_CONTROLLER_ID   0x10    // Controller ID.
#define DEFAULT_I2C_INPUT_BASE_ID   0x20    // Input modules' base ID.
#define DEFAULT_I2C_OUTPUT_BASE_ID  0x50    // Output modules' base ID.


// Define some fake modules (every third one).
#define FAKE_MODULE    && (module % 3)


#endif
