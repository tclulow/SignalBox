/** ImportExport.
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

#ifndef ImportExport_h
#define ImportExport_h


// Import word buffer.
const uint8_t WORD_BUFFER_LENGTH = 32;

// Export menu states.
const uint8_t EXP_ALL            =  0;
const uint8_t EXP_SYSTEM         =  1;
const uint8_t EXP_INPUTS         =  2;
const uint8_t EXP_OUTPUTS        =  3;
const uint8_t EXP_LOCKS          =  4;
const uint8_t EXP_MAX            =  5;


/** An Importer/exporter.
 */
class ImportExport
{
    private:

    int  lastChar;                              // Last character read.
    char wordBuffer[WORD_BUFFER_LENGTH + 1];    // Buffer to read characters with null terminator on the end.
    unsigned long messageTick = 1L;             // Time the last message was emitted.


    public:

    /** An ImportExport object.
     */
    ImportExport()
    {
    }


    /** Import configuration from Serial line.
     */
    void doImport()
    {
        int len = 0;

        messageTick = 1;            // Ensure "waiting" message appears.
        buttons.waitForButtonRelease();

        // Clear the buffer
        while (Serial.available())
        {
            Serial.read();
        }

        // Keep going until until button pressed.
        while (   ((len = readWord()) >= 0)
               && (!buttons.readButton()))
        {
            if (   (len > 0)
                && (wordBuffer[0] != CHAR_HASH))
            {
                importLine();
                messageTick = millis() + DELAY_READ;
            }

            // Skip rest of line.
            skipLine();
        }
    }


    /** Export selected items.
     */
    void doExport(int aExport)
    {
        uint8_t debugLevel = systemMgr.getDebugLevel();

        disp.printProgStrAt(-strlen_P(M_EXPORTING), LCD_ROW_DET, M_EXPORTING);
        systemMgr.setDebugLevel(DEBUG_NONE);

        switch(aExport)
        {
            case EXP_ALL:     exportSystem(debugLevel);
                              exportInputs(true);
                              exportOutputs();
                              exportLocks();
                              break;

            case EXP_SYSTEM:  exportSystem(debugLevel);
                              break;

            case EXP_INPUTS:  exportInputs(false);
                              break;

            case EXP_OUTPUTS: exportOutputs();
                              break;

            case EXP_LOCKS:   exportLocks();
                              break;

            default:          systemFail(M_EXPORT, aExport);
                              break;
        }

        disp.clearRow(-strlen_P(M_EXPORTING), LCD_ROW_DET);
        systemMgr.setDebugLevel(debugLevel);
    }


    private:

    /** Import a line.
     */
    void importLine()
    {
        if (!strcmp_P(wordBuffer, M_SYSTEM))
        {
            importSystem();
        }
        else if (!strcmp_P(wordBuffer, M_INPUT))
        {
            importInput();
        }
        else if (!strcmp_P(wordBuffer, M_OUTPUT))
        {
            importOutput();
        }
        else if (!strcmp_P(wordBuffer, M_LOCK))
        {
            importLock();
        }
        else
        {
            importError();
        }
    }


    /** Import the system def.
     *  Nothing to do but report the fact.
     */
    void importSystem()
    {
        disp.printProgStrAt(LCD_COLS - LCD_LEN_OPTION, LCD_ROW_TOP, M_SYSTEM, LCD_LEN_OPTION);
        disp.setCursor(LCD_COL_START, LCD_ROW_DET);
        while (readWord() > 0)
        {
            disp.printStr(wordBuffer);
            disp.printCh(CHAR_SPACE);
        }
        // disp.clearRow(LCD_COL_START, LCD_ROW_DET);
    }


    /** Import an Input
     */
    void importInput()
    {
        int node = 0;
        int pin  = 0;

        node = readData() & INPUT_NODE_MASK;
        pin  = readData() & INPUT_PIN_MASK;
        inputMgr.loadInput(node, pin);

        // Read the Input's type.
        readWord();
        for (inputType = 0; inputType < INPUT_TYPE_MAX; inputType++)
        {
            if (!strcmp_P(wordBuffer, M_INPUT_TYPES[inputType]))
            {
                break;
            }
        }

        if (inputType >= INPUT_TYPE_MAX)
        {
            importError();
        }
        else
        {
            // Read all the Input's Outputs.
            for (int index = 0; index < INPUT_OUTPUT_MAX; index++)
            {
                int outNode = readData();
                int outPin  = readData();
                boolean outDelay = outNode < 0;
                
                if (outDelay)
                {
                    outNode = node;
                }
                if (outPin < 0)
                {
                    outPin = 0;
                }
                
                inputDef.setOutputNode(index, outNode & OUTPUT_NODE_MASK);
                inputDef.setOutputPin(index, outPin & OUTPUT_PIN_MASK);
                inputDef.setDelay(index, outDelay);
            }

            disp.printProgStrAt(LCD_COLS - LCD_LEN_OPTION, LCD_ROW_TOP, M_INPUT, LCD_LEN_OPTION);
            disp.printProgStrAt(LCD_COL_START, LCD_ROW_DET, M_INPUT_TYPES[inputType], LCD_LEN_STATUS);
            disp.printHexChAt(LCD_COL_NODE, LCD_ROW_DET, node);
            disp.printHexChAt(LCD_COL_PIN , LCD_ROW_DET, pin);
            inputMgr.saveInput();
        }
    }


    /** Import an output
     */
    void importOutput()
    {
        uint8_t type  = 0;
        int     value = 0;

        outputNode = readData() & OUTPUT_NODE_MASK;
        outputPin  = readData() & OUTPUT_PIN_MASK;

        // Read the Output's type.
        readWord();
        for (type = 0; type < OUTPUT_TYPE_MAX; type++)
        {
            if (!strcmp_P(wordBuffer, M_OUTPUT_TYPES[type]))
            {
                break;
            }
        }

        if (type >= OUTPUT_TYPE_MAX)
        {
            importError();
        }
        else
        {
            // Read the Output's definition.
            outputDef.setType(type);
            outputDef.setState(outputCtl.getOutputState(outputNode, outputPin));
            outputDef.setLo(readData());
            outputDef.setHi(readData());

            // Read (optional) Pace.
            value = readData();
            if (value >= 0)
            {
                outputDef.setPace(value);
            }
            else
            {
                outputDef.setPace(OUTPUT_DEFAULT_PACE);
            }

            // Read (optional) reset.
            value = readData();
            if (value >= 0)
            {
                outputDef.setReset(value);
            }
            else
            {
                outputDef.setReset(OUTPUT_DEFAULT_RESET);
            }

            disp.printProgStrAt(LCD_COLS - LCD_LEN_OPTION, LCD_ROW_TOP, M_OUTPUT, LCD_LEN_OPTION);
            disp.printProgStrAt(LCD_COL_START, LCD_ROW_DET, M_OUTPUT_TYPES[type], LCD_LEN_STATUS);
            disp.printHexChAt(LCD_COL_NODE,  LCD_ROW_DET, outputNode);
            disp.printHexChAt(LCD_COL_PIN ,  LCD_ROW_DET, outputPin);

            outputCtl.writeOutput();
            outputCtl.writeSaveOutput();
        }
    }


    /** Import a lock
     */
    void importLock()
    {
        int value = 0;

        outputNode = readData() & OUTPUT_NODE_MASK;
        outputPin  = readData() & OUTPUT_PIN_MASK;

        disp.printProgStrAt(LCD_COLS - LCD_LEN_OPTION, LCD_ROW_TOP, M_LOCK, LCD_LEN_OPTION);
        disp.printHexChAt(LCD_COL_NODE,  LCD_ROW_DET, outputNode);
        disp.printHexChAt(LCD_COL_PIN ,  LCD_ROW_DET, outputPin);

        if (outputCtl.isOutputNodePresent(outputNode))
        {
            disp.printProgStrAt(LCD_COL_START, LCD_ROW_DET, M_OUTPUT_TYPES[outputDef.getType()], LCD_LEN_STATUS);
            
            // Fetch the Output's definition to update its locks.
            outputCtl.readOutput(outputNode, outputPin);

            // Process the Lo and Hi lock definitions.
            for (uint8_t hi = 0; hi < 2; hi++)
            {
                // Process the locks.
                for (uint8_t index = 0; index < OUTPUT_LOCK_MAX; index++)
                {
                    if (   (readWord() <= 0)
                        || (wordBuffer[0] == CHAR_DOT))
                    {
                        outputDef.setLock(hi, index, false);
                    }
                    else
                    {
                        outputDef.setLockState(hi, index, !strcmp_P(wordBuffer, M_HI));
                        value = readData();
                        if (value < 0)
                        {
                            outputDef.setLock(hi, index, false);
                        }
                        else
                        {
                            outputDef.setLock(hi, index, true);
                            outputDef.setLockNode(hi, index, value      & OUTPUT_NODE_MASK);
                            outputDef.setLockPin (hi, index, readData() & OUTPUT_PIN_MASK);
                        }
                    }
                }
            }

            outputCtl.writeOutput();
            outputCtl.writeSaveOutput();
        }
        else
        {
            disp.printProgStrAt(LCD_COL_START, LCD_ROW_DET, M_NONE, LCD_LEN_STATUS);
        }
    }


    /** Read a character into lastChar when available.
     *  Abandon reading if a button is pressed.
     *  Output Waiting message if we wait a long time.
     */
    int readChar()
    {
        while (   (!buttons.readButton())
               && (!Serial.available()))
        {
            delay(DELAY_BUTTON_WAIT);

            // Clear message if there's no activity.
            if (   (messageTick > 0)
                && (messageTick < millis()))
            {
                messageTick = 0;
                disp.clearRow(LCD_COLS - LCD_LEN_OPTION, LCD_ROW_TOP);
                disp.clearRow(LCD_COL_START, LCD_ROW_DET);
                disp.printProgStrAt(LCD_COL_START, LCD_ROW_DET, M_WAITING);
            }
        }

        return Serial.read();
    }


    /** Read (hexadecimal) data.
     *  Special-case: letters G-V represent 0x10 to 0x1f.
     *  If dots are present, return a negative number -HEX_MAX
     */
    int readData()
    {
        int  hex   = 0;
        int  value = 0;
        int  len   = readWord();

        if (len <= 0)
        {
            value = -HEX_MAX;                   // Nothing to read.
        }
        else
        {
            for (uint8_t index = 0; index < len; index++)
            {
                hex = charToHex(wordBuffer[index]);
                if (hex < 0)
                {
                    value = -HEX_MAX;
                    break;
                }
                else
                {
                    value <<= 4;
                    value += hex;
                }
            }
        }

        return value;
    }


    /** Are we at end-of-line?
     */
    bool isEndOfLine()
    {
        return    lastChar == CHAR_NEWLINE
               || lastChar == CHAR_RETURN;
    }


    /** Is latest character white-space?
     */
    bool isWhiteSpace()
    {
        return    (lastChar == CHAR_SPACE)
               || (lastChar == CHAR_TAB)
               || (isEndOfLine());
    }


    /** Skip rest of line
     */
    void skipLine()
    {
        while (   (!buttons.readButton())
               && (!isEndOfLine()))
        {
            lastChar = readChar();
        }

        lastChar = CHAR_SPACE;
    }


    /** Read a word from the Serial.
     *  Don't read beyond end-of-line
     */
    int readWord()
    {
        int index = 0;

        // Read upto WORD_BUFFER_LENGTH characters.
        while (   (!buttons.readButton())
               && (!isEndOfLine())
               && (index < WORD_BUFFER_LENGTH))
        {
            lastChar = readChar();

            if (isWhiteSpace())
            {
                if (index == 0)
                {
                    continue;
                }
                else
                {
                    break;
                }
            }

            wordBuffer[index++] = lastChar;
        }

        // Add terminator to word.
        wordBuffer[index] = CHAR_NULL;

        return index;
    }


    /** Report an import error.
     */
    void importError()
    {
        // Report unrecognised import line.
        disp.clearRow(LCD_COL_START, LCD_ROW_DET);
        disp.setCursor(LCD_COL_START, LCD_ROW_DET);
        disp.printStr(wordBuffer);
        disp.printCh(CHAR_QUERY);

        // Wait for user-input. BUTTON_RIGHT will continue, others will abort import.
        if (buttons.waitForButtonPress() == BUTTON_RIGHT)
        {
            disp.printProgStrAt(LCD_COL_START,  LCD_ROW_DET, M_WAITING, LCD_COLS);
            buttons.waitForButtonRelease();
        }
        else
        {
            disp.clearRow(LCD_COL_START, LCD_ROW_DET);
        }
    }


    /** Export the system parameters.
     */
    void exportSystem(uint8_t aDebugLevel)
    {
        // Export header comment.
        Serial.println(PGMT(M_EXPORT_SYSTEM));

        // Export system definition.
        Serial.print(PGMT(M_SYSTEM));
        Serial.print(CHAR_TAB);
        Serial.print(PGMT(M_VERSION));
        Serial.print(CHAR_TAB);
        Serial.print(PGMT(M_REPORT_PROMPTS[systemMgr.getReportLevel()]));
        Serial.print(CHAR_TAB);
        Serial.print(PGMT(M_DEBUG_PROMPTS[aDebugLevel]));
        Serial.println();
        Serial.println();

//        if (aDebugLevel >= DEBUG_FULL)
//        {
//            dumpMemory();
//        }
    }


    /** Export the Inputs.
     *  Only export connected inputs, unless aAll is set.
     */
    void exportInputs(bool aAll)
    {
        // Export header comment
        Serial.print(PGMT(M_EXPORT_INPUT));
        for (uint8_t index = 0; index < INPUT_OUTPUT_MAX; index++)
        {
            Serial.print(PGMT(M_EXPORT_INPUT_OUT));
            Serial.print(OPTION_ID(index));
        }
        Serial.println();

        // Export all the inputs
        for (int node = 0; node < INPUT_NODE_MAX; node++)
        {
            if (   (aAll)
                || (isInputNodePresent(node)))
            {
                for (int pin = 0; pin < INPUT_PIN_MAX; pin++)
                {
                    // Export Input defintion
                    inputMgr.loadInput(node, pin);

                    Serial.print(PGMT(M_INPUT));
                    Serial.print(CHAR_TAB);
                    Serial.print(HEX_CHARS[node]);
                    Serial.print(CHAR_TAB);
                    Serial.print(HEX_CHARS[pin]);
                    Serial.print(CHAR_TAB);
                    Serial.print(PGMT(M_INPUT_TYPES[inputType]));

                    // Export Input's Outputs.
                    for (int index = 0; index < INPUT_OUTPUT_MAX; index++)
                    {
                        Serial.print(CHAR_TAB);
                        if (inputDef.isDelay(index))
                        {
                            Serial.print(CHAR_DOT);
                            Serial.print(CHAR_SPACE);
                            if (inputDef.getOutputPin(index) == 0)
                            {
                                Serial.print(CHAR_DOT);
                            }
                            else
                            {
                                Serial.print(HEX_CHARS[inputDef.getOutputPin(index)]);
                            }
                        }
                        else
                        {
                            Serial.print(HEX_CHARS[inputDef.getOutputNode(index)]);
                            Serial.print(CHAR_SPACE);
                            Serial.print(HEX_CHARS[inputDef.getOutputPin(index)]);
                        }
                    }
                    Serial.println();
                }
                Serial.println();
            }
        }
    }


    /** Export the Outputs.
     */
    void exportOutputs()
    {
        // Export header comment.
        Serial.print(PGMT(M_EXPORT_OUTPUT));
        Serial.println();

        // Export all the Outputs.
        for (int node = 0; node < OUTPUT_NODE_MAX; node++)
        {
            if (outputCtl.isOutputNodePresent(node))
            {
                for (int pin = 0; pin < OUTPUT_PIN_MAX; pin++)
                {
                    // Export Output definition.
                    outputCtl.readOutput(node, pin);

                    Serial.print(PGMT(M_OUTPUT));
                    Serial.print(CHAR_TAB);
                    Serial.print(HEX_CHARS[node]);
                    Serial.print(CHAR_TAB);
                    Serial.print(HEX_CHARS[pin]);
                    Serial.print(CHAR_TAB);
                    Serial.print(PGMT(M_OUTPUT_TYPES[outputDef.getType()]));
                    Serial.print(CHAR_TAB);
                    printHex(outputDef.getLo(),    2);
                    Serial.print(CHAR_TAB);
                    printHex(outputDef.getHi(),    2);
                    Serial.print(CHAR_TAB);
                    printHex(outputDef.getPace(),  2);
                    Serial.print(CHAR_TAB);
                    printHex(outputDef.getReset(), 2);
                    Serial.println();
                }
                Serial.println();
            }
        }
    }


    /** Export the defined locks.
     */
    void exportLocks()
    {
        // Export header comment.
        Serial.print(PGMT(M_EXPORT_LOCKS));

        // Export the Lo and Hi lock header comments.
        for (uint8_t hi = 0; hi < 2; hi++)
        {
            for (uint8_t index = 0; index < OUTPUT_LOCK_MAX; index++)
            {
                Serial.print(PGMT(M_EXPORT_LOCK));
                Serial.print(PGMT(hi ? M_HI : M_LO));
                Serial.print(OPTION_ID(index));
            }
        }
        Serial.println();

        // Export all the locks.
        for (int node = 0; node < OUTPUT_NODE_MAX; node++)
        {
            if (outputCtl.isOutputNodePresent(node))
            {
                for (int pin = 0; pin < OUTPUT_PIN_MAX; pin++)
                {
                    // Export a lock definition.
                    outputCtl.readOutput(node, pin);

                    Serial.print(PGMT(M_LOCK));
                    Serial.print(CHAR_TAB);
                    Serial.print(HEX_CHARS[node]);
                    Serial.print(CHAR_TAB);
                    Serial.print(HEX_CHARS[pin]);

                    // Export locks, Lo and Hi
                    for (uint8_t hi = 0; hi < 2; hi++)
                    {
                        for (uint8_t index = 0; index < OUTPUT_LOCK_MAX; index++)
                        {
                            // Export a lock.
                            Serial.print(CHAR_TAB);
                            if (outputDef.isLock(hi, index))
                            {
                                Serial.print(PGMT(outputDef.getLockState(hi, index) ? M_HI : M_LO));
                                Serial.print(CHAR_SPACE);
                                Serial.print(HEX_CHARS[outputDef.getLockNode(hi, index)]);
                                Serial.print(CHAR_SPACE);
                                Serial.print(HEX_CHARS[outputDef.getLockPin (hi, index)]);
                            }
                            else
                            {
                                Serial.print(CHAR_DOT);
                            }
                        }
                    }
                    Serial.println();
                }
                Serial.println();
            }
        }
    }
};


/** A singleton instance of the class.
 */
ImportExport importExport;


#endif
