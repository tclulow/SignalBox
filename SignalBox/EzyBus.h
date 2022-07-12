/** EzyBus definitions.
 *  @file
 *
 *  (c)Copyright Tony Clulow  2021    tony.clulow@pentadtech.com
 *
 *  This work is licensed under the:
 *      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 *      http://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 *  For commercial use, please contact the original copyright holder(s) to agree licensing terms.
 */

#ifndef EzyBus_h
#define EzyBus_h


#if EZYBUS_CONVERT

const int     EZY_BASE        =   0;    // EEPROM address of output module definitions.
const uint8_t EZY_OUTPUT_SIZE =   4;    // Four bytes for an EzyBus output definition.

const int     EZY_MAGIC_ADDR  = 641;    // EEPROM address of EzyBus magic number.
const uint8_t EZY_MAGIC       =  90;    // EzyBus magic number.
const uint8_t EZY_NODE_MAX    =  16;    // EzyBus max node number.
const uint8_t EZY_SPEED_SHIFT =   3;    // EzyBus speed is shifted 3 bits.


/** Is an EzyBus setup detected?
 */
bool ezyBusDetected()
{
    return EEPROM.read(EZY_MAGIC_ADDR) == EZY_MAGIC;    // Check for tell-tale value in EEPROM
}


/** Make sure Ezybus won't recognise the (Panel) setup.
 */
void ezyBusClear()
{
    if (ezyBusDetected())
    {
        EEPROM.put(EZY_MAGIC_ADDR, EZY_MAGIC + 1);      // Corrupt the EzyBus magic number slightly.
    }
}

#endif

#endif
