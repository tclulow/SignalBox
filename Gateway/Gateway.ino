/** Gateway
 *  @file
 *
 *
 *  (c)Copyright Tony Clulow  2021  tony.clulow@pentadtech.com
 *
 *  This work is licensed under the:
 *      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 *      http://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 *  For commercial use, please contact the original copyright holder(s) to agree licensing terms.
 *
 *
 *  Libraries used:
 *
 *  Name              | Purpose
 *  ----------------- | -------
 *  EEPROM            | Reading and writing to EEPROM memory.
 *  Wire              | To handle i2c communications.
 *  ACAN              | CAN BUS communications
 *  CBUS              | CBUS protocol
 *  CBUS2515          | CBUS 2515 interface library
 *  CBUSconfig        | CBUS configuration
 *  cbusdefs          | CBUS constants
 *  CBUSParams        |
 *
 *
 *  Pin usage:
 *
 *  D0      Serial Rx.
 *  D1      Serial Tx.
 *  D2      CAN interupt.
 *  D10     CAN chip-select.
 *  D11     CAN SI.
 *  D12     CAN SO.
 *  D13     CAN SCK.
 * 
 *  A4      I2C SDA.
 *  A5      I2C SCL.
 *
 */


#include "Config.h"
#include "Gateway.h"


// I2C request command parameters
volatile uint8_t  requestCommand = COMMS_CMD_NONE;
volatile uint8_t  requestNode    = 0;


volatile uint8_t  outputNodeCount = 0;
volatile uint8_t  outputStates[OUTPUT_NODE_MAX];

volatile uint8_t  inputNodeCount = 0;
volatile uint16_t inputStates[OUTPUT_NODE_MAX];


// CBUS definitions
//unsigned char moduleName[] = "SB_GW  ";   // CBUS module name, 7 characters.
//CBUS2515      cbus2515;
//CBUSConfig    config;                     // Configuration object must be called "config".


///** Handle an incomming CBUS frame.
// */
//void cbusFrameHandler(CANFrame *aFrame)
//{
//    debugPrint(F("Frame"));
//    debugFrame(aFrame);
//    debugPrintln();
//}
//
//
///** Handle an incomming CBUS (learned) event.
// */
//void cbusEventHandler(byte index, CANFrame *aFrame)
//{
//    debugPrint(F("Event, index="));
//    debugPrintHex(index);
//    debugFrame(aFrame);
//    debugPrintln();
//}
//
//
///** Debug output of a CANFrame.
// */
//void debugFrame(CANFrame *aFrame)
//{
//    debugPrint(F(", id="));    
//    debugPrintHex(aFrame->id);
//    debugPrint(F(", ext="));
//    debugPrintHex(aFrame->ext);
//    debugPrint(F(", rtr="));
//    debugPrintHex(aFrame->rtr);
//    debugPrint(F(", len="));
//    debugPrintHex(aFrame->len);
//    debugPrint(F(", data="));
//    for (uint8_t ind = 0; ind < sizeof(aFrame->data); ind++)
//    {
//        debugPrintHex(aFrame->data[ind]);
//        debugPrint(' ');
//    }
//}
//
//
///** Initialise CBUS interface.
// */
//void initCbus()
//{
//    // Configure modules config (persistence of NVs and EVs)
//    config.EE_NVS_START       = 10;
//    config.EE_NUM_NVS         = 1;
//    config.EE_EVENTS_START    = 50;
//    config.EE_MAX_EVENTS      = 64;
//    config.EE_NUM_EVS         = 0;
//    config.EE_BYTES_PER_EVENT = 0;
//
//    config.setEEPROMtype(EEPROM_INTERNAL);
//    config.begin();
//
//    // Configure CBUS interface
//    CBUSParams params(config);
//    params.setVersion(CBUS_MAJ, CBUS_MIN, CBUS_BETA);
//    params.setModuleId(CBUS_ID);
//    params.setFlags(PF_FLiM | PF_PRODUCER);
//    cbus2515.setParams(params.getParams());
//
//    cbus2515.setName(moduleName);
//    cbus2515.setNumBuffers(2);
//    cbus2515.setOscFreq(CBUS_FREQ);
//    cbus2515.setPins(CBUS_PIN_CS, CBUS_PIN_INT);    // Chip select and interupt pins.
//
//    cbus2515.setEventHandler(cbusEventHandler);     // Handlers
//    cbus2515.setFrameHandler(cbusFrameHandler);
//
//    cbus2515.begin();
//}


/** Setup the Arduino.
 */
void setup()
{
    Serial.begin(19200);                    // Serial IO.

    i2cComms.setId(I2C_GATEWAY_ID);         // Start I2C communications.
    i2cComms.onReceive(processReceipt);
    i2cComms.onRequest(processRequest);

//    initCbus();                             // Start CBUS communications.

    debugPrint(F("Gateway"));
}


/** Report error.
 */
void reportError(const char* aMessage, uint8_t aCommand, uint8_t aOption)
{
    debugPrint(millis());
    debugPrint('\t');
    debugPrint(aMessage);
    debugPrint(F(", cmd="));
    debugPrintHex(aCommand);
    debugPrint(F(", opt="));
    debugPrint(aOption);
    debugPrintln();
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

//        debugPrintln();
//        debugPrint(millis());
//        debugPrint('\t');
//        debugPrint(F("Receipt"));
//        debugPrint(", cmd=");
//        debugPrintHex(command);
//        debugPrint(F(", opt="));
//        debugPrintHex(option);
//        debugPrint(F(", len="));
//        debugPrintHex(aLen);
//        debugPrintln();

        switch (command)
        {
            case COMMS_CMD_SYSTEM: processSystem(option);
                                   break;

            case COMMS_CMD_SET_LO:
            case COMMS_CMD_SET_HI:
            case COMMS_CMD_INP_LO:
            case COMMS_CMD_INP_HI: processStateChange(command, pin);
                                   break;

            default:               reportError("Unrecognised", command, option);
                                   break;
        }
    }
    else
    {
        // Null receipt - Just the master seeing if we exist.
        debugPrintln();
        debugPrint(millis());
        debugPrint('\t');
        debugPrint(F("Receipt, len="));
        debugPrintHex(aLen);
        debugPrintln();
    }

    // Consume unexpected data.
    if (i2cComms.available())
    {
        debugPrintln();
        debugPrint(millis());
        debugPrint('\t');
        debugPrint(F("Unexpected, len="));
        debugPrintHex(i2cComms.available());
        debugPrint(':');

        while (i2cComms.available())
        {
            uint8_t ch = i2cComms.readByte();
            {
                debugPrint(' ');
                debugPrintHex(ch);
            }
        }
        debugPrintln();
    }
}


/** Process a system receipt..
 */
void processSystem(uint8_t aOption)
{
    switch (aOption)
    {
        case COMMS_SYS_GATEWAY:    // At startup, the first polls are used to populate the output and input node states.
                                   if (outputNodeCount < OUTPUT_NODE_MAX)
                                   {
                                       requestCommand = COMMS_CMD_SYSTEM | COMMS_SYS_OUT_STATES;   // Ask for (next) Output node's states.
                                       requestNode = outputNodeCount;
                                   }
                                   else if (inputNodeCount < INPUT_NODE_MAX)
                                   {
                                       requestCommand = COMMS_CMD_SYSTEM | COMMS_SYS_INP_STATES;   // Ask for (next) Input node's states.
                                       requestNode = inputNodeCount;
                                   }
                                   else
                                   {
                                       // TODO - handle pending requests.
                                   }

                                   break;

        case COMMS_SYS_OUT_STATES: if (i2cComms.available() == 1)
                                   {
                                       outputStates[outputNodeCount] = i2cComms.readByte();
                                       debugPrint(millis());
                                       debugPrint(F("\tOutput node="));
                                       debugPrintHex(outputNodeCount);
                                       debugPrint(F(", states="));
                                       debugPrintHex(outputStates[outputNodeCount]);
                                       debugPrintln();

                                       outputNodeCount += 1;                    // Ensure next request is for next node
                                   }
                                   else
                                   {
                                       reportError("Missing output state", COMMS_CMD_SYSTEM, i2cComms.available());
                                   }

                                   break;

        case COMMS_SYS_INP_STATES: if (i2cComms.available() != 2)
                                   {
                                       inputStates[inputNodeCount] = (i2cComms.readByte() << 8) | i2cComms.readByte();
                                       debugPrint(millis());
                                       debugPrint(F("\tInput  node="));
                                       debugPrintHex(inputNodeCount);
                                       debugPrint(F(", states="));
                                       debugPrintHex(inputStates[inputNodeCount]);
                                       debugPrintln();

                                       inputNodeCount += 1;                    // Ensure next request is for next node
                                   }
                                   else
                                   {
                                       reportError("Missing input state", COMMS_CMD_SYSTEM, i2cComms.available());
                                   }

                                   break;

        default:                   reportError("Unrecognised system", COMMS_CMD_SYSTEM, aOption);
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

    debugPrint(millis());
    debugPrint('\t');
    debugPrint(isOutput ? F("Output ") : F("Input "));
    debugPrint(isHi ? F("Hi") : F("Lo"));
    debugPrint(F(", node="));
    debugPrintHex(node);
    debugPrint(F(", pin="));
    debugPrintHex(aPin);
    if (isOutput)
    {
        debugPrint(F(", delay="));
        debugPrintHex(delay);
    }
    debugPrintln();

//    sendCbusMessage((isHi     ? OPC_ACON    : OPC_ACON), 
//                    (isOutput ? 1           : 0),
//                    (isOutput ? (node << 3) : (node << 4)) | aPin);
}


///** Send a CBUS message.
// */
//void sendCbusMessage(uint8_t aOpCode, uint8_t aEventHi, uint8_t aEventLo)
//{
//    CANFrame frame;
//    frame.id      = config.CANID;
//    frame.len     = 5;
//    frame.data[0] = aOpCode;
//    frame.data[1] = highByte(config.nodeNum);
//    frame.data[2] = lowByte(config.nodeNum);
//    frame.data[3] = aEventHi;
//    frame.data[4] = aEventLo;
//
//    boolean sent = cbus2515.sendMessage(&frame);
//
//    debugPrint(millis());
//    debugPrint('\t');
//    debugPrint(F("CBUS, opCode="));
//    debugPrintHex(aOpCode);
//    debugPrint(F(", EventHi="));
//    debugPrintHex(aEventHi);
//    debugPrint(F(", EventLo="));
//    debugPrintHex(aEventLo);
//    if (sent)
//    {
//        debugPrint(F(" sent"));
//    }
//    else
//    {
//        debugPrint(F(" fail"));
//    }
//    debugPrintln();
//}


/** Main loop.
 */
void loop()
{
//    // Process CBUS stuff
//    cbus2515.process();
}
