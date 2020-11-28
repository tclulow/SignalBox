/* Communications between the modules.
 */
#ifndef _Comms_h
#define _Comms_h


#define COMMS_CMD_MASK      0xf0    // Top 4 bits.
#define COMMS_RFU_MASK      0x08    // RFU.
#define COMMS_PIN_MASK      0x07    // Bottom 3 bits.


#define COMMS_CMD_SET_HI    0x10    // go Hi
#define COMMS_CMD_SET_LO    0x20    // go Lo


#endif
