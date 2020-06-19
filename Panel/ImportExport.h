/** ImpoertExport.
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
  char lastChar;
  char wordBuffer[WORD_BUFFER_LENGTH + 1];
  

  /** Import a line.
   */
  void importLine()
  {
    lcd.clearRow(LCD_COL_START, LCD_ROW_BOT);
    if (!strcmp_P(wordBuffer, M_SYSTEM))
    {
      lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_SYSTEM);
    }
    else if (!strcmp_P(wordBuffer, M_IMPORT_INPUT))
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
    node = (node >> INPUT_NODE_SHIFT) & INPUT_NODE_MASK;
    loadInput(node, pin);
  
    readWord();
    if (!strcmp_P(wordBuffer, M_TOGGLE))
    {
      mask = INPUT_TOGGLE_MASK;
    }
    else if (!strcmp_P(wordBuffer, M_BUTTON))
    {
      mask = 0;
    }
    else
    {
      importError();
    }
  
    for (int i = 0; i < INPUT_OUTPUT_MAX; i++)
    {
      inputData.output[i] = readData();
      inputData.output[i] = (((inputData.output[i] >> 4) & OUTPUT_NODE_MASK) << OUTPUT_NODE_SHIFT) | (inputData.output[i] & OUTPUT_PIN_MASK);
      if (   (i > 0)
          && (wordBuffer[strlen(wordBuffer) - 1] == CHAR_STAR))
      {
        inputData.output[i] |= INPUT_DISABLED_MASK;    
      }
    }
  
    inputData.output[0] |= mask;
  
    saveInput();
  }
  
  
  /** Import an output
   */
  void importOutput()
  {
    int node = 0;
    int pin  = 0;
    
    node = readData();
    pin  = node & OUTPUT_PIN_MASK;
    node = (node >> 4) & OUTPUT_NODE_MASK;
    loadOutput(node, pin);
    
    readWord();
    if (!strcmp_P(wordBuffer, M_IMPORT_SERVO))
    {
      outputData.mode = (outputData.mode & ~OUTPUT_NODE_MASK) | OUTPUT_MODE_SERVO;
    }
    else if (!strcmp_P(wordBuffer, M_IMPORT_SIGNAL))
    {
      outputData.mode = (outputData.mode & ~OUTPUT_NODE_MASK) | OUTPUT_MODE_SIGNAL;
    }
    else if (!strcmp_P(wordBuffer, M_IMPORT_LED))
    {
      outputData.mode = (outputData.mode & ~OUTPUT_NODE_MASK) | OUTPUT_MODE_LED;
    }
    else
    {
      importError();
    }
  
    outputData.lo   = readData();
    outputData.hi   = readData();
    outputData.pace = readData();
  
    saveOutput();
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
    if (aChar > '9')
    {
      return (aChar - 'a' + 10) & 0xf;
    }
    else
    {
      return (aChar - '0'     ) & 0xf;
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
      lastChar = Serial.read();
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
    Serial.println();
    Serial.println();
  
    // dumpMemory();
  }
  
  
//  /** Dump all the EEPROM memory.
//   */
//  void dumpMemory()
//  {
//    dumpMemory(SYSTEM_BASE, SYSTEM_END);
//    Serial.println();
//    dumpMemory(OUTPUT_BASE, OUTPUT_END);
//    Serial.println();
//    dumpMemory(INPUT_BASE,  INPUT_END);
//    Serial.println();
//  }
//  
//  
//  /** Dump a range of the EEPROM memory.
//   */
//  void dumpMemory(int aStart, int aEnd)
//  {
//    for (int base = aStart; base < aEnd; base += 16)
//    {
//      printHex(base, 4);
//      Serial.print(":");
//      
//      for (int offs = 0; offs < 16; offs++)
//      {
//        Serial.print(CHAR_SPACE);
//        printHex(EEPROM.read(base + offs), 2);
//      }
//  
//      Serial.println();
//    }
//  }
  
  
  void exportInputs()
  {
    Serial.println(PGMT(M_EXPORT_INPUT));
  
    for (int node = 0; node < INPUT_NODE_MAX; node++)
    {
      if (isInputNode(node))
      {
        for (int pin = 0; pin < INPUT_NODE_SIZE; pin++)
        {
          loadInput(node, pin);
  
          Serial.print(PGMT(M_INPUT));
          Serial.print(CHAR_TAB);
          printHex(node, 1);
          Serial.print(CHAR_TAB);
          printHex(pin, 1);
          Serial.print(CHAR_TAB);
          Serial.print(PGMT(M_INPUT_TYPES[(inputData.output[0] & INPUT_TOGGLE_MASK ? 1 : 0)]));
          
          for (int output = 0; output < INPUT_OUTPUT_MAX; output++)
          {
            Serial.print(CHAR_TAB);
            printHex((inputData.output[output] >> OUTPUT_NODE_SHIFT) & OUTPUT_NODE_MASK, 1);
            Serial.print(CHAR_SPACE);
            printHex((inputData.output[output]                     ) & OUTPUT_PIN_MASK,  1);
            if (   (output > 0)
                && (inputData.output[output] & INPUT_DISABLED_MASK))
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
        for (int pin = 0; pin < OUTPUT_NODE_SIZE; pin++)
        {
          loadOutput(node, pin);
  
          Serial.print(PGMT(M_OUTPUT));
          Serial.print(CHAR_TAB);
          printHex(node, 1);
          Serial.print(CHAR_TAB);
          printHex(pin, 1);
          Serial.print(CHAR_TAB);
          Serial.print(PGMT(M_OUTPUT_TYPES[outputData.mode & OUTPUT_MODE_MASK]));
          Serial.print(CHAR_TAB);
          printHex(outputData.lo, 2);
          Serial.print(CHAR_TAB);
          printHex(outputData.hi, 2);
          Serial.print(CHAR_TAB);
          printHex((outputData.pace >> OUTPUT_PACE_SHIFT) & OUTPUT_PACE_MASK,  1);
          Serial.print(CHAR_TAB);
          printHex((outputData.pace                     ) & OUTPUT_DELAY_MASK, 1);
          Serial.println();
        }
        Serial.println();
      }
    }
  }
  
  
  /** Print a number as a string of hex digits.
   *  Padded with leading zeros to length aDigits.
   */
  void printHex(int aValue, int aDigits)
  {
    for (int digit = aDigits - 1; digit >= 0; digit--)
    {
      Serial.print(HEX_CHARS[(aValue >> (digit << 2)) & 0xf]);
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
    lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_WAITING);
  
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
      default:          systemFail(M_EXPORT, aExport);
    }
  
    lcd.clearRow(LCD_COL_EXP_STATUS, LCD_ROW_BOT);
  }
  

};


/** A singleton instance of the class.
 */
ImportExport importExport;;


#endif