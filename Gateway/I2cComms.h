/** Encapsulate Wire library for communications between the modules.
 *  @file
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
 *  I2C Comms protocol.
 *
 *  Most commands are a simple (write) I2C message from the Master to the Output module.
 *  Some messages require a response (maybe several bytes) from the output module.
 *  This is achieved by the master sending a write message indicating what's required,
 *  and then immediately issuing a read I2C message to read the response from the Output module.
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
 *      SYSTEM  GATEWAY                             <Request>    <Node>
 *      SYSTEM  OUT_STATES                          <OutStates>
 *      SYSTEM  INP_STATES                          <InpStates>
 *      SYSTEM  RENUMBER    <Node>      <NewNode>   <NewNode>
 *      SYSTEM  MOVE_LOCKS  <Node>      <NewNode>
 *
 *      DEBUG   <Level>
 *      SET_LO  <Pin>       <Node>      <Delay>
 *      SET_HI  <Pin>       <Node>      <Delay>
 *
 *      READ    <Pin>                               <OutputDef>
 *      WRITE   <Pin>       <OutputDef>
 *      SAVE    <Pin>
 *      RESET   <Pin>
 *      
 *      SET     <Pin>       <Value>
 *      
 *      INP_LO  <Pin>       <Node>
 *      INP_HI  <Pin>       <Node>
 *
 *      NONE    0xf
 *
 *
 * Data bytes
 *      Node        The node (0-31) being addressed.
 *      NewNode     The new number (0-31) for this output module.
 *      Level       The debug level to set (0-4). See DEBUG_... flags.
 *      Pin         The pin (0-7) to action the command against.
 *      Delay       Optional delay (in seconds, 0-255) before actioning the command.
 *      OutputDef   15 bytes defining an output. See below.
 *      Value       Value to set output to (0-255).
 *
 * Response bytes
 *      Request     The command (and option) requested by the gateway.
 *      OutStates   The current state of all output pins. Pin 0 in bit 0, to Pin 7 in bit 7. Bit set = pin is "Hi".
 *      InpStates   The current state of all input pins. High-order byte first, Pin 8 in bit 0, to Pin 15 in bit 7. Bit set = pin is "Hi".
 *                                                  then Low-order byte second, Pin 0 in bit 0, to Pin 7 in bit 7. Bit set = pin is "Hi".
 *      NewNode     The new node number (0-31) of the output module.
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

#ifndef I2cComms_h
#define I2cComms_h


#include <Wire.h>


// Command byte.
const uint8_t COMMS_COMMAND_MASK    = 0xf0;     // Top 4 bits.
const uint8_t COMMS_OPTION_MASK     = 0x0f;     // Bottom 4 bits.
const uint8_t COMMS_COMMAND_SHIFT   =    4;     // If command required as (4 bit) integer.


// Commands (in top nibble).
const uint8_t COMMS_CMD_SYSTEM      = 0x00;     // System commands.
const uint8_t COMMS_CMD_DEBUG       = 0x10;     // Set debug level.
const uint8_t COMMS_CMD_SET_LO      = 0x20;     // Set Output Lo
const uint8_t COMMS_CMD_SET_HI      = 0x30;     // Set Output Hi

const uint8_t COMMS_CMD_READ        = 0x40;     // Read data from Output's EEPROM definition (to the I2C master).
const uint8_t COMMS_CMD_WRITE       = 0x50;     // Write data to Output's EEPROM definition (from the I2C master).
const uint8_t COMMS_CMD_SAVE        = 0x60;     // Save Output's EEPROM definition (as set by a previous WRITE).
const uint8_t COMMS_CMD_RESET       = 0x70;     // Reset output to its saved state (from its EEPROM).
const uint8_t COMMS_CMD_SET         = 0x80;     // Set output to a discrete value

const uint8_t COMMS_CMD_INP_LO      = 0x90;     // Input went Lo
const uint8_t COMMS_CMD_INP_HI      = 0xA0;     // Input went Hi

const uint8_t COMMS_CMD_NONE        = 0xf0;     // Null command.


// System sub-commands (in bottom nibble)
const uint8_t COMMS_SYS_GATEWAY     = 0x00;     // System - any gateway request?
const uint8_t COMMS_SYS_OUT_STATES  = 0x01;     // System - Output states sub-command.
const uint8_t COMMS_SYS_INP_STATES  = 0x02;     // System - Input states sub-command.
const uint8_t COMMS_SYS_RENUMBER    = 0x03;     // System - renumber node sub-command.
const uint8_t COMMS_SYS_MOVE_LOCKS  = 0x04;     // System - renumber lock node numbers.


/** Class for handling i2c communications.
 */
class I2cComms
{
    private:

    uint8_t gatewayId     = 0;          // Marks the presence of an I2C gateway module.
                                        // Certain messages are duplicated to this module.

    public:

    /** I2cComms constructor.
     */
    I2cComms()
    {
        if (I2C_SPEED)
        {
            Wire.setClock(I2C_SPEED);   // Set custom clock speed if one was specified.
        }
    }


    /** Start I2C as a particular node
     */
    void setId(uint8_t aNodeId)
    {
        Wire.begin(aNodeId);
        // Wire.setWireTimeout(I2C_TIMEOUT);        // Timeout (microseconds) if protocol hangs.
   }


    /** Set the Receive handler.
    */
    void onReceive(void (*aHandler)(int))
    {
        Wire.onReceive(aHandler);
    }


    /** Set the Request handler.
    */
    void onRequest(void (*aHandler)(void))
    {
        Wire.onRequest(aHandler);
    }


    /** Is a particular node ID connected to the I2C bus?
     */
    bool exists(uint8_t aNodeId)
    {
        Wire.beginTransmission(aNodeId);
        return Wire.endTransmission() == 0;
    }


    /** Set the Id of the Gateway module.
     *  Certain messages get duplicated to this ID.
     */
    void setGateway(uint8_t aId)
    {
        gatewayId = aId;       // Marks the presence of an I2C gateway module.
    }


    /** Send an I2C message with no data.
     */
    uint8_t sendShort(uint8_t aNodeId, uint8_t aCommand)
    {
        Wire.beginTransmission(aNodeId);
        Wire.write(aCommand);
        return Wire.endTransmission();
    }


    /** Send an I2C message with data bytes.
     */
    uint8_t sendData(uint8_t aNodeId, uint8_t aCommand, int aDataByte1, int aDataByte2)
    {
        Wire.beginTransmission(aNodeId);
        Wire.write(aCommand);
        if (aDataByte1 >= 0)
        {
            Wire.write((uint8_t)aDataByte1);
        }
        if (aDataByte2 >= 0)
        {
            Wire.write((uint8_t)aDataByte2);
        }
        return Wire.endTransmission();
    }


    /** Send an I2C message to the Gateway (if there is one).
     */
    void sendGateway(uint8_t aCommand, int aDataByte1, int aDataByte2)
    {
        if (gatewayId > 0)
        {
            sendData(gatewayId, aCommand, aDataByte1, aDataByte2);
        }
    }


    /** Send an I2C message with payload.
     */
    uint8_t sendPayload(uint8_t aNodeId, uint8_t aCommand, void(* payload)())
    {
        Wire.beginTransmission(aNodeId);
        Wire.write(aCommand);
        payload();
        return Wire.endTransmission();
    }


    /** Send a byte.
     *  For use by payload functions.
     */
    size_t sendByte(uint8_t aByte)
    {
        return Wire.write(aByte);
    }


    /** Request a 1-byte response.
     *  Return the byte, or a negative number if failed.
     */
    int requestByte(uint8_t aNodeId)
    {
        Wire.requestFrom(aNodeId, (uint8_t)1);

        return Wire.read();
    }


    /** Request a packet (of aLength).
     *  Return true if correct packet-length arrives, else false.
     */
    bool requestPacket(uint8_t aNodeId, uint8_t aLength)
    {
        return    (Wire.requestFrom(aNodeId, aLength) == aLength)
               && (Wire.available() == aLength);
    }


    /** Request a Gateway command.
     *  Return true if command received.
     */
    bool requestGateway()
    {
        return    (gatewayId > 0)
               && (requestPacket(gatewayId, 2));
    }


    /** Gets the number of bytes available to read.
     */
    int available()
    {
        return Wire.available();
    }


    /** Reads a byte from the receive buffer.
     */
    int readByte()
    {
        return Wire.read();
    }


    /** Reads a word (16 bits, 2 bytes) from the I2C comms receive buffer.
     */
    int readWord()
    {
        return   (Wire.read() & 0xff)
               | (Wire.read() << 8);

    }


    /** Ignore all received data that's left in the receive buffer.
     */
    void readAll()
    {
        while (Wire.available())
        {
            Wire.read();
        }
    }
};


/** A singleton instance of the I2cComms class.
 */
I2cComms i2cComms;


#endif
