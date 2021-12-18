/** Display extension of LiquidCrystal
 *  @file
 *
 *
 *  (c)Copyright Tony Clulow  2021    tony.clulow@pentadtech.com
 *
 *  This work is licensed under the:
 *      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 *      http://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 *  For commercial use, please contact the original copyright holder(s) to agree licensing terms.
 */

#ifndef Display_h
#define Display_h


#include <LiquidCrystal.h>
#if LCD_I2C
#include <LiquidCrystal_I2C.h>
#endif


#define LCD_COLS             16   // Display is 16 columns
#define LCD_ROWS              2   // by 2 rows.
#define LCD_ROW_MASK          1   // Row mask for shield LCD (1 = 2 row, 3 = 4 rows)

#define LCD2_COLS            20   // I2C Display is 20 columns
#define LCD2_ROWS             4   // by 4 rows.
#define LCD2_ROW_MASK         3   // Row mask for I2C LCD (1 = 2 row, 3 = 4 rows)


#define LCD_ROW_TOP           0   // Rows for Display state messages.
#define LCD_ROW_DET           1   // Detail goes on second row.
#define LCD_ROW_EDT           2   // Edit on row 2 (if available).
#define LCD_ROW_BOT           3   // Bottom row.

#define LCD_LEN_OPTION        6   // Command menu options are (padded to) this length.
#define LCD_LEN_STATUS        8   // Status messages take up to 8 characters.

#define LCD_COL_START         0   // Cols for Display state messages.
#define LCD_COL_MARK          6   // Marker column when changing top-level option.
#define LCD_COL_CALIBRATE     6   // Button to press.
#define LCD_COL_STATE         7   // State of an output.
#define LCD_COL_NODE         11   // Node number.
#define LCD_COL_PIN          14   // Pin number.

#define LCD_COL_REPORT_PARAM  8   // Report parameter.
#define LCD_COL_REPORT_LENGTH 5   // Report parameter length.

#define LCD_COL_DEBUG_PARAM   8   // Debug parameter.
#define LCD_COL_DEBUG_LENGTH  6   // Debug parameter length.

#define LCD_COL_INPUT_OUTPUT  8   // Input's outputs start at this col.

#define LCD_COL_OUTPUT_PARAM  7   // Output's parameters(3) start at this col.
#define LCD_COL_OUTPUT_STEP   3   // Three columns per parameter.
#define LCD_COL_OUTPUT_LO     7   // Output's lo parameter at this col.
#define LCD_COL_OUTPUT_HI    12   // Output's hi parameter at this col.
#define LCD_COL_OUTPUT_PACE   8   // Output's pace parameter at this col.
#define LCD_COL_OUTPUT_RESET 12   // Output's reset parameter at this col.

#define LCD_COL_LOCK_MARK     2   // Marker column for lock options.
#define LCD_COL_LOCK_SELECT   4   // Lock selection (A,B,C,D) at this column.
#define LCD_COL_LOCK_STATE    7   // Lock state at this column.


/** A Display class that can print PROGMEM messages.
 *  Can display of LCD shield (LCD_ROWS x LCD_COLS) and/or an I2C LCD display (LCD2_ROWS x LCD2_COLS).
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
     *  LiquidCrystal lcdShield( 8,  9, 4, 5, 6, 7);
     *  LiquidCrystal lcdShield(12, 11, 5, 4, 3, 2);
     */
    LiquidCrystal* lcdShield;

#if LCD_I2C
    LiquidCrystal_I2C* lcdI2C;      // An LCD attached using I2C.
#else
    LiquidCrystal*     lcdI2C;      // Dummy variable, never set.
#endif
    uint8_t            lcdId = 0;   // The ID of the I2C LCD. Never set if no I2C LCD.


    public:

    /** Constructor.
     */
    Display()
    {
    }


    /** Create the LCD shield.
     */
    void createLcdShield()
    {
        lcdShield = new LiquidCrystal(LCD_RS, LCD_ENABLE, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
        lcdShield->begin(LCD_COLS, LCD_ROWS);
        // lcdShield->noCursor();
        // lcdShield->noBlink();
        lcdShield->createChar(CHAR_LO, BYTES_LO);       // Custom character to indicate "Lo".
    }


#if LCD_I2C
    /** Creates the I2C LCD object and sets its I2C ID.
     */
    void setLcd(uint8_t aLcdId)
    {
        lcdId = aLcdId;
        lcdI2C = new LiquidCrystal_I2C(lcdId, LCD2_COLS, LCD2_ROWS);
        lcdI2C->begin(LCD2_COLS, LCD2_ROWS);
        lcdI2C->noCursor();                             // Cursor and Blink should be off by default,
        lcdI2C->noBlink();                              // but sometimes appear on, so force them off.
        lcdI2C->backlight();
        lcdI2C->createChar(CHAR_LO, BYTES_LO);          // Custom character to indicate "Lo".
    }
#endif


    /** Gets the I2C LCD's ID.
     *  If no I2C LCD connected (or code is disabled with LCD_I2C false),
     *  then this is never set.
     */
    uint8_t getLcdId()
    {
        return lcdId;
    }


    /** Clears the display.
     *  Delegate to library classes.
     */
    void clear()
    {
        if (lcdShield)
        {
            lcdShield->clear();
        }

        if (lcdI2C)
        {
            lcdI2C->clear();
        }
    }


    /** Sets the cursor location.
     *  Negative aCol measures from right-hand-end of display.
     *  Delegate to library classes.
     */
    void setCursor(int aCol, int aRow)
    {
        if (aCol < 0)
        {
            // Position relative to the end of the row.
            if (lcdShield)
            {
                lcdShield->setCursor(aCol + LCD_COLS, aRow & LCD_ROW_MASK);
            }
            if (lcdI2C)
            {
                lcdI2C->setCursor(aCol + LCD2_COLS, aRow & LCD2_ROW_MASK);
            }
        }
        else
        {
            // Position relative to the start of the row.
            if (lcdShield)
            {
                lcdShield->setCursor(aCol, aRow & LCD_ROW_MASK);
            }
            if (lcdI2C)
            {
                lcdI2C->setCursor(aCol, aRow & LCD2_ROW_MASK);
            }
        }
    }


    /** Prints a character on the LCD.
     *  Delegate to library classes.
     */
    void printCh(char aChar)
    {
        if (lcdShield)
        {
            lcdShield->print(aChar);
        }
        if (lcdI2C)
        {
            lcdI2C->print(aChar);
        }
    }


    /** Print a character at a particular location.
     */
    void printChAt(int aCol, uint8_t aRow, char aChar)
    {
        setCursor(aCol, aRow);
        printCh(aChar);
    }


    /** Prints a char string on the LCD.
     *  Delegate to library classes.
     */
    void printStr(char* aString)
    {
        if (lcdShield)
        {
            lcdShield->print(aString);
        }
        if (lcdI2C)
        {
            lcdI2C->print(aString);
        }
    }


    /** Prints a Flash string on the LCD.
     *  Delegate to library classes.
     */
    void printProgStr(PGM_P aMessagePtr)
    {
        if (lcdShield)
        {
            lcdShield->print(PGMT(aMessagePtr));
        }
        if (lcdI2C)
        {
            lcdI2C->print(PGMT(aMessagePtr));
        }
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
    void printProgStrAt(int aCol, uint8_t aRow, PGM_P aMessagePtr, uint8_t aSize)
    {
        setCursor(aCol, aRow);
        printProgStr(aMessagePtr, aSize);
    }


    /** Print a PROGMEM message at a particular location.
     *  No padding.
     */
    void printProgStrAt(int aCol, uint8_t aRow, PGM_P aMessagePtr)
    {
        printProgStrAt(aCol, aRow, aMessagePtr, 0);
    }


    /** Print a number as 2 hex digits.
     */
    void printHexByte(uint8_t aValue)
    {
        printHexCh((aValue >> 4) & 0x0f);
        printHexCh(aValue & 0x0f);
    }


    /** Print a number as 2 hex digits at the specified location.
     */
    void printHexByteAt(int aCol, uint8_t aRow, uint8_t aValue)
    {
        setCursor(aCol, aRow);
        printHexByte(aValue);
    }


    /** Print a HEX character.
     *  Note that this can print 'hex' characters 'G' to 'V' also.
     */
    void printHexCh(uint8_t aHexValue)
    {
        printCh(HEX_CHARS[aHexValue & 0x1f]);
    }


    /** Print a HEX character. at the specified location.
     *  Note that this can print 'hex' characters 'G' to 'V' also.
     */
    void printHexChAt(int aCol, uint8_t aRow, uint8_t aHexValue)
    {
        setCursor(aCol, aRow);
        printHexCh(aHexValue);
    }


    /** Print a number as a string of dec digits at the specified location.
     *  Padded with leading spaces to length aDigits.
     */
    void printDecAt(int aCol, uint8_t aRow, int aValue, uint8_t aDigits)
    {
        setCursor(aCol, aRow);
        printDec(aValue, aDigits, CHAR_SPACE);
    }


    /** Print a number as a string of dec digits.
     *  Padded with leading aPad characters to length aDigits.
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
     *  negative column measures from the right-hand-end of the display.
     */
    void clearRow(int aCol, uint8_t aRow)
    {
        uint8_t len = LCD_COLS - aCol;
        if (aCol < 0)
        {
            len = -aCol;
        }

        setCursor(aCol, aRow);
        for (uint8_t spaces = 0; spaces < len; spaces++)
        {
            printCh(CHAR_SPACE);
        }

        // Add extra spaces for lcdI2C if necessary.
        if (   (lcdI2C)
            && (aCol >= 0))
        {
            for (uint8_t spaces = LCD_COLS; spaces < LCD2_COLS; spaces++)
            {
                lcdI2C->print(CHAR_SPACE);
            }
        }
    }


    /** Clear bottom two rows.
     *  On 2-row screen, that's also the top 2 rows.
     */
    void clearBottomRows()
    {
        clearRow(LCD_COL_START, LCD_ROW_EDT);
        clearRow(LCD_COL_START, LCD_ROW_BOT);
    }
};


/** A singleton instance of the Display class.
 */
Display disp;


#endif
