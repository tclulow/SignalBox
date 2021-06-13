/** Gateway
 *
 *
 *  (c)Copyright Tony Clulow  2021  tony.clulow@pentadtech.com
 *
 *  This work is licensed under the:
 *      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 *      http://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 *  For commercial use, please contact the original copyright holder(s) to agree licensing terms.
 */


#include "I2cComms.h"


#define OUTPUT_NODE_MAX      32        // Length of output node array.
#define INPUT_NODE_MAX        8        // Length of input node array.


// I2C request command parameters
volatile uint8_t  requestCommand = COMMS_CMD_NONE;
volatile uint8_t  requestNode    = 0;


volatile uint8_t  outputNodeCount = 0;
volatile uint8_t  outputStates[OUTPUT_NODE_MAX];

volatile uint8_t  inputNodeCount = 0;
volatile uint16_t inputStates[OUTPUT_NODE_MAX];


/** Setup the Arduino.
 */
void setup()
{
    Serial.begin(19200);     // Serial IO.


    // Start I2C communications.
    i2cComms.setId(0x11);
    i2cComms.onReceive(processReceipt);
    i2cComms.onRequest(processRequest);

    Serial.println("Gateway");
}


/** Report unrecognised command.
 */
void unrecognisedCommand(const char* aMessage, uint8_t aCommand, uint8_t aOption)
{
    Serial.print(millis());
    Serial.print('\t');
    Serial.print(aMessage);
    Serial.print(", cmd=");
    Serial.print(aCommand, HEX);
    Serial.print(", opt=");
    Serial.print(aOption);
    Serial.println();
}


/** Process a Request (for data).
 *  Send data to master.
 */
void processRequest()
{
    Wire.write(requestCommand);
    Wire.write(requestNode & 0x1F);

    // Clear pending command.
    requestCommand = COMMS_CMD_NONE;
}


/** Data received.
 *  Process the command.
 */
void processReceipt(int aLen)
{
    if (aLen > 0)
    {
        // Read the command byte.
        uint8_t command = i2cComms.readByte();
        uint8_t option  = command & COMMS_OPTION_MASK;
        uint8_t node    = 0;
        uint8_t pin     = option  & 0x07;
        
        command &= COMMS_COMMAND_MASK;

//        Serial.println();
//        Serial.print(millis());
//        Serial.print('\t');
//        Serial.print("Receipt");
//        Serial.print(", cmd=");
//        Serial.print(command, HEX);
//        Serial.print(", opt=");
//        Serial.print(option, HEX);
//        Serial.print(", len=");
//        Serial.print(aLen, HEX);
//        Serial.println();

        switch (command)
        {
            case COMMS_CMD_SYSTEM: processSystem(option);
                                   break;
                                   
            case COMMS_CMD_SET_LO:
            case COMMS_CMD_SET_HI:
            case COMMS_CMD_INP_LO:
            case COMMS_CMD_INP_HI: processStateChange(command, pin);
                                   break;
                              
            default:               unrecognisedCommand("Unrecognised", command, option);
                                   break;
        }
    }
    else
    {
        // Null receipt - Just the master seeing if we exist.
        Serial.println();
        Serial.print(millis());
        Serial.print('\t');
        Serial.print("Receipt, len=");
        Serial.print(aLen, HEX);
        Serial.println();
    }
    
    // Consume unexpected data.
    if (i2cComms.available())
    {
        Serial.println();
        Serial.print(millis());
        Serial.print('\t');
        Serial.print("Unexpected, len=");
        Serial.print(i2cComms.available(), HEX);
        Serial.print(':');
        
        while (i2cComms.available())
        {
            uint8_t ch = i2cComms.readByte();
            {
                Serial.print(' ');
                Serial.print(ch, HEX);
            }
        }
        Serial.println();
    }
}


/** Process a system receipt..
 */
void processSystem(uint8_t aOption)
{
    switch (aOption)
    {
        case COMMS_SYS_GATEWAY:    if (outputNodeCount < OUTPUT_NODE_MAX)
                                   {
                                       requestCommand = COMMS_CMD_SYSTEM | COMMS_SYS_OUT_STATES;   // Ask for (next) Output node's states.
                                       requestNode = outputNodeCount;
                                   }
                                   else if (inputNodeCount < INPUT_NODE_MAX)
                                   {
                                       requestCommand = COMMS_CMD_SYSTEM | COMMS_SYS_INP_STATES;   // Ask for (next) Input node's states.
                                       requestNode = inputNodeCount;
                                   }
                                   break;

        case COMMS_SYS_OUT_STATES: outputStates[outputNodeCount] = i2cComms.readByte();
                                   Serial.print(millis());
                                   Serial.print("\tOutput node=");
                                   Serial.print(outputNodeCount, HEX);
                                   Serial.print(", states=");
                                   Serial.print(outputStates[outputNodeCount], HEX);
                                   Serial.println();
                               
                                   outputNodeCount += 1;                    // Ensure next request is for next node
                                   break; 

        case COMMS_SYS_INP_STATES: inputStates[inputNodeCount] = (i2cComms.readByte() << 8) | i2cComms.readByte();
                                   Serial.print(millis());
                                   Serial.print("\tInput  node=");
                                   Serial.print(inputNodeCount, HEX);
                                   Serial.print(", states=");
                                   Serial.print(inputStates[inputNodeCount], HEX);
                                   Serial.println();
                               
                                   inputNodeCount += 1;                    // Ensure next request is for next node
                                   break; 

        default:                   unrecognisedCommand("Unrecognised system", COMMS_CMD_SYSTEM, aOption);
    }
}


/** Process a state-change command.
 */
void processStateChange(uint8_t aCommand, uint8_t aPin)
{
    uint8_t node     = 0;
    uint8_t delay    = 0;
    boolean isOutput =    (aCommand == COMMS_CMD_SET_LO)
                       || (aCommand == COMMS_CMD_SET_HI);
    boolean isHi     =    (aCommand == COMMS_CMD_INP_HI)
                       || (aCommand == COMMS_CMD_SET_HI);
                      
    if (i2cComms.available())           // Should be present for all state change commands.
    {
        node = i2cComms.readByte();
    }
    
    if (i2cComms.available())           // Set commands only.
    {
        delay = i2cComms.readByte();
    }
    
    Serial.print(millis());
    Serial.print('\t');
    Serial.print(isOutput ? "Output " : "Input ");
    Serial.print(isHi ? "Hi" : "Lo");
    Serial.print(", node=");
    Serial.print(node, HEX);
    Serial.print(", pin=");
    Serial.print(aPin, HEX);
    if (isOutput)
    {
        Serial.print(", delay=");
        Serial.print(delay, HEX);
    }
    Serial.println();
}


/** Main loop.
 */
void loop()
{
    // Loop code goes here.
}
