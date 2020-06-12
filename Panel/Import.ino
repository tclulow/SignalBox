/** Import data.
 */

#define WORD_BUFFER_LENGTH  32

char lastChar;
char wordBuffer[WORD_BUFFER_LENGTH + 1];


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


void importLine()
{
  lcd.clearRow(LCD_COL_START, LCD_ROW_BOT);
  if (!strcmp_P(wordBuffer, M_SYSTEM))
  {
    lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_SYSTEM);
  }
  else if (!strcmp_P(wordBuffer, M_INPUT2))
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
    // Report unrecognised import line.
    lcd.setCursor(LCD_COL_START, LCD_ROW_BOT);
    lcd.print(wordBuffer);
    lcd.print("?");

    // Wait for user-input. BUTTON_RIGHT will continue, others will abort import.
    if (waitForButton() == BUTTON_RIGHT)
    {
      lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_WAITING);
      waitForButtonRelease();
    }
    else
    {
      lcd.clearRow(LCD_COL_START, LCD_ROW_BOT);
    }
  }

  // Skip rest of line.
  while (!endOfLine())
  {
    readWord();
  }
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
  if (!strcmp_P(wordBuffer, M_SERVO))
  {
    outputData.mode = OUTPUT_MODE_SERVO;
  }
  else if (!strcmp_P(wordBuffer, M_SIGNAL))
  {
    outputData.mode = OUTPUT_MODE_SIGNAL;
  }
  else if (!strcmp_P(wordBuffer, M_LED))
  {
    outputData.mode = OUTPUT_MODE_LED;
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


/** Skip whole line
 */
void skipLine()
{
  while ((lastChar = Serial.read()) != CHAR_NEWLINE)
  {
    if (readButton())
    {
      return;
    }
  }
  Serial.println("Skip line");
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
  Serial.print("Read ");
  Serial.println(wordBuffer);
  
  return index; 
}
