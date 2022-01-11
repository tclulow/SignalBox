/** Input definition.
 *  @file
 *
 *  (c)Copyright Tony Clulow  2021    tony.clulow@pentadtech.com
 *
 *  This work is licensed under the:
 *      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 *      http://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 *  For commercial use, please contact the original copyright holder(s) to agree licensing terms.
 */

#ifndef InputDef_h
#define InputDef_h


// Input nodes.
#define INPUT_NODE_MAX            8     // Maximum nodes.
#define INPUT_NODE_MASK        0x07     // 3 bits for 8 nodes.
#define INPUT_NODE_SHIFT          4     // Shift input number this amount to get a node number.
#define INPUT_PIN_MAX            16     // 16 inputs to each node.
#define INPUT_PIN_MASK         0x0f     // Mask to get input pin within a node.

// Mask for Input options
#define INPUT_OUTPUT_MAX          6     // Number of outputs each input can control. See also EEPROM in System.h
#define INPUT_OUTPUT_DISP         3     // Number of outputs each input can display.
// #define INPUT_DELAY_MASK             // Use the OUTPUT_PIN_MASK which is re-purposed for delay entries.

// Input types
#define INPUT_TYPE_MASK        0x03     // Input types take 2 bits each.
#define INPUT_TYPE_SHIFT          1     // Input type shifted by 1 bit = multiplied by 2.

#define INPUT_TYPE_TOGGLE         0     // A Toggle Input.
#define INPUT_TYPE_ON_OFF         1     // An on/off Input.
#define INPUT_TYPE_ON             2     // An on Input.
#define INPUT_TYPE_OFF            3     // An off Input.
#define INPUT_TYPE_MAX            4     // Limit of Input types.

#define INPUT_STATE_LEN           2     // Length on an Input MCP state message.


// Mask for MCP device none or all bits.
#define MCP_ALL_LOW   0x00
#define MCP_ALL_HIGH  0xFF

// MCP message commands.
#define MCP_IODIRA    0x00    // IO direction, High = input.
#define MCP_IODIRB    0x01
#define MCP_IPOLA     0x02    // Polarity, High = GPIO reversed.
#define MCP_IPOLB     0x03
#define MCP_GPINTENA  0x04    // Interupt enabled.
#define MCP_GPINTENB  0x05
#define MCP_DEFVALA   0x06    // Interupt compare value. Used if INTCON set.
#define MCP_DEFVALB   0x07
#define MCP_INTCONA   0x08    // Interup control, High = use DEFVAL, low = use previous value.
#define MCP_INTCONB   0x09
#define MCP_IOCON     0x0A    // Control register. Not used. See datasheet.
#define MCP_IOCON_DUP 0x0B
#define MCP_GPPUA     0x0C    // Pull-ups. High = pull-up resistor enabled.
#define MCP_GPPUB     0x0D
#define MCP_INTFA     0x0E    // Interupt occurred on these pins (read-only).
#define MCP_INTFB     0x0F
#define MCP_INTCAPA   0x10    // Interupt capture. Copy of GPIO when interups occurred.
#define MCP_INTCAPB   0x11    // Cleared when read (or when GPIO read).
#define MCP_GPIOA     0x12    // GPIO pins.
#define MCP_GPIOB     0x13
#define MCP_OLATA     0x14    // Output latches (connected to GPIO pins).
#define MCP_OLATB     0x15

// Commands required to initialise MCPs.
#define INPUT_COMMANDS_LEN 4
uint8_t INPUT_COMMANDS[] = { MCP_IODIRA, MCP_IODIRB, MCP_GPPUA, MCP_GPPUB };


/** Definition of an Input..
 */
class InputDef
{
    private:

    uint8_t delayMask = 0;              // Mask showing which outputs are "delay"s.
    uint8_t output[INPUT_OUTPUT_MAX];   // The outputs conrolled by this input.


    public:

    /** Gets the nth outputNode.
     */
    uint8_t getOutputNode(uint8_t aIndex)
    {
        return (output[aIndex] >> OUTPUT_NODE_SHIFT) & OUTPUT_NODE_MASK;
    }


    /** Sets the nth outputNode.
     */
    void setOutputNode(uint8_t aIndex, uint8_t aOutputNode)
    {
        output[aIndex] = (output[aIndex] & ~(OUTPUT_NODE_MASK << OUTPUT_NODE_SHIFT))
                       | ((aOutputNode   &  OUTPUT_NODE_MASK) << OUTPUT_NODE_SHIFT);
    }


    /** Gets the nth outputPin.
     */
    uint8_t getOutputPin(uint8_t aIndex)
    {
        return output[aIndex] & OUTPUT_PIN_MASK;
    }


    /** Sets the nth outputPin.
     */
    void setOutputPin(uint8_t aIndex, uint8_t aOutputPin)
    {
        output[aIndex] = (output[aIndex] & ~OUTPUT_PIN_MASK)
                       | (aOutputPin     &  OUTPUT_PIN_MASK);
    }


    /** Gets the nth outputNumber.
     */
    uint8_t getOutput(uint8_t aIndex)
    {
        return output[aIndex];
    }


    /** Sets the nth outputNumber.
     */
    void setOutput(uint8_t aIndex, uint8_t aOutputNumber)
    {
        output[aIndex] = aOutputNumber;
    }


    /** Is the nth output a delay?
     */
    boolean isDelay(uint8_t aIndex)
    {
        return (delayMask & (1 << aIndex)) != 0;
    }


    /** Sets the nth output delay (or not).
     */
    void setDelay(uint8_t aIndex, boolean aDelay)
    {
        uint8_t mask = 1 << aIndex;
        if (aDelay)
        {
            delayMask |= mask;
        }
        else
        {
            delayMask &= ~mask;
        }
    }


    /** Gets the index of the first Output that's a real output (not a delay).
     *  Return 0 if there are no outputs configured.
     */
    uint8_t getFirstOutput()
    {
        for (uint8_t index = 0; index < INPUT_OUTPUT_MAX; index++)
        {
            if (!isDelay(index))
            {
                return index;
            }
        }

        return 0;
    }


//    /** Gets the number of outputs this input drives.
//     *  Ignoring delay entries.
//     */
//    uint8_t getOutputCount()
//    {
//        uint8_t count = 0;
//
//        for (uint8_t index = 0; index < INPUT_OUTPUT_MAX; index++)
//        {
//            if (!isDelay(index))
//            {
//                count += 1;
//            }
//        }
//
//        return count;
//    }
};


#endif
