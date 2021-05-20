/* Communications between the modules.
 *
 *
 *  (c)Copyright Tony Clulow  2021    tony.clulow@pentadtech.com
 *
 *  This work is licensed under the:
 *      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 *      http://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 *  For commercial use, please contact the original copyright holder(s) to agree licensing terms.
 *
 *
 *  Comms protocol.
 *
 *  Most commands are a simple (write) i2c message from the Master to the Output module.
 *  Some messages require a response (maybe several bytes) from the output module. 
 *  This is achieved by the master sending a write message indicating what's required,
 *  and then immediately issuing a read i2c message to read the response from the Output module.
 *  
 *  Basic message:      <CommandByte><Data byte>...
 *  Optional response:  <Response byte>...
 *  
 *  Command byte:   7 6 5 4   3 2 1 0
 *                  Command   Option
 *                  
 *  Command nibble: As defined below with COMMS_CMD_... flags
 *  Option  nibble: Either the system command option (COMMS_SYS_... flags below) or the pin (0-7) to operate the command against.
 *  
 *  
 *  Messages:
 *  
 *      Command Option      Data                    Response
 *      SYSTEM  STATES                              <PinStatus>
 *      SYSTEM  RENUMBER    <NewNode>               <NewNode>
 *      SYSTEM  MOVE_LOCKS  <OldNode>   <NewNode>
 *      
 *      DEBUG   <Level>
 *      SET_LO  <Pin>       [Delay]
 *      SET_HI  <Pin>       [Delay]
 *      
 *      READ    <Pin>                               <OutputDef>
 *      WRITE   <Pin>       <OutputDef>
 *      SAVE    <Pin>
 *      RESET   <Pin>
 *      
 *      NONE    0xf
 *
 *      
 * Data bytes
 *      NewNode     The new number (0-31) for this output module.
 *      Level       The debug level to set (0-4). See DEBUG_... flags.
 *      Pin         The pin (0-7) to action the command against.
 *      Delay       Optional delay (in seconds, 0-255) before actioning the command.
 *      OutputDef   15 bytes defining an output. See below.
 *      
 * Response bytes
 *      PinStatus   The current status of all output pins. Pin 0 in bit 0, to Pin 7 in bit 7. Bit set = pin is "Hi".
 *      NewNode     The new node number (0-31) of the output module.
 *      OldNode     The old node number (0-31) of the output module.
 *      OutputDef   15 bytes defining an output. See below.
 *      
 * OutputDef
 *      Type        Byte indicating the type of output (see OUTPUT_TYPE_...).
 *      Lo          The Lo setting for this output (0-255).
 *      Hi          The Hi setting for this output (0-255).
 *      Pace        The pace (speed) at which to operate (0-15).
 *      Reset       The interval (in seconds) after which to reset (0-255).
 *      Locks       Mask indicating which interlocks are active. Bottom nibble for 4 Lo locks, top nibble for 4 Hi locks.
 *      LocksLo     Four bytes indicating the 4 Lo locks. See Lock below.
 *      LocksHi     Four bytes indicating the 4 Hi locks. See Lock below.
 *      Lock        Byte defining an output node and pin. Node number (0-31) in top 5 bits, pin number (0-7) in bottom 3 bits. See OUTPUT_NODE_... and OUTPUT_PIN_...
 */
 
#ifndef Comms_h
#define Comms_h


// Command byte.
#define COMMS_COMMAND_MASK      0xf0    // Top 4 bits.
#define COMMS_OPTION_MASK       0x0f    // Bottom 4 bits.
#define COMMS_COMMAND_SHIFT        4    // If command required as (4 bit) integer.


// Commands (in top nibble).
#define COMMS_CMD_SYSTEM        0x00    // System commands.
#define COMMS_CMD_DEBUG         0x10    // Set debug level.
#define COMMS_CMD_SET_LO        0x20    // Go Lo
#define COMMS_CMD_SET_HI        0x30    // Go Hi

#define COMMS_CMD_READ          0x40    // Read data from Output's EEPROM definition (to the i2c master).    
#define COMMS_CMD_WRITE         0x50    // Write data to Output's EEPROM definition (from the i2c master).
#define COMMS_CMD_SAVE          0x60    // Save Output's EEPROM definition (as set by a previous WRITE).
#define COMMS_CMD_RESET         0x70    // Reset output to its saved state (from its EEPROM).

#define COMMS_CMD_NONE          0xff    // Null command.


// System sub-commands (in bottom nibble)
#define COMMS_SYS_STATES        0x00    // System states sub-command.
#define COMMS_SYS_RENUMBER      0x01    // System renumber node sub-command.
#define COMMS_SYS_MOVE_LOCKS    0x02    // System renumber lock node numbers.


#endif
