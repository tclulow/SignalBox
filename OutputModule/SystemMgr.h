/** System data.
 *  @file
 *
 *  (c)Copyright Tony Clulow  2021    tony.clulow@pentadtech.com
 *
 *  This work is licensed under the:
 *      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 *      http://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 *  For commercial use, please contact the original copyright holder(s) to agree licensing terms.
 */

#ifndef System_h
#define System_h


#if SB_CONTROLLER
    const long MAGIC_NUMBER = 0x786f6253;   // Magic number = "Sbox".
#elif SB_OUTPUT_MODULE
    const long MAGIC_NUMBER = 0x74756f53;   // Magic number = "Sout".
#endif

const long    VERSION       = 0x0463;       // Version number of software.  See also Messages.M_VERSION.


// Timing constants
const long    MILLIS_PER_SECOND = (1000L);                      // Millisecs in a second.
const long    MILLIS_PER_MINUTE = (MILLIS_PER_SECOND * 60L);    // Millisecs in a minute.
const long    MILLIS_PER_HOUR   = (MILLIS_PER_MINUTE * 60L);    // Millisecs per hour.


// Debug levels.
const uint8_t DEBUG_NONE        = 0;    // See also M_DEBUG_PROMPTS.
const uint8_t DEBUG_ERRORS      = 1;
const uint8_t DEBUG_BRIEF       = 2;
const uint8_t DEBUG_DETAIL      = 3;
const uint8_t DEBUG_FULL        = 4;
const uint8_t DEBUG_MAX         = 5;    // Maximum debug option.


// Reporting levels.
const uint8_t REPORT_NONE       = 0;    // See also M_REPORT_PROMPTS.
const uint8_t REPORT_SHORT      = 1;
const uint8_t REPORT_LONG       = 2;
const uint8_t REPORT_PAUSE      = 3;
const uint8_t REPORT_MAX        = 4;    // Maximum report option.


// Options identified by a character.
#define OPTION_ID(index) ((char)(CHAR_UPPER_A + index))
const uint8_t SYS_MODULE_ID_JUMPERS = 0xff;     // Use jumpers to decide module ID.


// Custom character to indicate "Lo".
byte BYTES_LO[] = { 0, 0, 0, 0, 0x11, 0xa, 0x4, 0 };


// Useful characters

const uint8_t CHAR_STX     = 0x02;
const uint8_t CHAR_ETX     = 0x03;
const uint8_t CHAR_DLE     = 0x10;
const uint8_t CHAR_SYN     = 0xff;

const char    CHAR_SPACE   = ' ';
const char    CHAR_TAB     = '\t';
const char    CHAR_NEWLINE = '\n';
const char    CHAR_RETURN  = '\r';
const char    CHAR_NULL    = 0;

const char    CHAR_HASH    = '#';
const char    CHAR_STAR    = '*';
const char    CHAR_DOT     = '.';
const char    CHAR_COMMA   = ',';
const char    CHAR_COLON   = ':';
const char    CHAR_LEFT    = '<';
const char    CHAR_RIGHT   = '>';
const char    CHAR_QUERY   = '?';
const char    CHAR_HI      = '^';
const char    CHAR_TILDE   = '~';
const char    CHAR_ZERO    = '0';
const char    CHAR_NINE    = '9';
const char    CHAR_UPPER_A = 'A';
const char    CHAR_LOWER_A = 'a';

const char    CHAR_LO      = 0;

// Hex characters - they are in fact base 32.
const char    HEX_CHARS[]  = "0123456789ABCDEFGHIJKLMNOPQRSTUV";
const uint8_t HEX_MAX      = sizeof(HEX_CHARS);


/** A System manager (extends Persisted) for persisting SystemData in EEPROM.
 */
class SystemMgr: public Persisted
{
    private:

    uint8_t jumperModuleId = 0;                                 // The hardware module ID - read from jumperPins.

    /** Data describing an Output's operation.
     */
    struct SystemData
    {
        public:

        long    magic           = MAGIC_NUMBER;                 // Magic number to identify software.
        long    version         = VERSION;                      // Software version number to identify upgrades.

        uint8_t rfu3[3];                                        // RFU. Was the I2C addresses.

        uint8_t i2cModuleId     = SYS_MODULE_ID_JUMPERS;        // The module number we're using - default, use hardware.

        uint8_t debugLevel      = DEBUG_ERRORS;                 // Debugging level.
        uint8_t reportLevel     = REPORT_LONG;                  // Reporting level.

        int     buttons[6];                                     // Configuration of analog buttons - BUTTON_HIGH + 1.

        char    rfu[6];                                         // RFU. 32 bytes in all.
    };

    SystemData systemData;      // Singleton instance of the SystemData


    public:

    /** A SystemMgr.
     */
    SystemMgr(uint16_t aBase) : Persisted(aBase)
    {
        size = sizeof(SystemData);
    }


    /** Initialise SystemMgr.
     *  Can only do this when system is started.
     */
    void init()
    {
#if SB_OUTPUT_MODULE

        // Read the Jumper pins.
        for (uint8_t pin = 0, mask=1; pin < JUMPER_PINS; pin++, mask <<= 1)
        {
            if (jumperPins[pin] <= ANALOG_PIN_LAST)
            {
                // Pins should be in INPUT_PULLUP state at startup.
                pinMode(jumperPins[pin], INPUT_PULLUP);
//                Serial.print("Pin ");
//                Serial.print(jumperPins[pin]);
//                if (jumperPins[pin] >= ANALOG_PIN_FIRST)
//                {
//                    Serial.print(", analog=");
//                    Serial.print(analogRead(jumperPins[pin]));
//                }
//                else
//                {
//                    Serial.print(", digital=");
//                    Serial.print(digitalRead(jumperPins[pin]));
//                }
//                Serial.println();
                if (   (   (jumperPins[pin] >= ANALOG_PIN_FIRST)
                        && (analogRead(jumperPins[pin]) > ANALOG_PIN_CUTOFF))
                    || (   (jumperPins[pin] <  ANALOG_PIN_FIRST)
                        && (digitalRead(jumperPins[pin]))))
                {
                    jumperModuleId |= mask;
                }
            }
        }
#endif

    }


    /** Load SystemData from EEPROM.
     *  Return true if valid, else don't load and return false.
     */
    bool loadSystemData()
    {
        EEPROM.get(getBase(), systemData.magic);  // Check the magic number

        if (systemData.magic == MAGIC_NUMBER)
        {
            EEPROM.get(getBase(), systemData);
//            Serial.print("ModuleID=");
//            Serial.print(systemData.i2cModuleId, HEX);
//            Serial.print(", jumperId=");
//            Serial.print(jumperModuleId, HEX);
//            Serial.println();
        }

        debugSystemData();
        return systemData.magic == MAGIC_NUMBER;
    }


    /** Save SystemData.
     */
    void saveSystemData()
    {
        systemData.magic = MAGIC_NUMBER;
        EEPROM.put(getBase(), systemData);

        // debugSystemData();
    }


    /** Gets the (recorded) version number.
     */
    long getVersion()
    {
        return systemData.version;
    }


    /** Is an update required?
     *  Version number has changed.
     */
    bool isUpdateRequired()
    {
        return systemData.version != VERSION;
    }


    /** Update performed.
     *  Bring the version number up to date.
     */
    void update()
    {
        systemData.version = VERSION;
        saveSystemData();
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

        // Always output software version info, even if debugging disabled.
        // if (isDebug(DEBUG_NONE))
        {
            Serial.print(PGMT(M_SOFTWARE));
            Serial.print(CHAR_SPACE);
            Serial.print(PGMT(M_VERSION));
            Serial.print(CHAR_SPACE);
            Serial.print(PGMT(M_VERSION_DATE));
            Serial.println();
        }

#if SB_OUTPUT_MODULE
        // Decide how many jumper pins to indicate
        uint8_t maskLimit = 0x1f;       // TODO - tie to Output max node number.
        if (isJumperId())
        {
            maskLimit >>= 1;            // Don't show software jumper pin
        }

        // Flash module number, short blink for set (one), long blink for unset (zero).
        delay(DELAY_BLINK_LONG);
        for (uint8_t mask = 1; mask <= maskLimit; mask <<= 1)
        {
            digitalWrite(LED_BUILTIN, HIGH);
            delay((getModuleId(false) & mask) ? DELAY_BLINK : DELAY_BLINK_LONG);
            digitalWrite(LED_BUILTIN, LOW);
            delay(DELAY_BLINK);
        }
        delay(DELAY_BLINK_LONG);
#endif

    }


#if SB_OUTPUT_MODULE
    /** Is the ID set by jumpers?
     */
    bool isJumperId()
    {
        return systemData.i2cModuleId == SYS_MODULE_ID_JUMPERS;
    }


    /** Set the I2C module Id.
     */
    void setModuleId(uint8_t aModuleId)
    {
        systemData.i2cModuleId = aModuleId;
    }


    /** Gets the output module ID.
     *  Either by hardware jumpers or from EEPROM.
     */
    uint8_t getModuleId(bool aIncludeBase)
    {
        uint8_t moduleId = isJumperId() ? jumperModuleId : systemData.i2cModuleId;

        // Announce module ID
        if (   (isDebug(DEBUG_BRIEF))
            && (aIncludeBase))
        {
            Serial.print(PGMT(M_DEBUG_MODULE));
            Serial.print(CHAR_SPACE);
            Serial.print(I2C_OUTPUT_BASE_ID + moduleId, HEX);
            Serial.println();
        }

        return (aIncludeBase ? I2C_OUTPUT_BASE_ID : 0) + moduleId;
    }
#endif


    /** Sets the debug level.
     */
    void setDebugLevel(uint8_t aLevel)
    {
        systemData.debugLevel = aLevel;
    }


    /** Gets the Debug level.
     */
    uint8_t getDebugLevel()
    {
        return systemData.debugLevel;
    }


#ifndef isDebug
    /** Is debugging enabled at this level?
     */
    bool isDebug(uint8_t aLevel)
    {
        return systemData.debugLevel >= aLevel;
    }
#endif


    /** Sets the report level.
     */
    void setReportLevel(uint8_t aLevel)
    {
        systemData.reportLevel = aLevel;
    }


    /** Gets the report level.
     */
    uint8_t getReportLevel()
    {
        return systemData.reportLevel;
    }


    /** Is reporting enabled (at a particular level)?
     */
    bool isReportEnabled(uint8_t aLevel)
    {
        return aLevel <= getReportLevel();
    }


    /** Length of time to wait for depending on the reporting level.
     */
    int getReportDelay()
    {
        return DELAY_READ * getReportLevel();
    }


    /** Gets (a pointer to) the button definitions.
     */
    int* getButtons()
    {
        return systemData.buttons;
    }


    /** Print system data if debug so enabled.
     */
    void debugSystemData()
    {
        if (isDebug(DEBUG_BRIEF))
        {
#if SB_OUTPUT_MODULE
            Serial.print(PGMT(M_DEBUG_MODULE));
            Serial.print(CHAR_SPACE);
            Serial.print(systemData.i2cModuleId, HEX);
            Serial.print(CHAR_SPACE);
#endif
            Serial.print(PGMT(M_DEBUG_DEBUG));
            Serial.print(CHAR_SPACE);
            Serial.print(systemData.debugLevel,  HEX);
            Serial.print(CHAR_SPACE);
            Serial.print(PGMT(M_DEBUG_REPORT));
            Serial.print(CHAR_SPACE);
            Serial.print(systemData.reportLevel, HEX);
            Serial.println();

//            if (isDebug(DEBUG_FULL))
//            {
//                dumpMemory();
//            }
        }
    }
};


/** Singleton SystemMgr at EEPROM offset 0. */
SystemMgr systemMgr(0);


/** TODO - revise debugging
 */
# ifndef isDebug
bool isDebug(uint8_t aLevel)
{
    return systemMgr.isDebug(aLevel);
}
#endif


// Global helper methods.

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


///** Dump a range of the EEPROM memory.
// */
//void dumpMemory(PGM_P aMessage, int aStart, int aEnd)
//{
//    Serial.print(CHAR_HASH);
//    Serial.print(CHAR_SPACE);
//    Serial.println(PGMT(aMessage));
//
//    for (int base = aStart; base < aEnd; base += 16)
//    {
//        Serial.print(CHAR_HASH);
//        Serial.print(CHAR_SPACE);
//        printHex(base, 4);
//        Serial.print(CHAR_COLON);
//
//        for (int offs = 0; offs < 16; offs++)
//        {
//            Serial.print(CHAR_SPACE);
//            printHex(EEPROM.read(base + offs), 2);
//        }
//
//        Serial.print(CHAR_SPACE);
//        Serial.print(CHAR_SPACE);
//        Serial.print(CHAR_SPACE);
//
//        for (int offs = 0; offs < 16; offs++)
//        {
//            char ch = EEPROM.read(base + offs);
//            if (   (ch >= CHAR_SPACE)
//                && (ch <= CHAR_TILDE))
//            {
//                Serial.print(ch);
//            }
//            else
//            {
//                Serial.print(CHAR_DOT);
//            }
//        }
//
//        Serial.println();
//    }
//}


///** Dump all the EEPROM memory.
// */
//void dumpMemory()
//{
//    dumpMemory(M_SYSTEM, SYSTEM_BASE, SYSTEM_END);
//    Serial.println();
//
//#if OUTPUT_BASE
//    dumpMemory(M_OUTPUT, OUTPUT_BASE, OUTPUT_END);
//    Serial.println();
//#endif
//
//#if TYPES_BASE
//    dumpMemory(M_TYPES,  TYPES_BASE,  TYPES_END);
//    Serial.println();
//#endif
//
//#if INPUT_BASE
//    dumpMemory(M_INPUT,  INPUT_BASE,  INPUT_END);
//    Serial.println();
//#endif


#endif
