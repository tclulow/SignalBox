/* Communications between the modules.
 */
#ifndef _Comms_h
#define _Comms_h


#define COMMS_CMD_MASK      0xf0    // Top 4 bits.
#define COMMS_RFU_MASK      0x08    // RFU.
#define COMMS_PIN_MASK      0x07    // Bottom 3 bits.


#define COMMS_CMD_STATES    0x00    // Read the state of all node's Outputs.
#define COMMS_CMD_DEBUG     0x10    // Set debug level.
#define COMMS_CMD_SET_LO    0x20    // Go Lo
#define COMMS_CMD_SET_HI    0x30    // Go Hi

#define COMMS_CMD_READ      0x40    // Read data from Output's EEPROM definition (to the i2c master).    
#define COMMS_CMD_WRITE     0x50    // Write data to Output's EEPROM definition (from the i2c master).
#define COMMS_CMD_SAVE      0x60    // Write data to Output's EEPROM definition and save it.
#define COMMS_CMD_RESET     0x70    // Reset output to its saved state (from its EEPROM).

#define COMMS_CMD_NONE      0xff    // Null command.


#define COMMS_LEN_WRITE        4    // Length of data sent with a COMMS_CMD_WRITE


#endif
