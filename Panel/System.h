/** System data.
 */
#ifndef _System_h
#define _System_h


#define MAGIC_NUMBER    0x50616e6c  // Magic number = Panl.
#define VERSION         0x0021      // Version number of software.

#define DEFAULT_REPORT  REPORT_LONG // Default reporting to long.
#define SYSTEM_RFU      8           // Number of RFU bytes in system structure.

#define MILLIS_PER_SECOND (1000L)                       // Millisecs in a second.
#define MILLIS_PER_MINUTE (MILLIS_PER_SECOND * 60L)     // Millisecs in a minute.
#define MILLIS_PER_HOUR   (MILLIS_PER_MINUTE * 60L)     // Millisecs per hour.


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
    int8_t  reportLevel     = 0;            // Reporting level.
    int     buttons[6];                     // Configuration of analog buttons.
    
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

        Serial.println();
    }
}


/** Dump all the EEPROM memory.
 */
void dumpMemory()
{
    dumpMemory(M_SYSTEM, SYSTEM_BASE, SYSTEM_END);
    Serial.println();
    dumpMemory(M_INPUT,  INPUT_BASE,  INPUT_END);
    Serial.println();
    dumpMemory(M_TYPES,  TYPES_BASE,  TYPES_END);
    Serial.println();
}
#endif
