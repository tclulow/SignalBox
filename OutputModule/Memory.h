/** Layout of EEPROM memory.
 */
#ifndef _Memory_h
#define _Memory_h


// System Data saved in EEPROM
#define SYSTEM_BASE  0                                              // EEPROM base of System data.
#define SYSTEM_SIZE  sizeof(SystemData)                             // Size of System Data.
#define SYSTEM_END   (SYSTEM_BASE + SYSTEM_SIZE)                    // End of System EEPROM.

// OutputData saved in EEPROM
#define OUTPUT_BASE  SYSTEM_END                                     // EEPROM base of OutputData.
#define OUTPUT_SIZE  sizeof(OutputDef)                              // Size of OutputData entry.
#define OUTPUT_END   (OUTPUT_BASE + OUTPUT_SIZE * OUTPUT_PIN_MAX)   // End of OutputData EEPROM.

#define EEPROM_END   OUTPUT_END                                     // End of EEPROM memory


#endif
