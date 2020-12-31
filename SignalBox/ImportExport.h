/** ImportExport.
 */
#ifndef _ImportExport_h
#define _ImportExport_h

// Import word buffer.
#define WORD_BUFFER_LENGTH  32

// Export menu states.
#define EXP_ALL      0
#define EXP_SYSTEM   1
#define EXP_INPUTS   2
#define EXP_OUTPUTS  3
#define EXP_LOCKS    4
#define EXP_MAX      5
 

/** An Importer/exporter.
 */
class ImportExport
{
    int  lastChar;                              // Last character read.
    char wordBuffer[WORD_BUFFER_LENGTH + 1];    // Buffer to read characters with null terminator on the end.
    long messageTick = 1L;                      // Time the last message was emitted.


    /** Import a line.
     */
    void importLine()
    {
        lcd.clearRow(LCD_COL_START, LCD_ROW_BOT);
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
        lcd.printAt(LCD_COLS - LCD_LEN_OPTION, LCD_ROW_TOP, M_SYSTEM, LCD_LEN_OPTION);
    }


    /** Import an Input
     */
    void importInput()
    {
        int node = 0;
        int pin  = 0;
        int mask = 0;
        
        node = readData();
        pin  = node & INPUT_PIN_MASK;
        node = (node >> 4) & INPUT_NODE_MASK;       // Node number is in upper nibble
        loadInput(node, pin);
    
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
                int value = readData();
                if (value >= 0)
                {
                    // Active Output
                    inputDef.setOutputNode(index, (value >> 4) & OUTPUT_NODE_MASK);
                    inputDef.setOutputPin(index,  (value     ) & OUTPUT_PIN_MASK);
                    inputDef.setDelay(index, false);
                }
                else
                {
                    // Delay
                    inputDef.setOutputNode(index, node);
                    inputDef.setOutputPin(index, value & OUTPUT_PIN_MASK);
                    inputDef.setDelay(index, true);
                }
            }

            lcd.printAt(LCD_COLS - LCD_LEN_OPTION, LCD_ROW_TOP, M_INPUT, LCD_LEN_OPTION);
            lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_INPUT_TYPES[inputType], LCD_LEN_OPTION);
            lcd.printAt(LCD_COL_NODE, LCD_ROW_BOT, HEX_CHARS[node]);
            lcd.printAt(LCD_COL_PIN , LCD_ROW_BOT, HEX_CHARS[pin]);
            saveInput();
        }
    }
    
    
    /** Import an output
     */
    void importOutput()
    {
        uint8_t type  = 0;
        int     value = 0;
        
        outputNode = readData();
        outputPin  = (outputNode     ) & OUTPUT_PIN_MASK;
        outputNode = (outputNode >> 4) & OUTPUT_NODE_MASK;
        
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
            outputDef.setType(type);
            outputDef.setState(getOutputState(outputNode, outputPin));
            outputDef.setLo(readData());
            outputDef.setHi(readData());
            value = readData();
            if (value >= 0)
            {
                outputDef.setPace(value);
            }
            else
            {
                outputDef.setPace(OUTPUT_DEFAULT_PACE);
            }
            value = readData();
            if (value >= 0)
            {
                outputDef.setReset(value);
            }
            else
            {
                outputDef.setReset(OUTPUT_DEFAULT_RESET);
            }

            lcd.printAt(LCD_COLS - LCD_LEN_OPTION, LCD_ROW_TOP, M_OUTPUT, LCD_LEN_OPTION);
            lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_OUTPUT_TYPES[type], LCD_LEN_OPTION);
            lcd.printAt(LCD_COL_NODE,  LCD_ROW_BOT, HEX_CHARS[outputNode]);
            lcd.printAt(LCD_COL_PIN ,  LCD_ROW_BOT, HEX_CHARS[outputPin]);
            
            writeOutput();
            writeSaveOutput();
        }
    }


    /** Import a lock
     */
    void importLock()
    {
        int value = 0;
        
        outputNode = readData();
        outputPin  = (outputNode     ) & OUTPUT_PIN_MASK;
        outputNode = (outputNode >> 4) & OUTPUT_NODE_MASK;
        if (isOutputNode(outputNode))
        {
            readOutput(outputNode, outputPin);
            for (uint8_t hi = 0; hi < 2; hi++)
            {
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
                            outputDef.setLockNode(hi, index, (value >> 4) & OUTPUT_NODE_MASK);
                            outputDef.setLockPin (hi, index, (value     ) & OUTPUT_PIN_MASK);
                        }
                    }
                }
            }
        
            lcd.printAt(LCD_COLS - LCD_LEN_OPTION, LCD_ROW_TOP, M_LOCK, LCD_LEN_OPTION);
            lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_OUTPUT_TYPES[outputDef.getType()], LCD_LEN_OPTION);
            lcd.printAt(LCD_COL_NODE,  LCD_ROW_BOT, HEX_CHARS[outputNode]);
            lcd.printAt(LCD_COL_PIN ,  LCD_ROW_BOT, HEX_CHARS[outputPin]);
            
            writeOutput();
            writeSaveOutput();
        }
    }


    /** Read a character into lastChar when available.
     *  Abandon reading if a button is pressed.
     *  Output Waiting message if we wait a long time.
     */
    int readChar()
    {
        while (   (!readButton())
               && (!Serial.available()))
        {
            delay(DELAY_BUTTON_WAIT);

            // Clear message if there's no activity.
            if (   (messageTick > 0)
                && (messageTick < millis()))
            {
                messageTick = 0;
                lcd.clearRow(LCD_COLS - LCD_LEN_OPTION, LCD_ROW_TOP);
                lcd.clearRow(LCD_COL_START, LCD_ROW_BOT);
                lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_WAITING);
            }
        }

        return Serial.read();
    }
    
    
    /** Read (hexadecimal) data.
     *  If dots are present, return a negative number relative to -HEX_LEN
     */
    int readData()
    {
        int value = 0;
        int len = readWord();
        if (len <= 0)
        {
            value = -HEX_MAX;                        // Delay of zero (ie disabled).
        }
        else
        {
            if (wordBuffer[0] == CHAR_DOT)      // Delay
            {
                value = -HEX_MAX;
            }
            else
            {
                value = hexValue(wordBuffer[0]) << 4;
            }
            
            if (len == 1)
            {
                len = readWord();
                if (len > 0)
                {
                    value += hexValue(wordBuffer[0]);
                }
            }
            else
            {
                value += hexValue(wordBuffer[1]);
            }
        }

        return value;
    }
    

    /** Convert a character to it's HEX value.
     *  Returns zero for illegal chars.
     */
    int hexValue(char aChar)
    {
        for (int value = HEX_MAX - 1; value >= 0; value--)
        {
            if (aChar == HEX_CHARS[value])
            {
                return value;
            }
        }

        return 0;
    }
    
    
    /** Are we at end-of-line?
     */
    boolean isEndOfLine()
    {
        return    lastChar == CHAR_NEWLINE
               || lastChar == CHAR_RETURN;
    }


    /** Is latest character white-space?
     */
    boolean isWhiteSpace()
    {
        return    (lastChar == CHAR_SPACE)
               || (lastChar == CHAR_TAB)
               || (isEndOfLine());
    }
    
    
    /** Skip rest of line
     */
    void skipLine()
    {
        while (   (!readButton())
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
        while (   (!readButton())
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
        lcd.clearRow(LCD_COL_START, LCD_ROW_BOT);
        lcd.setCursor(LCD_COL_START, LCD_ROW_BOT);
        lcd.print(wordBuffer);
        lcd.print(CHAR_QUERY);

        // Wait for user-input. BUTTON_RIGHT will continue, others will abort import.
        if (waitForButton() == BUTTON_RIGHT)
        {
            lcd.clearRow(LCD_COL_START, LCD_ROW_BOT);
            lcd.printAt(LCD_COL_START,  LCD_ROW_BOT, M_WAITING);
            waitForButtonRelease();
        }
        else
        {
            lcd.clearRow(LCD_COL_START, LCD_ROW_BOT);
        }
    }

    /** Export the system parameters.
     */
    void exportSystem(boolean aIncludeDump)
    {
        Serial.println(PGMT(M_EXPORT_SYSTEM));
        
        Serial.print(PGMT(M_SYSTEM));
        Serial.print(CHAR_TAB);
        Serial.print(PGMT(M_VERSION));
        Serial.print(CHAR_TAB);
        Serial.print(PGMT(M_SYS_I2C));
        Serial.print(CHAR_TAB);
        printHex(systemData.i2cControllerID, 2);
        Serial.print(CHAR_TAB);
        printHex(systemData.i2cInputBaseID,  2);
        Serial.print(CHAR_TAB);
        printHex(systemData.i2cOutputBaseID, 2);
        Serial.print(CHAR_TAB);
        // printHex(systemData.reportLevel,     2);
        Serial.print(PGMT(M_REPORT_PROMPTS[systemData.reportLevel]));
        Serial.println();
        Serial.println();

        if (aIncludeDump)
        {
            dumpMemory();
        }
    }
    

    /** Export the Inputs.
     *  Only export connected inputs unless aAll is set.
     */
    void exportInputs(boolean aAll)
    {
        Serial.print(PGMT(M_EXPORT_INPUT));
        for (uint8_t index = 0; index < INPUT_OUTPUT_MAX; index++)
        {
            Serial.print(PGMT(M_EXPORT_INPUT_OUT));
            Serial.print(OPTION_ID(index));
        }
        Serial.println();
    
        for (int node = 0; node < INPUT_NODE_MAX; node++)
        {
            if (   (aAll)
                || (isInputNode(node)))
            {
                for (int pin = 0; pin < INPUT_PIN_MAX; pin++)
                {
                    loadInput(node, pin);
    
                    Serial.print(PGMT(M_INPUT));
                    Serial.print(CHAR_TAB);
                    printHex(node, 1);
                    Serial.print(CHAR_TAB);
                    printHex(pin, 1);
                    Serial.print(CHAR_TAB);
                    Serial.print(PGMT(M_INPUT_TYPES[inputType]));
                    
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
                                printHex(inputDef.getOutputPin(index),  1);
                            }
                        }
                        else
                        {
                            printHex(inputDef.getOutputNode(index), 1);
                            Serial.print(CHAR_SPACE);
                            printHex(inputDef.getOutputPin(index),  1);
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
        Serial.print(PGMT(M_EXPORT_OUTPUT));
        Serial.println();
        
        for (int node = 0; node < OUTPUT_NODE_MAX; node++)
        {
            if (isOutputNode(node))
            {
                for (int pin = 0; pin < OUTPUT_PIN_MAX; pin++)
                {
                    readOutput(node, pin);
    
                    Serial.print(PGMT(M_OUTPUT));
                    Serial.print(CHAR_TAB);
                    printHex(node, 1);
                    Serial.print(CHAR_TAB);
                    printHex(pin, 1);
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
    void exportLocks(boolean aAll)
    {
        Serial.print(PGMT(M_EXPORT_LOCKS));
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
        
        for (int node = 0; node < OUTPUT_NODE_MAX; node++)
        {
            if (isOutputNode(node))
            {
                for (int pin = 0; pin < OUTPUT_PIN_MAX; pin++)
                {
                    readOutput(node, pin);

                    if (   (aAll)
                        || ((outputDef.getLockCount(false) + outputDef.getLockCount(true)) > 0))
                    {
                        Serial.print(PGMT(M_LOCK));
                        Serial.print(CHAR_TAB);
                        printHex(node, 1);
                        Serial.print(CHAR_TAB);
                        printHex(pin, 1);
    
                        // Output locks, Lo and Hi
                        for (uint8_t hi = 0; hi < 2; hi++)
                        {
                            for (uint8_t index = 0; index < OUTPUT_LOCK_MAX; index++)
                            {
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
                }
                Serial.println();
            }
        }
    }
    
    
    public:
    
    /** A ImportExport object.
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
        waitForButtonRelease();
    
        // Clear the buffer
        while (Serial.available())
        {
            Serial.read();
        }

        // Keep going until until button pressed.
        while (   ((len = readWord()) >= 0)
               && (!readButton()))
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
        uint8_t debugLevel = getDebug();

        lcd.printAt(LCD_COL_EXP_STATUS, LCD_ROW_BOT, M_EXPORTING);
        setDebug(DEBUG_NONE);
        
        switch(aExport)
        {
            case EXP_ALL:     exportSystem(debugLevel >= DEBUG_FULL);
                              exportInputs(true);
                              exportOutputs();
                              exportLocks(true);
                              break;
            case EXP_SYSTEM:  exportSystem(debugLevel >= DEBUG_FULL);
                              break;
            case EXP_INPUTS:  exportInputs(false);
                              break;
            case EXP_OUTPUTS: exportOutputs();
                              break;
            case EXP_LOCKS:   exportLocks(false);
                              break;
            default:          systemFail(M_EXPORT, aExport, 0);
        }
    
        lcd.clearRow(LCD_COL_EXP_STATUS, LCD_ROW_BOT);
        setDebug(debugLevel);
    }
};


/** A singleton instance of the class.
 */
ImportExport importExport;


#endif
