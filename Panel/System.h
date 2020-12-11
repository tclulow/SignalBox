/** System data.
 */
#ifndef _System_h
#define _System_h


#define MAGIC_NUMBER    0x50616e6c          // Magic number = Panl.
#define VERSION         0x0023              // Version number of software.  See also M_VERSION.

#define SYSTEM_RFU      7                   // 7 rfu bytes in systemData.
#define DEFAULT_REPORT  REPORT_LONG         // Default reporting to long.
#define DEFAULT_DEBUG   DEBUG_ERRORS        // Default debugging to erors only.

#define MILLIS_PER_SECOND (1000L)                       // Millisecs in a second.
#define MILLIS_PER_MINUTE (MILLIS_PER_SECOND * 60L)     // Millisecs in a minute.
#define MILLIS_PER_HOUR   (MILLIS_PER_MINUTE * 60L)     // Millisecs per hour.


// Useful characters
const char HEX_CHARS[]  = "0123456789abcdef";
const char EDIT_CHARS[] = "ABC";


/** Data describing an Output's operation.
 */
struct SystemData
{
    public:
    
    long    magic           = 0;            // Magic number to identify software.
    long    version         = 0;            // Software version number to identify upgrades.

    uint8_t i2cControllerID = 0;            // I2C node IDs.
    uint8_t i2cInputBaseID  = 0;
    uint8_t i2cOutputBaseID = 0;
    uint8_t reportLevel     = 0;            // Reporting level.
    int     buttons[6];                     // Configuration of analog buttons.
    uint8_t debugLevel      = 0;            // Debugging level.
    
    char    rfu[SYSTEM_RFU];                // RFU. 32 bytes in all.
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


/** Report a system failure.
 */
void systemFail(PGM_P aMessage, int aValue, int aDelay);


/** Is an EzyBus setup detected?
 */
boolean ezyBusDetected();


/** Make sure Ezybus won't recognise the (Panel) setup.
 */
void ezyBusClear();


/** Print a number as a string of hex digits.
 *  Padded with leading zeros to length aDigits.
 */
void printHex(int aValue, int aDigits)
{
    for (int digit = aDigits - 1; digit >= 0; digit--)
    {
        Serial.print(HEX_CHARS[(aValue >> (digit << 2)) & 0xf]);
    }
}


/** Dump a range of the EEPROM memory.
 */
void dumpMemory(PGM_P aMessage, int aStart, int aEnd);


/** Dump all the EEPROM memory.
 */
void dumpMemory();


#endif
