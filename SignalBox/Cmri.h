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
    CMRI_DLE,       // Processing a DEL escape in a data stream.
    CMRI_ETX        // Expecting an ETX character.
};


// CMRI Message types.
const char TYPE_INIT     = 'I';
const char TYPE_POLL     = 'P';
const char TYPE_RECEIVE  = 'R';
const char TYPE_TRANSMIT = 'T';


/** A CMRI interface handler using a state machine.
 */
class Cmri
{
    private:

    int       currentByte = 0;              // The current byte being processed.
    CmriState state       = CMRI_IDLE;      // Current state of the state machine.
    char      address     = 0;              // Address of the current message.
    char      type        = 0;              // Type of the current message.
    char      node        = 0;              // Node type.
    uint16_t  delay       = 0;              // Delay when transmitting data.
    char      sets        = 0;              // Number of sets.
    uint8_t   length      = 0;              // Length of data message (so far).


    public:

    /** Update the state machine.
     */
    void update()
    {
        while (   (Serial.available() > 0)              // Serial characters to process
               && (   (state != CMRI_IDLE)              // CMRI processing in progress
                   || (Serial.peek() == CHAR_SYN)))     // or start of CMRI message
        {
            processChar();
        }
    }


    private:

    void processChar()
    {
        currentByte = Serial.read();
        if (currentByte < 0)
        {
            state = CMRI_IDLE;                  // Stop processing on error.
            systemFail(M_CMRI, currentByte);
        }
        else
        {
            switch (state)
            {
                case CMRI_IDLE: cmriCheck(CHAR_SYN, CMRI_SYN);      // Move to next state.
                                address = 0;                        // Address of the current message.
                                type    = 0;                        // Type of the current message.
                                length  = 0;                        // Length of data message (so far).
                                break;
                
                case CMRI_SYN:  cmriCheck(CHAR_SYN, CMRI_STX);      // Move to next state.
                                break;
                        
                case CMRI_STX:  cmriCheck(CHAR_STX, CMRI_ADDR);     // Move to next state.
                                break;
    
                case CMRI_ADDR: address = currentByte;              // Record the address the message is for.
                                state = CMRI_TYPE;
                                break;
    
                case CMRI_TYPE: type = currentByte;                 // Record the type of message.

                                // Initialise if it's an INIT message.
                                if (type == TYPE_INIT)
                                {
                                    node        = 0;                // Node type.
                                    delay       = 0;                // Delay when transmitting data.
                                    sets        = 0;                // Number of sets.
                                }
                                
                                state = CMRI_DATA;

                                // Show message on display.
                                if (systemMgr.isReportEnabled(REPORT_SHORT))
                                {
                                    disp.clear();
                                    disp.printProgStrAt(LCD_COL_START,    LCD_ROW_TOP, M_CMRI);
                                    disp.printHexByteAt(LCD_COL_CMRI,     LCD_ROW_TOP, address);
                                    disp.printChAt     (LCD_COL_CMRI + 3, LCD_ROW_TOP, type);
                                }

                                break;
    
                case CMRI_DATA: if (currentByte == CHAR_DLE)        // Escape character.
                                {
                                    state = CMRI_DLE;               // Treat next character as data whatever it is.
                                }
                                else if (currentByte == CHAR_ETX)   // End of message.
                                {
                                    state = CMRI_IDLE;              // Finished processing.
                                }
                                else
                                {
                                    processData();                  // Process a data byte.
                                }
                                break;
    
                case CMRI_DLE:  processData();                      // Process escaped data byte.
                                state = CMRI_DATA;                  // And return to normal data state.
                                break;
    
                case CMRI_ETX:  if (currentByte == CHAR_ETX)        // Skip till ETX.
                                {
                                    processEndOfMessage();
                                    state = CMRI_IDLE;              // Stop when found.
                                }
                                break;
    
                default:        systemFail(M_CMRI, state);          // Unexpected state.
                                state = CMRI_ETX;
                                break;
            }
        }
    }


    /** Check the expected character arrives and move to the next state.
     */
    void cmriCheck(int aExpected, CmriState aNextState)
    {
        if (currentByte == aExpected)
        {
            state = aNextState;         // Move to next state.
        }
        else
        {
            state  = CMRI_ETX;          // Ignore everything till CHAR_ETX.
            systemFail(M_CMRI, currentByte);
        }
    }


    /** Process the current byte as data.
     */
    void processData()
    {
        if (type == TYPE_INIT)
        {
            // Handle INIT message depending on how much we've received so far.
            switch (length)
            {
                case 0:  node = currentByte;
                         break;

                case 1:  delay = currentByte << 8;
                         break;

                case 2:  delay = delay | currentByte;
                         break;

                case 3:  sets = currentByte;

                         // Show message on display.
                         if (systemMgr.isReportEnabled(REPORT_SHORT))
                         {
                             disp.printHexByteAt(LCD_COL_CMRI,     LCD_ROW_DET, node);
                             disp.printHexByteAt(LCD_COL_CMRI + 3, LCD_ROW_DET, (delay >> 8) & 0xff);
                             disp.printHexByte  (delay & 0xff);
                             disp.printHexByteAt(LCD_COL_CMRI + 8, LCD_ROW_DET, sets);
                         }

                         break;

                default: systemFail(M_CMRI, length);
            }
        }
        else if (type == TYPE_TRANSMIT)
        {
            // TODO - handle incomming byte.
        }

        length += 1;        // Record that a character has been processed.
    }


    /** Process end of message.
     *  Typically, send a response to a poll.
     *  Otherwise, normally nothing to do.
     */
    void processEndOfMessage()
    {
        if (type == TYPE_POLL)
        {
            // TODO - send response.
        }
    }
};


/** Singleton instance. */
Cmri cmri;


#endif
