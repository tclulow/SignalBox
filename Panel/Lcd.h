/** LCD extension of LiquidCrystal
 */
#ifndef _LCD_h
#define _LCD_h

// include the library code:
#include <LiquidCrystal.h>


#define LCD_COLS 16         // LCD is 16 columns
#define LCD_ROWS  2         // by 2 rows.

#define LCD_ROW_TOP     0   // Rows for LCD state messages.
#define LCD_ROW_BOT     1

#define LCD_COL_START         0   // Cols for LCD state messages.
#define LCD_COL_MARK          6   // Marker when changing top-level option.
#define LCD_COL_VERSION       8   // Where to put version number/date.
#define LCD_COL_NODE       11   // Node number.
#define LCD_COL_PIN          14   // Pin number.
// #define LCD_COL_STATE        14   // State of output.

#define LCD_COL_I2C_PARAM     7   // I2C parameters(3) start at this col.
#define LCD_COL_I2C_STEP      3   // Three columns per parameter.
#define LCD_COL_REP_STATUS    8   // Report status.
#define LCD_COL_INPUT_OUTPUT  8   // Input's outputs(3) start at this col.
#define LCD_COL_INPUT_STEP    3   // Three columns per output.
#define LCD_COL_OUTPUT_PARAM  7   // Output's parameters(3) start at this col.
#define LCD_COL_OUTPUT_STEP   3   // Three columns per parameter.


/** An LCD class that can print PROGMEM messages.
 */
class LCD: public LiquidCrystal
{
  public:
  LCD(uint8_t rs,  uint8_t enable, 
      uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3):
      LiquidCrystal(rs, enable, d0, d1, d2, d3)
  {
  }

  LCD(uint8_t rs, uint8_t rw, uint8_t enable,
      uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3):
      LiquidCrystal(rs, rw, enable, d0, d1, d2, d3)
  {
  }

  
  /** Print a PROGMEM message to the LCD.
   *  Return length of message printed.
   */
  void print_P(PGM_P messagePtr)
  {
    print(PGMT(messagePtr));
  }

  /** Print a message at a particular location.
   *  Return the length of the message printed.
   */
  void printAt(int col, int row, PGM_P messagePtr)
  {
    setCursor(col, row);
    print_P(messagePtr);
  }


  /** Print a character at a particular location.
   *  Return the length of the character printed (1).
   */
  void printAt(int col, int row, char aChar)
  {
    setCursor(col, row);
    print(aChar);
  }


  /** Print a number as a string of hex digits.
   *  Padded with leading zeros to length aDigits.
   */
  void printAtHex(int col, int row, int aValue, int aDigits)
  {
    setCursor(col, row);
    for (int digit = aDigits - 1; digit >= 0; digit--)
    {
      print(HEX_CHARS[(aValue >> (4 * digit)) & 0xf]);
    }
  }


  /** Clear a row from the given column to the end.
   *  Return the number of spaces output.
   */
  void clearRow(int aCol, int aRow)
  {
    int spaces = 0;
    setCursor(aCol, aRow);
    for (spaces = 0; spaces < LCD_COLS - aCol; spaces++)
    {
      print(CHAR_SPACE);
    }
  }


//  /** Print an int at a particular location.
//   *  Use the specified number of characters with leading spaces.
//   */
//  uint8_t printAt(int col, int row, int aNumber, uint8_t aLength)
//  {
//    int value = aNumber;
//    char buffer[aLength + 1];
//    int offset = aLength;
//    
//    buffer[offset] = '\0';
//    
//    while(--offset >= 0)
//    {
//      if (value >= 0)
//      {
//        buffer[offset] = '0' + (char)(value % 10);
//        value = value / 10;
//        if (value == 0)
//        {
//          value = -1;
//        }
//      }
//      else
//      {
//        buffer[offset] = ' ';
//      }
//    }
//    
//    return printAt(col, row, buffer);
//  }
};


/** A singleton instance of the class.
 *  Initialize the LCD library with the numbers of the interface pins
 *  Typical options:
 *  LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
 *  LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
 */
LCD lcd(8, 9, 4, 5, 6, 7);


#endif
