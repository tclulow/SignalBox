/** Config data.
 */
#ifndef _Config_h
#define _Config_h

#define DEBUG true

#define DELAY 2000    // Delay in msecs when the LCD may need to be read by an operator.


/** The i2c node IDs. */
const uint8_t controllerID = 0x10;    // Controller ID.
const uint8_t outputBaseID = 0x50;    // Base ID of the Output modules.
const uint8_t inputBaseID  = 0x20;    // Base ID of the Input modules.

// Define some fake modules (every third one).
#define FAKE_MODULE    && (module % 3)


#endif
