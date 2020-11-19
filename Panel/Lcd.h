/** LCD extension of LiquidCrystal
 *  
 * The circuit:
 * LCD RS pin to digital pin 12m
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
 * 
 */
#ifndef _LCD_h
#define _LCD_h

// include the library code:
#include <LiquidCrystal.h>


#define LCD_COLS             16   // LCD is 16 columns
#define LCD_ROWS              2   // by 2 rows.

#define LCD_ROW_TOP           0   // Rows for LCD state messages.
#define LCD_ROW_BOT           1

#define LCD_LEN_OPTION        6   // Command menu options are (padded to) this length.

#define LCD_COL_START         0   // Cols for LCD state messages.
#define LCD_COL_MARK          6   // Marker when changing top-level option.
#define LCD_COL_VERSION       8   // Where to put version number/date.
#define LCD_COL_CALIBRATE     6   // Button to press.
#define LCD_COL_STATE         8   // State of an output.
#define LCD_COL_NODE         11   // Node number.
#define LCD_COL_PIN          14   // Pin number.
// #define LCD_COL_STATE        14   // State of output.

#define LCD_COL_I2C_PARAM     7   // I2C parameters(3) start at this col.
#define LCD_COL_I2C_STEP      3   // Three columns per parameter.

#define LCD_COL_DEBUG_PARAM   8   // Debug parameter.
#define LCD_COL_DEBUG_LENGTH  5   // Debug parameter length.

#define LCD_COL_EXP_STATUS    7   // Export status.

#define LCD_COL_INPUT_OUTPUT  8   // Input's outputs(3) start at this col.
#define LCD_COL_INPUT_STEP    3   // Three columns per output.

#define LCD_COL_OUTPUT_PARAM  7   // Output's parameters(3) start at this col.
#define LCD_COL_OUTPUT_STEP   3   // Three columns per parameter.
#define LCD_COL_OUTPUT_LO     8   // Output's lo parameter at this col.
#define LCD_COL_OUTPUT_HI    12   // Output's hi parameter at this col.
#define LCD_COL_OUTPUT_PACE   8   // Output's pace parameter at this col.
#define LCD_COL_OUTPUT_DELAY 13   // Output's delay parameter at this col.


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
     *  Pad with spaces to aSize.
     */
    void print_P(PGM_P messagePtr, int aSize)
    {
//        Serial.print("p_P ");
//        Serial.print(PGMT(messagePtr));
//        Serial.print(" [");
//        Serial.print(aSize);
//        Serial.println("]");

        print(PGMT(messagePtr));
        int padding = aSize - strlen_P(messagePtr);
        while (padding-- > 0)
        {
            print(CHAR_SPACE);
        }
    }


    /** Print a message at a particular location.
     *  Pad with spaces to aSize.
     */
    void printAt(int col, int row, PGM_P messagePtr, int aSize)
    {
        setCursor(col, row);
        print_P(messagePtr, aSize);
    }


    /** Print a message at a particular location.
     *  No padding.
     */
    void printAt(int col, int row, PGM_P messagePtr)
    {
        printAt(col, row, messagePtr, 0);
    }


    /** Print a character at a particular location.
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


    /** Print a number as a string of dec digits.
     *  Padded with leading spaces to length aDigits.
     */
    void printAtDec(int col, int row, int aValue, int aDigits)
    {
        int value = aValue;
        int divisor = 1;
        boolean leadingBlanks = true;
        
        setCursor(col, row);

        // Calculate starting power of 10 for desired number of digits.
        for (int digit = 1; digit < aDigits; digit++)
        {
            divisor *= 10;
        }

        // Output the digits in sequence.
        for (int digit = aDigits - 1; digit >= 0; digit--)
        {
            if (value / divisor > 0)
            {
                print((char) ('0' + (value / divisor)));
                leadingBlanks = false;
            }
            else if (   (digit > 0)
                     && (leadingBlanks))
            {
                print(CHAR_SPACE);
            }
            else
            {
                print('0');
            }

            // Next digit.
            value = value % divisor;
            divisor /= 10;
        }
    }


    /** Clear a row from the given column to the end.
     */
    void clearRow(int aCol, int aRow)
    {
//        Serial.print("clr ");
//        Serial.print(aCol);
//        Serial.print(",");
//        Serial.println(aRow);
        
        int spaces = 0;
        setCursor(aCol, aRow);
        for (spaces = 0; spaces < LCD_COLS - aCol; spaces++)
        {
            print(CHAR_SPACE);
        }
    }

};


/** A singleton instance of the class.
 *  Initialize the LCD library with the numbers of the interface pins
 *  Typical options:
 *  LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
 *  LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
 */
LCD lcd(8, 9, 4, 5, 6, 7);


#endif
