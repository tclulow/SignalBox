/** CMRInet processor.
 *  @file
 *
 *  (c)Copyright Tony Clulow  2021  tony.clulow@pentadtech.com
 *
 *  This work is licensed under the:
 *      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 *      http://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 *  For commercial use, please contact the original copyright holder(s) to agree licensing terms.
 */

#ifndef Cmri_h
#define Cmri_h


/** Possible states of the state machine.
 */
enum CmriState
{
    CMRI_IDLE,      // Not processing.
    CMRI_SYN,       // Expecting a SYN byte.
    CMRI_STX,       // Expecting a STX byte.
    CMRI_ADDR,      // Expecting address.
    CMRI_TYPE,      // Expecting type byte.
    CMRI_DATA,      // Expecting data.
    CMRI_DLE,       // Processing a DLE escape in a data stream.
    CMRI_ETX,       // Expecting an ETX character.
    CMRI_ETXD       // Processing a DLE escape when skipping to ETX.
};


// CMRI Message types.
const uint8_t TYPE_INIT     = 'I';
const uint8_t TYPE_POLL     = 'P';
const uint8_t TYPE_RECEIVE  = 'R';
const uint8_t TYPE_TRANSMIT = 'T';
const uint8_t TYPE_ERROR    = 'E';


/** A CMRI interface handler using a state machine.
 */
class Cmri
{
    private:

    Stream&   stream;                       // Stream for all the IO.
    uint8_t   currentByte   = 0;            // The current byte being processed.
    CmriState cmriState     = CMRI_IDLE;    // Current state of the state machine.
    uint8_t   address       = 0;            // Address of the current message.
    uint8_t   messageType   = 0;            // Type of the current message.
    uint8_t   nodeType      = 0;            // Node type.
    uint16_t  transDelay    = 0;            // Delay when transmitting data.
    uint8_t   sets          = 0;            // Number of sets.
    uint8_t   messageLength = 0;            // Length of data message (so far).


    public:

    /** A Cmri handler using the given Stream.
     */
    Cmri(Stream& aStream):
         stream(aStream)
    {
    }


    /** Update the state machine.
     */
    void update()
    {
        if (   (stream.available() > 0)                 // Serial characters to process
            && (   (cmriState != CMRI_IDLE)             // CMRI processing in progress
                || (stream.peek() == CHAR_SYN)))        // or start of CMRI message
        {
            processByte();
        }
    }


    /** Is the handler idle?
     *  Not currently receiving a message.
     */
    bool isIdle()
    {
        return cmriState == CMRI_IDLE;
    }


    private:

    /** Process a CMRI byte.
     */
    void processByte()
    {
        currentByte = stream.read();

        switch (cmriState)
        {
            case CMRI_IDLE: if (currentByte == CHAR_SYN)
                            {
                                cmriState = CMRI_SYN;       // Move to next state.
                                address       = 0;          // Address of the current message.
                                messageType   = 0;          // Type of the current message.
                                messageLength = 0;          // Length of data message (so far).
                            }
                            break;
            
            case CMRI_SYN:  if (currentByte == CHAR_SYN)    // Move to next state.
                            {
                                cmriState = CMRI_STX;
                            }
                            else
                            {
                                cmriState = CMRI_ETX;
                                // error("Syn ", CHAR_SYN, currentByte);
                            }
                            break;
                    
            case CMRI_STX:  if (currentByte == CHAR_STX)    // Move to next state.
                            {
                                cmriState = CMRI_ADDR;
                            }
                            else
                            {
                                cmriState = CMRI_ETX;
                                // error("Stx ", CHAR_STX, currentByte);
                            }
                            break;

            case CMRI_ADDR: address = currentByte;          // Record the address the message is for (starts with 'A').
                            cmriState = CMRI_TYPE;
                            break;

            case CMRI_TYPE: messageType = currentByte;      // Record the type of message.

                            // Initialise if it's an INIT message.
                            if (messageType == TYPE_INIT)
                            {
                                nodeType    = 0;            // Node type.
                                transDelay  = 0;            // Delay when transmitting data.
                                sets        = 0;            // Number of sets.
                            }
                            cmriState = CMRI_DATA;

//                            // Show message on display.
//                            if (   (systemMgr.isReportEnabled(REPORT_SHORT))
//                                && (messageType != TYPE_POLL))
//                            {
//                                disp.printProgStrAt(LCD_COL_START,    LCD_ROW_EDT, M_CMRI);
//                                disp.printHexByteAt(LCD_COL_CMRI,     LCD_ROW_EDT, address);
//                                disp.printHexByteAt(LCD_COL_CMRI + 3, LCD_ROW_EDT, messageType);
//                                controller.setDisplayTimeout(systemMgr.getReportDelay());
//                            }
                            break;

            case CMRI_DATA: if (currentByte == CHAR_DLE)        // Escape character.
                            {
                                cmriState = CMRI_DLE;           // Treat next character as data whatever it is.
                            }
                            else if (currentByte == CHAR_ETX)   // End of message.
                            {
                                cmriState = CMRI_IDLE;          // Finished processing.
                            }
                            else
                            {
                                processData();                  // Process a data byte.
                            }
                            break;

            case CMRI_DLE:  processData();                      // Process escaped data byte.
                            cmriState = CMRI_DATA;              // And return to normal data state.
                            break;

            case CMRI_ETX:  if (currentByte == CHAR_DLE)        // Escape character.
                            {
                                cmriState = CMRI_ETXD;          // Ensure escaped ETX character is ignored.
                            }
                            else if (currentByte == CHAR_ETX)   // Found ETX.
                            {
                                cmriState = CMRI_IDLE;          // Stop when found.
                            }
                            break;

            case CMRI_ETXD: cmriState = CMRI_ETX;               // Ignore character, resume scanning for ETX.
                            break;

            default:        cmriState = CMRI_ETX;
                            // error("State", -1, cmriState);      // Unexpected state.
                            break;
        }

        // Message is finished when we return to IDLE state.
        if (cmriState == CMRI_IDLE)
        {
//            // Show message on display.
//            if (   (systemMgr.isReportEnabled(REPORT_SHORT))
//                && (messageType != TYPE_POLL))
//            {
//                disp.printHexByteAt(-2, LCD_ROW_EDT, messageLength);
//                controller.setDisplayTimeout(systemMgr.getReportDelay());
//            }
            processEndOfMessage();
        }
    }


//    /** Check the expected character arrives and move to the next state.
//     */
//    void cmriCheck(uint8_t aExpected, CmriState aNextState)
//    {
//        if (currentByte == aExpected)
//        {
//            state = aNextState;         // Move to next state.
//        }
//        else
//        {
//            error("Expected", aExpected, currentByte);
//        }
//    }


    /** Process the current byte as data.
     */
    void processData()
    {
        switch(messageType)
        {
            case TYPE_INIT:     switch (messageLength)
                                {
                                    case 0:  nodeType = currentByte;
                                             break;
                    
                                    case 1:  transDelay = currentByte << 8;
                                             break;
                    
                                    case 2:  transDelay = transDelay | currentByte;
                                             break;
                    
                                    case 3:  sets = currentByte;
                    
//                                             // Show message on display.
//                                             if (systemMgr.isReportEnabled(REPORT_SHORT))
//                                             {
//                                                 disp.printHexByteAt(LCD_COL_CMRI,     LCD_ROW_BOT, nodeType);
//                                                 disp.printHexByteAt(LCD_COL_CMRI + 3, LCD_ROW_BOT, (transDelay >> 8) & 0xff);
//                                                 disp.printHexByte  (transDelay & 0xff);
//                                                 disp.printHexByteAt(LCD_COL_CMRI + 8, LCD_ROW_BOT, sets);
//                                                 controller.setDisplayTimeout(systemMgr.getReportDelay());
//                                             }
                                             break;
                    
                                    default: // Ignore any subsequent data
                                             // error("Init len", messageLength);
                                             break;
                                }
                                break;

            case TYPE_TRANSMIT: processTransByte();
                                break;

            case TYPE_POLL:
            case TYPE_RECEIVE:
            case TYPE_ERROR:    break;

            default:            // Ignore unrecognised message types.
                                // error("Type", -1, messageType);
                                break;
        }

        messageLength += 1;     // Record that a character has been processed.
    }


    /** Process a transmitted byte.
     *  Action the associated Input.
     */
    void processTransByte()
    {
//        if (systemMgr.isReportEnabled(REPORT_SHORT))
//        {
//            disp.printHexByteAt(messageLength * 3, LCD_ROW_BOT, currentByte);
//        }

        bool    isInput = messageLength < (INPUT_NODE_MAX * 2);                     // Input nodes sent first, 2 bytes per Input node, then Output nodes.
        uint8_t node    = (isInput ? (messageLength >> 1)                           // Inputs use two bytes per node.
                                   : (messageLength - (INPUT_NODE_MAX * 2)));       // Outputs start after 2 * INPUT_NODE_MAX bytes, 1 byte per node.
        uint8_t pin     = (isInput ? (messageLength & 1) << 3 : 0);                 // Start at pin zero except 2nd byte of Input node starts at 8.
        
        for (uint8_t mask = 1; mask > 0; pin++, mask <<= 1)
        {
            if ((currentByte & mask) != 0)                                          // CMRI signal high means fire the Input/Output.
            {
                if (isInput)
                {
                    controller.processInput(node, pin, false);
                }
                else
                {
                    controller.processOutput(node, pin, !getOutputState(node, pin), 0);
                }
            }
        }
    }


    /** Process end of message.
     *  Typically, send a response to a poll.
     *  Otherwise, normally nothing to do.
     */
    void processEndOfMessage()
    {
        // If message was a poll, respond with the current state.
        if (messageType == TYPE_POLL)
        {
            delay(50);                      // tiny delay to let things recover

            // Send CMRI header
            sendByte(CHAR_SYN, false);
            sendByte(CHAR_SYN, false);
            sendByte(CHAR_STX, false);
            sendByte(address,  false);
            sendByte(TYPE_RECEIVE, false);

            // Send Input node states.
            for (uint8_t node = 0; node < INPUT_NODE_MAX; node++)
            {
                uint16_t inputStates = controller.getInputState(node);
                sendByte((inputStates     ) & 0xff, true);
                sendByte((inputStates >> 8) & 0xff, true);
            }

            // Send Output node states.
            for (uint8_t node = 0; node < OUTPUT_NODE_MAX; node++)
            {
                sendByte(getOutputStates(node) & 0xff, true);
            }

            sendByte(CHAR_ETX, false);
            //stream.flush();
        }
    }


    /** Send a byte (as a character).
     *  If escape flag is set, precede special characters with CHAR_DLE.
     */
    void sendByte(uint8_t aByte, boolean escape)
    {
        if (escape)
        {
            if (aByte <= CHAR_DLE)
            {
                stream.write(CHAR_DLE);
            }
        }
        stream.write(aByte);
    }


//    /** Report an error in the protocol.
//     */
//    void error(const char* aMessage, int aOptional, uint8_t aByte)
//    {
//        cmriState = CMRI_ETX;           // Skip till an ETX
//        nodeType  = TYPE_ERROR;         // Ensure message isn't processed
//
//        // Display the error.
//        if (   (systemMgr.isReportEnabled(REPORT_SHORT)))
//        {
//            disp.clearBottomRows();
//            disp.printProgStrAt(LCD_COL_START,    LCD_ROW_EDT, M_CMRI);
//            disp.printHexByteAt(LCD_COL_CMRI,     LCD_ROW_EDT, address);
//            disp.printHexByteAt(LCD_COL_CMRI + 3, LCD_ROW_EDT, messageType);
//            disp.printHexByteAt(-2,               LCD_ROW_EDT, messageLength);
//
//            // disp.printProgStrAt(LCD_COL_START, LCD_ROW_EDT, M_FAILURE);
//            disp.setCursor(LCD_COL_START, LCD_ROW_BOT);
//            disp.printStr(aMessage);
//            if (aOptional >= 0)
//            {
//                disp.printHexByteAt(-5, LCD_ROW_BOT, aOptional);
//            }
//            disp.printHexByteAt(-2, LCD_ROW_BOT, aByte);
//
//            controller.setDisplayTimeout(DELAY_FAIL);
//        }
//        // delay(5000);
//    }
};


#endif
