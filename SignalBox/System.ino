/** System data.
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

#include "All.h"


/** Load SystemData from EEPROM.
 *  Return true if valid, else don't load and return false.
 */
boolean loadSystemData()
{
    EEPROM.get(SYSTEM_BASE, systemData.magic);  // Check the magic number

    if (systemData.magic == MAGIC_NUMBER)
    {
        EEPROM.get(SYSTEM_BASE, systemData);
    }

    return systemData.magic == MAGIC_NUMBER;
}


/** Save SystemData.
 */
void saveSystemData()
{
    EEPROM.put(SYSTEM_BASE, systemData);

    debugSystemData();
}


/** Print system data if debug so enabled.
 */
void debugSystemData()
{
    if (isDebug(DEBUG_BRIEF))
    {
        Serial.print(millis());
        Serial.print(CHAR_TAB);
        Serial.print(PGMT(M_DEBUG_SYSTEM));
        Serial.print(CHAR_SPACE);
        Serial.print(PGMT(M_DEBUG_DEBUG));
        Serial.print(CHAR_SPACE);
        Serial.print(systemData.debugLevel,      HEX);
        Serial.print(CHAR_SPACE);
        Serial.print(PGMT(M_DEBUG_REPORT));
        Serial.print(CHAR_SPACE);
        Serial.print(systemData.reportLevel,     HEX);
        Serial.println();

        if (isDebug(DEBUG_FULL))
        {
            dumpMemory();
        }
    }
}


/** Is debugging enabled at this level?
 */
boolean isDebug(uint8_t aLevel)
{
    return systemData.debugLevel >= aLevel;
}


/** Gets the debugging level.
 */
uint8_t getDebug()
{
    return systemData.debugLevel;
}


/** Sets the debugging level.
 */
void setDebug(uint8_t aLevel)
{
    systemData.debugLevel = aLevel;
}


/** Show version number by flashing LED
 *  and reporting it on Serial output.
 */
void flashVersion()
{
    pinMode(LED_BUILTIN, OUTPUT);       // Configure the on-board LED pin for output

    // Flash the digits of the version number string.
    for (uint8_t ind = 0; ind < strlen_P(M_VERSION); ind++)
    {
        char ch = pgm_read_byte_near(M_VERSION + ind);
        if (ch == CHAR_ZERO)
        {
            digitalWrite(LED_BUILTIN, HIGH);            // Long flash for zeros.
            delay(DELAY_BLINK_LONG);
            digitalWrite(LED_BUILTIN, LOW);
            delay(DELAY_BLINK);
        }
        else if (ch > CHAR_ZERO && ch <= CHAR_NINE)     // Short flashes for digits.
        {
            while (ch-- > CHAR_ZERO)
            {
                digitalWrite(LED_BUILTIN, HIGH);
                delay(DELAY_BLINK);
                digitalWrite(LED_BUILTIN, LOW);
                delay(DELAY_BLINK);
            }
        }
        else
        {
            delay(DELAY_BLINK_LONG);                    // Long gap for non-numeric data.
        }
    }

    if (isDebug(DEBUG_NONE))
    {
        Serial.print(PGMT(M_SOFTWARE));
        Serial.print(CHAR_SPACE);
        Serial.print(PGMT(M_VERSION));
        Serial.print(CHAR_SPACE);
        Serial.print(PGMT(M_VERSION_DATE));
        Serial.println();
    }
    
#if !MASTER
    // Flash module number.
    delay(DELAY_BLINK_LONG);
    for (uint8_t mask = 1; mask <= OUTPUT_NODE_MASK; mask <<= 1)
    {
        digitalWrite(LED_BUILTIN, HIGH);
        delay((getModuleId(false) & mask) ? DELAY_BLINK_LONG : DELAY_BLINK);
        digitalWrite(LED_BUILTIN, LOW);
        delay(DELAY_BLINK);
    }
#endif
}


#if MASTER

/** Report a system failure.
 */
void systemFail(PGM_P aMessage, int aValue)
{
    Serial.print(PGMT(M_FAILURE));
    Serial.print(CHAR_TAB);
    Serial.print(PGMT(aMessage));
    Serial.print(CHAR_SPACE);
    Serial.print(aValue, HEX);
    Serial.println();
    
    disp.clear();
    disp.printProgStrAt(LCD_COL_START, LCD_ROW_EDT, M_FAILURE);
    disp.printProgStrAt(LCD_COL_START, LCD_ROW_BOT, aMessage);
    disp.printHexByteAt(-2, LCD_ROW_BOT, aValue);

    setDisplayTimeout(DELAY_FAIL);
}


/** Is an EzyBus setup detected?
 */
boolean ezyBusDetected()
{
    return EEPROM.read(EZY_MAGIC_ADDR) == EZY_MAGIC;    // Check for tell-tale value in EEPROM
}


/** Make sure Ezybus won't recognise the (Panel) setup.
 */
void ezyBusClear()
{
    if (ezyBusDetected())
    {
        EEPROM.put(EZY_MAGIC_ADDR, EZY_MAGIC + 1);      // Corrupt the EzyBus magic number slightly.
    }
}


#else // Not master - Slave output module.


// The hardware module ID - read from jumperPins.
uint8_t jumperModuleId = 0;

/** Read the hardware jumper pins and record the setting at startup.
 */
void readJumperPins()
{
    for (uint8_t pin = 0, mask=1; pin < JUMPER_PINS; pin++, mask <<= 1)
    {
        if (jumperPins[pin] <= ANALOG_PIN_LAST)
        {
            // Pins should be in INPUT_PULLUP state at startup.
            // pinMode(jumperPins[pin], INPUT_PULLUP);
            if (   (   (jumperPins[pin] >= ANALOG_PIN_FIRST)
                    && (analogRead(jumperPins[pin]) > ANALOG_PIN_CUTOFF))
                || (   (jumperPins[pin] <  ANALOG_PIN_FIRST)
                    && (digitalRead(jumperPins[pin]))))
            {
                jumperModuleId |= mask;
            }
        }
    }
}


/** Is the ID set by jumpers?
 */
boolean isJumperId()
{
    return systemData.i2cModuleID > OUTPUT_NODE_MAX;
}


/** Gets the output module ID.
 *  Either by hardware jumpers or from EEPROM.
 */
uint8_t getModuleId(boolean aIncludeBase)
{
    uint8_t moduleId = isJumperId() ? jumperModuleId : systemData.i2cModuleID;

    // Announce module ID
    if (   (isDebug(DEBUG_BRIEF))
        && (aIncludeBase))
    {
        Serial.print(millis());
        Serial.print(CHAR_TAB);
        Serial.print(PGMT(M_DEBUG_MODULE));
        Serial.print(CHAR_SPACE);
        Serial.print(I2C_OUTPUT_BASE_ID + moduleId, HEX);
        Serial.println();
    }

    return (aIncludeBase ? I2C_OUTPUT_BASE_ID : 0) + moduleId;
}


#endif


/** Convert a character to hex value.
 *  Special-case: letters G-V represent 0x10 to 0x1f.
 *  If any other characters, return a negative number -HEX_MAX
 */
int charToHex(char ch)
{
    int value = (int)(10 + ch);         // Assume alphabetic.
    
    if (ch >= CHAR_LOWER_A)
    {
        value -= CHAR_LOWER_A;          // Adjust for lower-case range.
    }
    else if (ch >= CHAR_UPPER_A)
    {
        value -= CHAR_UPPER_A;          // Adjust for upper-case range.
    }
    else if (   (ch >= CHAR_ZERO)
             && (ch <= CHAR_NINE))
    {
        value = (int)(ch - CHAR_ZERO);  // A Decimal digit.
    }
    else
    {
        value = -HEX_MAX;
    }
    
    if (value > 0x1f)
    {
        value = -HEX_MAX;
    }

    return value;
}


/** Print a number as a string of hex digits.
 *  Padded with leading zeros to length aDigits.
 */
void printHex(int aValue, uint8_t aDigits)
{
    for (int digit = aDigits - 1; digit >= 0; digit--)
    {
        Serial.print(HEX_CHARS[(aValue >> (digit << 2)) & 0xf]);
    }
}


/** Dump a range of the EEPROM memory.
 */
void dumpMemory(PGM_P aMessage, int aStart, int aEnd)
{
    Serial.print(CHAR_HASH);
    Serial.print(CHAR_SPACE);
    Serial.println(PGMT(aMessage));
    
    for (int base = aStart; base < aEnd; base += 16)
    {
        Serial.print(CHAR_HASH);
        Serial.print(CHAR_SPACE);
        printHex(base, 4);
        Serial.print(CHAR_COLON);
        
        for (int offs = 0; offs < 16; offs++)
        {
            Serial.print(CHAR_SPACE);
            printHex(EEPROM.read(base + offs), 2);
        }

        Serial.print(CHAR_SPACE);
        Serial.print(CHAR_SPACE);
        Serial.print(CHAR_SPACE);

        for (int offs = 0; offs < 16; offs++)
        {
            char ch = EEPROM.read(base + offs);
            if (   (ch >= CHAR_SPACE)
                && (ch <= CHAR_TILDE))
            {
                Serial.print(ch);    
            }
            else
            {
                Serial.print(CHAR_DOT);
            }
        }

        Serial.println();
    }
}


/** Dump all the EEPROM memory.
 */
void dumpMemory()
{
    dumpMemory(M_SYSTEM, SYSTEM_BASE, SYSTEM_END);
    Serial.println();
    
#if OUTPUT_BASE
    dumpMemory(M_OUTPUT, OUTPUT_BASE, OUTPUT_END);
    Serial.println();
#endif

#if TYPES_BASE
    dumpMemory(M_TYPES,  TYPES_BASE,  TYPES_END);
    Serial.println();
#endif

#if INPUT_BASE
    dumpMemory(M_INPUT,  INPUT_BASE,  INPUT_END);
    Serial.println();
#endif

}
