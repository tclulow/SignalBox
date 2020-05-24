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
  
  long    magic = 0;
  long    version = 0;
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
