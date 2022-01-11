/** Something that's persisted in EEPROM.
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

#ifndef Persisted_h
#define Persisted_h


#include <EEPROM.h>


/** An object that's persisted in EEPROM.
 */
class Persisted
{
    private:
    
    uint16_t base = 0;


    protected:

    uint16_t size = 0;


    public:

    /** A Persisted object.
     *  Persisted in EEPROM at the given base offset.
     */
    Persisted(uint16_t aBase)
    {
        base = aBase;
    };


    /** Gets the base offset (in EEPROM) where this user stores data.
     */
    uint16_t getBase()
    {
        return base;
    }


    /** Gets the end of the data this user stores in EEPROM.
     */
    uint16_t getEnd()
    {
        return base + size;
    }
};


#endif
