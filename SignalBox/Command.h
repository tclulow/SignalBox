/** Command processor.
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

#ifndef Command_h
#define Command_h

#define COMMAND_BUFFER_LEN   8                  // Serial command buffer length


/** Command class.
 *  Handles input from Serial input and executes the commands received.
 */
class Command
{
    private:

    char    commandBuffer[COMMAND_BUFFER_LEN + 1];  // Buffer to read characters with null terminator on the end.
    uint8_t commandLen = 0;                         // Length of command.


    public:
    
    /** Look for serial input and process it.
     */
    void update()
    {
        // Look for command characters
        if (Serial.available() > 0)
        {
            if (   (commandLen > 0)                     // Already processing a command.
                || (Serial.peek() != CHAR_SYN))         // Or character can't be a CMRI start-of-message
            {
                char ch = Serial.read();
                if (ch == CHAR_RETURN)
                {
                    // Ignore carriage-return
                }
                else if (   (ch == CHAR_NEWLINE)
                         || (ch == CHAR_COMMA))
                {
                    if (commandLen > 0)                 // Process the received command
                    {
                        commandBuffer[commandLen] = CHAR_NULL;
                        processCommand();
                        commandLen = 0;
                    }
                }
                else if (commandLen <= COMMAND_BUFFER_LEN)
                {
                    commandBuffer[commandLen++] = ch;   // Add the character to the command.
                }
            }
        }
    }


    /** Is the handler idle?
     *  Not currently receiving a command.
     */
    bool isIdle()
    {
        return commandLen == 0;
    }


    private:
    
    /** Process a received command.
     *  Using the contents of the commandBuffer:
     *      iNP - Action input for node N, pin P.
     *      lNP - Action output Lo for node N, pin P.
     *      hNP - Action output Hi for node N, pin P.
     *      oNP - Action output Hi/Lo (based on current state) for node N, pin P.
     */
    void processCommand()
    {
        bool    executed = false;
        uint8_t node     = 0;
        uint8_t pin      = 0;
        bool    state    = true;        // Default to setting state high - see switch statement.
    
        if (isDebug(DEBUG_BRIEF))
        {
            Serial.print(PGMT(M_INPUT));
            Serial.print(PGMT(M_DEBUG_COMMAND));
            Serial.println(commandBuffer);
        }
    
        // Expect three characters, command, nodeId, pinId
        if (commandLen == 3)
        {
            node = charToHex(commandBuffer[1]);
            pin  = charToHex(commandBuffer[2]);
    
            switch (commandBuffer[0] | 0x20)            // Command character converted to lower-case.
            {
                case 'i': if (   (node < INPUT_NODE_MAX)
                              && (pin  < INPUT_PIN_MAX))
                          {
                              controller.processInput(node, pin, false);
                              executed = true;
                          }
                          break;
    
                case 'o': state = outputCtl.getOutputState(node, pin);    // Use current state - new state will be the opposite of this.
                          [[fallthrough]];
                case 'l': state = !state;                                 // Use opposite of default state, or opposite of current state if fallthrough.
                          [[fallthrough]];
                case 'h': if (   (node < OUTPUT_NODE_MAX)                 // Use default state, or state as dictated by fallthrough cases.
                              && (pin  < OUTPUT_PIN_MAX))
                          {
                              controller.processOutput(node, pin, state, 0);
                              executed = true;
                          }
                          break;
    
                default:  break;
            }
        }
    
        // Report error if not executed.
        if (!executed)
        {
            if (isDebug(DEBUG_ERRORS))
            {
                Serial.print(PGMT(M_UNKNOWN));
                Serial.print(PGMT(M_DEBUG_COMMAND));
                Serial.println(commandBuffer);
            }
    
            if (systemMgr.isReportEnabled(REPORT_SHORT))
            {
                disp.clearRow(LCD_COL_START, LCD_ROW_BOT);
                disp.setCursor(LCD_COL_START, LCD_ROW_BOT);
                disp.printProgStr(M_UNKNOWN);
                disp.printCh(CHAR_SPACE);
                disp.printStr(commandBuffer);
                controller.setDisplayTimeout(systemMgr.getReportDelay());
            }
        }
    }
};


#endif
