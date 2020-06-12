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
  else if (!strcmp_P(wordBuffer, M_INPUT))
  {
    lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_INPUT);
  }
  else if (!strcmp_P(wordBuffer, M_OUTPUT))
  {
    lcd.printAt(LCD_COL_START, LCD_ROW_BOT, M_OUTPUT);
  }
  else
  {
    lcd.setCursor(LCD_COL_START, LCD_ROW_BOT);
    lcd.print(wordBuffer);
    lcd.print("?");
  }

  Serial.print(wordBuffer);
  while (!endOfLine())
  {
    readWord();
    Serial.print(" ");
    Serial.print(wordBuffer);
  }
  Serial.println();
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
    // do nothing.
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
//  Serial.print("Read ");
//  Serial.println(wordBuffer);
  
  return index; 
}
