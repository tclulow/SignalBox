/** Layout of EEPROM memory.
 */

#ifndef _Memory_h
#define _Memory_h


// System Data saved in EEPROM
#define SYSTEM_BASE  0                                            // EEPROM base of System data.
#define SYSTEM_SIZE  sizeof(SystemData)                           // Size of System Data.
#define SYSTEM_END   (SYSTEM_BASE + SYSTEM_SIZE)                  // End of System EEPROM.

// OutputData saved in EEPROM
#define OUTPUT_BASE  SYSTEM_END                                   // EEPROM base of OutputData.
#define OUTPUT_SIZE  sizeof(OutputData)                           // Size of OutputData entry.
#define OUTPUT_MAX   (OUTPUT_NODE_SIZE * OUTPUT_NODE_MAX)         // Maximum outputs (up to 128, 16 nodes, 8 outputs each).
#define OUTPUT_END   (OUTPUT_BASE + OUTPUT_SIZE * OUTPUT_MAX)     // End of OutputData EEPROM.

// InputData saved in EEPROM
#define INPUT_BASE   OUTPUT_END                                   // EEPROM base of Input data.
#define INPUT_SIZE   sizeof(InputData)                            // Size of InputData entry.
#define INPUT_MAX    (INPUT_NODE_SIZE * INPUT_NODE_MAX)           // Maximum inputs (up to 128, 8 nodes, 16 inputs each).
#define INPUT_END    (INPUT_BASE + INPUT_SIZE * INPUT_MAX)        // End of Input EEPROM.

// Input types saved in EEPROM
#define TYPES_BASE   INPUT_END                                    // EEPROM base of Input type data.
#define TYPES_SIZE   sizeof(uint32_t)                             // Size of Input types.
#define TYPES_END    (TYPES_BASE + TYPES_SIZE * INPUT_NODE_MAX)   // End of Input Types EEPROM.

// Input types saved in EEPROM
#define STATES_BASE  TYPES_END                                    // EEPROM base of Input states.
#define STATES_SIZE  sizeof(uint32_t)                             // Size of Input states.
#define STATES_END   (STATES_BASE + STATES_SIZE * INPUT_NODE_MAX) // End of Input states EEPROM.

#define EEPROM_END   STATES_END                                   // End of EEPROM memory


#endif
