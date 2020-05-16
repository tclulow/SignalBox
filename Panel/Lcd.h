/**
 */
#ifndef _LCD_H
#define _LCD_H
 
// include the library code:
#include <LiquidCrystal.h>


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
  uint8_t print_P(PGM_P messagePtr)
  {
    char b;
    uint8_t chars = 0;
    
    while (b = pgm_read_byte(messagePtr++))
    {
      chars++;
      print(b);
    }

    return chars;
  }


  /** Print a message at a particular location.
   *  Return the length of the message printed.
   */
  uint8_t printAt(int col, int row, PGM_P messagePtr)
  {
    setCursor(col, row);
    return print_P(messagePtr);
  }


  /** Print an int at a particular location.
   *  Use the specified number of characters with leading spaces.
   */
  uint8_t printAt(int col, int row, int aNumber, uint8_t aLength)
  {
    int value = aNumber;
    char buffer[aLength + 1];
    int offset = aLength;
    
    buffer[offset] = '\0';
    
    while(--offset >= 0)
    {
      if (value >= 0)
      {
        buffer[offset] = '0' + (char)(value % 10);
        value = value / 10;
        if (value == 0)
        {
          value = -1;
        }
      }
      else
      {
        buffer[offset] = ' ';
      }
    }
    
    return printAt(col, row, buffer);
  }
};

#endif
