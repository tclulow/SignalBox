/** EzyBus definitions.
 *
 *
 *  (c)Copyright Tony Clulow  2021    tony.clulow@pentadtech.com
 *
 *  This work is licensed under the:
 *      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 *      http://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 *  For commercial use, please contact the original copyright holder(s) to agree licensing terms
 */
#ifndef EzyBus_h
#define EzyBus_h

#define EZY_BASE          0   // EEPROM address of output module definitions.
#define EZY_OUTPUT_SIZE   4   // Four bytes for an EzyBus output definition.

#define EZY_MAGIC_ADDR  641   // EEPROM address of EzyBus magic number.
#define EZY_MAGIC        90   // EzyBus magic number.
#define EZY_NODE_MAX     16   // EzyBus max node number.
#define EZY_SPEED_SHIFT   3   // EzyBus speed is shifted 3 bits.


#endif
