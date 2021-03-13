/** Display extension of LiquidCrystal
 *
 *
 *  (c)Copyright Tony Clulow  2021    tony.clulow@pentadtech.com
 *
 *  This work is licensed under the:
 *      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 *      http://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 *  For commercial use, please contact the original copyright holder(s) to agree licensing terms
 */
#ifndef _Display_h
#define _Display_h

#include <LiquidCrystal.h>
// #include <LiquidCrystal_I2C.h>


#define LCD_COLS             16   // Display is 16 columns
#define LCD_ROWS              2   // by 2 rows.

#define LCD_ROW_TOP           0   // Rows for Display state messages.
#define LCD_ROW_BOT           1

#define LCD_LEN_OPTION        6   // Command menu options are (padded to) this length.

#define LCD_COL_START         0   // Cols for Display state messages.
#define LCD_COL_MARK          6   // Marker column when changing top-level option.
#define LCD_COL_CALIBRATE     6   // Button to press.
#define LCD_COL_STATE         7   // State of an output.
#define LCD_COL_NODE         11   // Node number.
#define LCD_COL_PIN          14   // Pin number.

#define LCD_COL_I2C_PARAM     7   // I2C parameters(3) start at this col.
#define LCD_COL_I2C_STEP      3   // Three columns per parameter.

#define LCD_COL_REPORT_PARAM  8   // Report parameter.
#define LCD_COL_REPORT_LENGTH 5   // Report parameter length.

#define LCD_COL_DEBUG_PARAM   8   // Debug parameter.
#define LCD_COL_DEBUG_LENGTH  6   // Debug parameter length.

#define LCD_COL_EXP_STATUS    7   // Export status.

#define LCD_COL_INPUT_OUTPUT  8   // Input's outputs start at this col.

#define LCD_COL_OUTPUT_PARAM  7   // Output's parameters(3) start at this col.
#define LCD_COL_OUTPUT_STEP   3   // Three columns per parameter.
#define LCD_COL_OUTPUT_LO     8   // Output's lo parameter at this col.
#define LCD_COL_OUTPUT_HI    12   // Output's hi parameter at this col.
#define LCD_COL_OUTPUT_PACE   8   // Output's pace parameter at this col.
#define LCD_COL_OUTPUT_RESET 12   // Output's reset parameter at this col.

#define LCD_COL_LOCK_MARK     2   // Marker column for lock options.
#define LCD_COL_LOCK_SELECT   4   // Lock selection (A,B,C,D) at this column.
#define LCD_COL_LOCK_STATE    7   // Lock state at this column.



/** A Display class that can print PROGMEM messages.
 */
class Display
{
      
    private:

    /** An LCD attached as a shield.
     *  Typical options:
     *  
     *  LiquidCrystal(rs,     enable,                 d4, d5, d6, d7)
     *  LiquidCrystal(rs, rw, enable,                 d4, d5, d6, d7)
     *  LiquidCrystal(rs,     enable, d0, d1, d2, d3, d4, d5, d6, d7)
     *  LiquidCrystal(rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7) 
     *  
     *  LiquidCrystal lcdShield(8, 9, 4, 5, 6, 7);
     *  LiquidCrystal lcdShield(12, 11, 5, 4, 3, 2);
     */ 
    LiquidCrystal lcdShield = LiquidCrystal(8, 9, 4, 5, 6, 7);
//    LiquidCrystal_I2C* lcd2 = new LiquidCrystal_I2C(0x27, 16, 2);


    
    public:
    
    /** Constructor.
     */
    Display()
    {
        lcdShield.begin(LCD_COLS, LCD_ROWS);
        
        // Custom character to indicate "Lo".
        lcdShield.createChar(CHAR_LO, BYTES_LO);      
        for (uint8_t index = 0; index < CHAR_LO; index++)
        {
            lcdShield.createChar(index, LOGO[index]);
        }
        Serial.println("Constructor()");
    }


    /** Clears the display.
     *  Delegate to library class.
     */
    void clear()
    {
        lcdShield.clear();
    }


    /** Sets the cursor location.
     *  Delegate to library class.
     */
    void setCursor(int aCol, int aRow)
    {
        lcdShield.setCursor(aCol, aRow);
    }
    

    /** Prints a character on the LCD.
     *  Delegate to library class.
     */
    void printCh(char aChar)
    {
        lcdShield.print(aChar);
    }


    /** Print a character at a particular location.
     */
    void printChAt(uint8_t col, uint8_t row, char aChar)
    {
        setCursor(col, row);
        printCh(aChar);
    }


    /** Prints a char string on the LCD.
     *  Delegate to library class.
     */
    void printStr(char* aString)
    {
//        Serial.print("print(\"");
//        Serial.print(aString);
//        Serial.println("\")");
        lcdShield.print(aString);
    }


    /** Prints a Flash string on the LCD.
     *  Delegate to library class.
     */
    void printProgStr(PGM_P aMessagePtr)
    {
//        Serial.print("printProgStr(\"");
//        Serial.print(PGMT(aMessagePtr));
//        Serial.println("\")");
        lcdShield.print(PGMT(aMessagePtr));
    }


    /** Print a PROGMEM message to the LCD.
     *  Pad with spaces to aSize.
     */
    void printProgStr(PGM_P aMessagePtr, uint8_t aSize)
    {
        int8_t padding = aSize - strlen_P(aMessagePtr);
        printProgStr(aMessagePtr);
        while (padding-- > 0)
        {
            printCh(CHAR_SPACE);
        }
    }


    /** Print a PROGMEM message at a particular location.
     *  Pad with spaces to aSize.
     */
    void printProgStrAt(uint8_t col, uint8_t row, PGM_P aMessagePtr, uint8_t aSize)
    {
        setCursor(col, row);
        printProgStr(aMessagePtr, aSize);
    }


    /** Print a PROGMEM message at a particular location.
     *  No padding.
     */
    void printProgStrAt(uint8_t col, uint8_t row, PGM_P aMessagePtr)
    {
        printProgStrAt(col, row, aMessagePtr, 0);
    }


    /** Print a number as 2 hex digits.
     */
    void printHexByte(uint8_t aValue)
    {
        printHexCh(aValue >> 4);
        printHexCh(aValue);
    }


    /** Print a number as a string of hex digits at the specified location.
     *  Padded with leading zeros to length aDigits.
     */
    void printHexByteAt(uint8_t col, uint8_t row, uint8_t aValue)
    {
        setCursor(col, row);
        printHexByte(aValue);
    }


    /** Print a HEX character.
     */
    void printHexCh(uint8_t aHexValue)
    {
        printCh(HEX_CHARS[aHexValue & 0x0f]);
    }


    /** Print a HEX character.
     */
    void printHexChAt(uint8_t col, uint8_t row, uint8_t aHexValue)
    {
        setCursor(col, row);
        printHexCh(aHexValue);
    }


    /** Print a number as a string of dec digits at the specified location.
     *  Padded with leading spaces to length aDigits.
     */
    void printDecAt(uint8_t col, uint8_t row, int aValue, uint8_t aDigits)
    {
        setCursor(col, row);
        printDec(aValue, aDigits, CHAR_SPACE);
    }


    /** Print a number as a string of dec digits.
     *  Padded with leading spaces to length aDigits.
     */
    void printDec(int aValue, int aDigits, char aPad)
    {
        int value   = aValue;
        int digits  = 1;
        int divisor = 1;
        boolean leadingBlanks = true;

        // Calculate min digits required
        while (value >= 10)
        {
            value  /= 10;
            digits += 1;
        }
        value = aValue;
        
        // Use at least the min digits requested.
        if (digits < aDigits)
        {
            digits = aDigits;
        }
        
        // Calculate starting power of 10 for desired number of digits.
        for (int digit = 1; digit < digits; digit++)
        {
            divisor *= 10;
        }

        // Output the digits in sequence.
        while (--digits >= 0)
        {
            if (value / divisor > 0)
            {
                printCh((char) (CHAR_ZERO + (value / divisor)));
                leadingBlanks = false;
            }
            else if (   (digits > 0)            // Don't pad the last digit
                     && (leadingBlanks))        // Don't pad after we've had some non-zero digits.
            {
                printCh(aPad);
            }
            else
            {
                printCh(CHAR_ZERO);
            }

            // Next digit.
            value = value % divisor;
            divisor /= 10;
        }
    }


    /** Clear a row from the given column to the end.
     */
    void clearRow(uint8_t aCol, uint8_t aRow)
    {
        setCursor(aCol, aRow);
        for (uint8_t spaces = 0; spaces < LCD_COLS - aCol; spaces++)
        {
            printCh(CHAR_SPACE);
        }
    }

};


/** A singleton instance of the class.
 */
Display disp; 


#endif
