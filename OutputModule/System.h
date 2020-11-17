/** System data.
 */
#ifndef _System_h
#define _System_h


#define MAGIC_NUMBER 0x50616e6f       // Magic number = "Pano.
#define VERSION      0x0001           // Version number of software.


/** Data describing an Output's operation.
 */
struct SystemData
{
    public:
    
    long    magic           = 0;    // Magic number to identify software.
    long    version         = 0;    // Software version number to identify upgrades.

    uint8_t i2cControllerID = 0;    // I2C node IDs.
    uint8_t i2cInputBaseID  = 0;
    uint8_t i2cOutputBaseID = 0;
    
    char    rfu[21]         = "RFU rfu OutputModule";                 // RFU. 32 bytes in all.
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


#endif
