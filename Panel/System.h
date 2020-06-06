/** System data.
 */
#ifndef _System_h
#define _System_h

// System Data saved in EEPROM
#define SYSTEM_BASE  0                            // EEPROM base of System data.
#define SYSTEM_SIZE  sizeof(SystemData)           // Size of System Data.
#define SYSTEM_END   (SYSTEM_BASE + SYSTEM_SIZE)  // End of System EEPROM.

#define MAGIC_NUMBER 0x50616f6e                   // Majic number.
#define VERSION      0x0001                       // Version number of software.


/** Data describing an Output's operation.
 */
class SystemData
{
  public:
  
  long    magic           = 0;    // Magic number to identify software.
  long    version         = 0;    // Software version number to identify upgrades.

  uint8_t i2cControllerID = 0;    // I2C node IDs.
  uint8_t i2cInputBaseID  = 0;
  uint8_t i2cOutputBaseID = 0;
  uint8_t filler1         = 0;    // Filler for word boundary.

  int     buttons[6];             // Configuration of analog buttons.
  
  char    rfu[8]          = "RFU rfu";                 // RFU. 32 bytes in all.
};


/** An instance of the singleton.
 */
SystemData systemData;


/** Load SystemData from EEPROM
 *  Return true if valid
 */
boolean loadSystemData()
{
  EEPROM.get(SYSTEM_BASE, systemData);
  return systemData.magic == MAGIC_NUMBER;
}


/** Save SystemData.
 */
void saveSystemData()
{
  EEPROM.put(SYSTEM_BASE, systemData);
}


#endif
