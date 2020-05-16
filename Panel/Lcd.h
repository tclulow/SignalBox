/**
 */
 
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
   */
  void print_P(PGM_P messagePtr)
  {
    char b;
    while (b = pgm_read_byte(messagePtr++))
    {
      print(b);
    }
  }


  /** Print a message at a particular location.
   */
  void printAt(int col, int row, PGM_P messagePtr)
  {
    setCursor(col, row);
    print_P(messagePtr);
  }
};
