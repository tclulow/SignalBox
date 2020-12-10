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
#define EXP_MAX      4


/** An Importer/exporter.
 */
class ImportExport
{
    char lastChar;                              // Last character read.
    char wordBuffer[WORD_BUFFER_LENGTH + 1];    // Buffer to read characters
    long messageTick = 1L;                      // Time the last message was emitted.


    /** Import a line.
     */
    void importLine()
    {
        lcd.clearRow(LCD_COL_START, LCD_ROW_BOT);
        if (!strcmp_P(wordBuffer, M_SYSTEM))
        {
            lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_SYSTEM);
        }
        else if (!strcmp_P(wordBuffer, M_INPUT))
        {
            lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_INPUT);
            importInput();
        }
        else if (!strcmp_P(wordBuffer, M_OUTPUT))
        {
            lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_OUTPUT);
            importOutput();
        }
        else
        {
            importError();
        }
    
        // Skip rest of line.
        skipLine();
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
            for (int index = 0; index < INPUT_OUTPUT_MAX; index++)
            {
                uint8_t value = readData();
                inputDef.setOutputNode(index, (value >> 4) & OUTPUT_NODE_MASK);
                inputDef.setOutputPin(index,  (value     ) & OUTPUT_PIN_MASK);
                inputDef.setDisabled(index, wordBuffer[strlen(wordBuffer) - 1] == CHAR_STAR);
            }

            lcd.printAt(LCD_COL_NODE, LCD_ROW_BOT, HEX_CHARS[node]);
            lcd.printAt(LCD_COL_PIN , LCD_ROW_BOT, HEX_CHARS[pin]);
            saveInput();
        }
    }
    
    
    /** Import an output
     */
    void importOutput()
    {
        uint8_t type = 0;
        uint8_t pace = 0;
        
        outputNode = readData();
        outputPin  = outputNode & OUTPUT_PIN_MASK;
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
            outputDef.setLo(readData());
            outputDef.setHi(readData());
            pace = readData();
            outputDef.setPace(pace >> OUTPUT_PACE_SHIFT);
            outputDef.setDelay(pace & OUTPUT_DELAY_MASK);
        
            lcd.printAt(LCD_COL_NODE, LCD_ROW_BOT, HEX_CHARS[outputNode]);
            lcd.printAt(LCD_COL_PIN , LCD_ROW_BOT, HEX_CHARS[outputPin]);
            writeOutput(true);
        }
    }
    
    
    /** Read (hexadecimal) data.
     */
    int readData()
    {
        int value = 0;
        int len = readWord();
        if (len > 0)
        {
            value = hexValue(wordBuffer[0]) << 4;
            if (len == 1)
            {
                len = readWord();
                if (len > 0)
                {
                    value |= hexValue(wordBuffer[0]);
                }
            }
            else
            {
                value |= hexValue(wordBuffer[1]);
            }
        }
        return value;
    }
    

    /** Convert a character to it's HEX value.
     */
    int hexValue(char aChar)
    {
        if (aChar > CHAR_NINE)
        {
            return (aChar - CHAR_LOWER_A + 10) & 0xf;
        }
        else
        {
            return (aChar - CHAR_ZERO        ) & 0xf;
        }
    }
    
    
    /** Are we at end-of-line?
     */
    boolean endOfLine()
    {
        return lastChar == CHAR_NEWLINE;
    }
    
    
    /** Skip rest of line
     */
    void skipLine()
    {
        while (lastChar != CHAR_NEWLINE)
        {
            if (Serial.available())
            {
                lastChar = Serial.read();
            }
            if (readButton())
            {
                return;
            }
        }
    }
    
    
    /** Read a word from the Serial.
     */
    int readWord()
    {
        int index = 0;
    
        // Read upto WORD_BUFFER_LENGTH characters.
        for (index = 0; index < WORD_BUFFER_LENGTH; )
        {
            while ((lastChar = Serial.read()) < 0)
            {
                if (readButton())
                {
                    return 0;
                }
                delay(DELAY_BUTTON_WAIT);
    
                // Clear message if there's no activity.
                if (   (messageTick > 0)
                    && (messageTick < millis()))
                {
                    messageTick = 0;
                    lcd.clearRow(LCD_COL_START, LCD_ROW_BOT);
                    lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_WAITING);
                }
            }
    
            if (   (lastChar <  0)
                || (lastChar == CHAR_SPACE)
                || (lastChar == CHAR_TAB)
                || (lastChar == CHAR_NEWLINE)
                || (lastChar == CHAR_RETURN))
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
            lcd.print("?");
    
            // Wait for user-input. BUTTON_RIGHT will continue, others will abort import.
            if (waitForButton() == BUTTON_RIGHT)
            {
                lcd.clearRow(LCD_COL_START, LCD_ROW_BOT);
                lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_WAITING);
                waitForButtonRelease();
                skipLine();
            }
            else
            {
                lcd.clearRow(LCD_COL_START, LCD_ROW_BOT);
            }
    }

    /** Export the system parameters.
     */
    void exportSystem()
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
    
        dumpMemory();
    }
    
    
    void exportInputs()
    {
        Serial.println(PGMT(M_EXPORT_INPUT));
    
        for (int node = 0; node < INPUT_NODE_MAX; node++)
        {
            if (isInputNode(node))
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
                        printHex(inputDef.getOutputNode(index), 1);
                        Serial.print(CHAR_SPACE);
                        printHex(inputDef.getOutputPin(index),  1);
                        if (inputDef.isDisabled(index))
                        {
                            Serial.print(CHAR_STAR);
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
        Serial.println(PGMT(M_EXPORT_OUTPUT));
        
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
                    printHex(outputDef.getPace(),  1);
                    Serial.print(CHAR_TAB);
                    printHex(outputDef.getDelay(), 1);
                    Serial.println();
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
        waitForButtonRelease();
    
        // Clear the buffer
        while (Serial.available())
        {
            Serial.read();
        }

        // Keep going until until button pressed.
        while (!readButton())
        {
            int chars = readWord();
            if (chars > 0)
            {
                if (wordBuffer[0] == CHAR_HASH)
                {
                    skipLine();
                }
                else
                {
                    importLine();
                    messageTick = millis() + DELAY_READ;
                }
            }
        }
    
        lcd.clearRow(LCD_COL_START, LCD_ROW_BOT);
    }
    
    
    /** Export selected items.
     */
    void doExport(int aExport)
    {
        lcd.printAt(LCD_COL_EXP_STATUS, LCD_ROW_BOT, M_EXPORTING);
    
        switch(aExport)
        {
            case EXP_ALL:     exportSystem();
                              exportInputs();
                              exportOutputs();
                              break;
            case EXP_SYSTEM:  exportSystem();
                              break;
            case EXP_INPUTS:  exportInputs();
                              break;
            case EXP_OUTPUTS: exportOutputs();
                              break;
            default:          systemFail(M_EXPORT, aExport, 0);
        }
    
        lcd.clearRow(LCD_COL_EXP_STATUS, LCD_ROW_BOT);
    }
};


/** A singleton instance of the class.
 */
ImportExport importExport;


#endif
