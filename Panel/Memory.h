/** Layout of EEPROM memory.
 */

#ifndef _Memory_h
#define _Memory_h


// System Data saved in EEPROM
#define SYSTEM_BASE  0                                            // EEPROM base of System data.
#define SYSTEM_SIZE  sizeof(SystemData)                           // Size of System Data.
#define SYSTEM_END   (SYSTEM_BASE + SYSTEM_SIZE)                  // End of System EEPROM.

// InputDef saved in EEPROM
#define INPUT_BASE   SYSTEM_END                                   // EEPROM base of Input data.
#define INPUT_SIZE   sizeof(InputDef)                             // Size of InputData entry.
#define INPUT_MAX    (INPUT_NODE_MAX * INPUT_PIN_MAX)             // Maximum inputs (16 nodes with 8 pins each).
#define INPUT_END    (INPUT_BASE + INPUT_SIZE * INPUT_MAX)        // End of Input EEPROM.

// Input types saved in EEPROM
#define TYPES_BASE   INPUT_END                                    // EEPROM base of Input type data.
#define TYPES_SIZE   sizeof(uint32_t)                             // Size of Input types.
#define TYPES_END    (TYPES_BASE + TYPES_SIZE * INPUT_NODE_MAX)   // End of Input Types EEPROM.

#define EEPROM_END   TYPES_END                                   // End of EEPROM memory


#endif
