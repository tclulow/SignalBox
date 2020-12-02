/* Communications between the modules.
 */
#ifndef _Comms_h
#define _Comms_h


#define COMMS_CMD_MASK      0xf0    // Top 4 bits.
#define COMMS_RFU_MASK      0x08    // RFU.
#define COMMS_PIN_MASK      0x07    // Bottom 3 bits.


#define COMMS_CMD_SET_LO    0x00    // go Lo
#define COMMS_CMD_SET_HI    0x10    // go Hi
#define COMMS_CMD_READ      0x20    // Read data from Output's definition (to the i2c master).    
#define COMMS_CMD_WRITE     0x30    // Write data to Output's definition (from the i2c master).

#define COMMS_CMD_NONE      0xff    // Null command.


#define COMMS_LEN_WRITE        4    // Length of data sent with a COMMS_CMD_WRITE


#endif
