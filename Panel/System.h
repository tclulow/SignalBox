/** System data.
 */
#ifndef _System_h
#define _System_h


#define MAGIC_NUMBER    0x50616e6c  // Magic number = Panl.
#define VERSION         0x0012      // Version number of software.

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


#endif
