/** System data.
 *
 *
 *  (c)Copyright Tony Clulow  2021    tony.clulow@pentadtech.com
 *
 *  This work is licensed under the:
 *      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 *      http://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 *   For commercial use, please contact the original copyright holder(s) to agree licensing terms
 */
#ifndef _System_h
#define _System_h


#if MASTER
    #define MAGIC_NUMBER 0x786f6253         // Magic number = "Sbox".
#else
    #define MAGIC_NUMBER 0x74756f53         // Magic number = "Sout".
#endif

#define VERSION         0x0310              // Version number of software.  See also M_VERSION.


// i2c node numbers.
#define I2C_DEFAULT_CONTROLLER_ID   0x10    // Controller ID.
#define I2C_DEFAULT_INPUT_BASE_ID   0x20    // Input nodes' base ID.
#define I2C_DEFAULT_OUTPUT_BASE_ID  0x50    // Output nodes' base ID.
#define I2C_MODULE_ID_JUMPERS       0xff    // Use jumpers to decide module ID.


// Timing constants
#define MILLIS_PER_SECOND   (1000L)                     // Millisecs in a second.
#define MILLIS_PER_MINUTE   (MILLIS_PER_SECOND * 60L)   // Millisecs in a minute.
#define MILLIS_PER_HOUR     (MILLIS_PER_MINUTE * 60L)   // Millisecs per hour.


// Debug levels.
#define DEBUG_NONE      0       // See also M_DEBUG_PROMPTS.
#define DEBUG_ERRORS    1
#define DEBUG_BRIEF     2
#define DEBUG_DETAIL    3
#define DEBUG_FULL      4
#define DEBUG_MAX       5       // Maximum debug option.

// Reporting levels.
#define REPORT_OFF      0       // See also M_REPORT_PROMPTS.
#define REPORT_SHORT    1
#define REPORT_LONG     2
#define REPORT_PAUSE    3
#define REPORT_MAX      4       // Maximum report option.


// Options identified by a character.
#define OPTION_ID(index)  ((char)(CHAR_UPPER_A + index))


// System Data saved in EEPROM
#define SYSTEM_BASE  0                                                  // EEPROM base of System data.
#define SYSTEM_SIZE  32 //sizeof(systemData)                            // TODO - re-instate this  Size of System Data.
#define SYSTEM_END   (SYSTEM_BASE + SYSTEM_SIZE)                        // End of System EEPROM.


#if MASTER

    // Input types saved in EEPROM
    #define TYPES_BASE   SYSTEM_END                                     // EEPROM base of Input type data.
    #define TYPES_SIZE   4 // sizeof(uint32_t)                          // Size of Input types.
    #define TYPES_END    (TYPES_BASE + TYPES_SIZE * INPUT_NODE_MAX)     // End of Input Types EEPROM.

    // InputDef saved in EEPROM
    #define INPUT_BASE   TYPES_END                                      // EEPROM base of Input data.
    #define INPUT_SIZE   7 // sizeof(InputDef)                          // TODO - re-instate this  Size of InputData entry.
    #define INPUT_MAX    (INPUT_NODE_MAX * INPUT_PIN_MAX)               // Maximum inputs (16 nodes with 8 pins each).
    #define INPUT_END    (INPUT_BASE + INPUT_SIZE * INPUT_MAX)          // End of Input EEPROM.

    #define EEPROM_END   INPUT_END                                      // End of EEPROM memory

#else

    // OutputData saved in EEPROM
    #define OUTPUT_BASE  SYSTEM_END                                     // EEPROM base of OutputData.
    #define OUTPUT_SIZE  sizeof(OutputDef)                              // Size of OutputData entry.
    #define OUTPUT_END   (OUTPUT_BASE + OUTPUT_SIZE * OUTPUT_PIN_MAX)   // End of OutputData EEPROM.

    #define EEPROM_END   OUTPUT_END                                     // End of EEPROM memory

#endif


// Logo bit-map, up to seven characters (not const to avoid compiler warnings).
byte LOGO[][8] = { // { 0x00, 0x00, 0x0e, 0x1f, 0x0f, 0x0f, 0x0f, 0x04 },
                   { 0x1f, 0x09, 0x09, 0x0f, 0x0f, 0x0f, 0x0f, 0x04 },
                   { 0x00, 0x08, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x0e },
                   { 0x00, 0x0c, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x0e },
                   { 0x0c, 0x0c, 0x1e, 0x1e, 0x1e, 0x1e, 0x1f, 0x0a },
                 };
#define LOGO_LEN (sizeof(LOGO) / sizeof(LOGO[0]))

// Custom character to indicate "Lo".
byte BYTES_LO[]   = { 0, 0, 0, 0, 0x11, 0xa, 0x4, 0 };

// Useful characters
#define HEX_MAX  32
const char HEX_CHARS[]  = "0123456789ABCDEFGHIJKLMNOPQRSTUV";

const char CHAR_SPACE   = ' ';
const char CHAR_TAB     = '\t';
const char CHAR_NEWLINE = '\n';
const char CHAR_RETURN  = '\r';
const char CHAR_NULL    = 0;

const char CHAR_HASH    = '#';
const char CHAR_STAR    = '*';
//const char CHAR_COMMA   = ',';
const char CHAR_DASH    = '-';
const char CHAR_DOT     = '.';
const char CHAR_COLON   = ':';
const char CHAR_LEFT    = '<';
const char CHAR_RIGHT   = '>';
const char CHAR_QUERY   = '?';
const char CHAR_HI      = '^';
const char CHAR_TILDE   = '~';
const char CHAR_ZERO    = '0';
const char CHAR_NINE    = '9';
const char CHAR_UPPER_A = 'A';
const char CHAR_LOWER_A = 'a';
const char CHAR_LO      = (char)LOGO_LEN;


/** Data describing an Output's operation.
 */
struct SystemData
{
    public:
    
    long    magic           = MAGIC_NUMBER;                 // Magic number to identify software.
    long    version         = VERSION;                      // Software version number to identify upgrades.

    uint8_t i2cControllerID = I2C_DEFAULT_CONTROLLER_ID;    // I2C node IDs.
    uint8_t i2cInputBaseID  = I2C_DEFAULT_INPUT_BASE_ID;
    uint8_t i2cOutputBaseID = I2C_DEFAULT_OUTPUT_BASE_ID;
    uint8_t i2cModuleID     = I2C_MODULE_ID_JUMPERS;        // The module number we're using - default, use hardware.

    uint8_t debugLevel      = DEBUG_ERRORS;                 // Debugging level.
    uint8_t reportLevel     = REPORT_LONG;                  // Reporting level.
    
    int     buttons[6];                                     // Configuration of analog buttons - BUTTON_LIMIT + 1.
    
    char    rfu[6];                                         // RFU. 32 bytes in all.
};


/** An instance of the singleton.
 */
SystemData systemData;


/** Load SystemData from EEPROM
 *  Return true if valid
 */
boolean loadSystemData();


/** Save SystemData.
 */
void saveSystemData();


/** Is debugging enabled at this level?
 */
boolean isDebug(uint8_t aLevel);


/** Gets the debugging level.
 */
uint8_t getDebug();


/** Sets the debugging level.
 */
void setDebug(uint8_t aLevel);


/** Show version number by flashing LED
 *  and reporting it on Serial output.
 */
void flashVersion();


#if MASTER

/** Report a system failure.
 */
void systemFail(PGM_P aMessage, int aValue, int aDelay);


/** Is an EzyBus setup detected?
 */
boolean ezyBusDetected();


/** Make sure Ezybus won't recognise the (Panel) setup.
 */
void ezyBusClear();


#else

/** Gets the output module ID.
 *  Either by hardware jumperss of from EEPROM.
 */
uint8_t getModuleId(boolean aIncludeBase);


#endif


/** Print a number as a string of hex digits.
 *  Padded with leading zeros to length aDigits.
 */
void printHex(int aValue, uint8_t aDigits);


/** Dump a range of the EEPROM memory.
 */
void dumpMemory(PGM_P aMessage, int aStart, int aEnd);


/** Dump all the EEPROM memory.
 */
void dumpMemory();


#endif
