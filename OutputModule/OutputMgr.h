/** Output Manager.
 *  @file
 *
 *
 *  (c)Copyright Tony Clulow  2021    tony.clulow@pentadtech.com
 *
 *  This work is licensed under the:
 *      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 *      http://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 *  For commercial use, please contact the original copyright holder(s) to agree licensing terms.
 */

#ifndef OutputMgr_h
#define OutputMgr_h


// The Outputs' data in RAM - global variable for convenience.
OutputDef outputDefs[OUTPUT_PIN_MAX];


/** An OutputMgr (extends Persisted) for persisting OutputDefs in EEPROM.
 */
class OutputMgr: public Persisted
{
    public:

    /** An OutputMgr.
     */
    OutputMgr(uint16_t aBase) : Persisted(aBase)
    {
        size = OUTPUT_NODE_MAX * sizeof(OutputDef);
    }


    /** Load an Output's definition from EEPROM.
     */
    void loadOutput(uint8_t aPin)
    {
        EEPROM.get(getBase() + aPin * sizeof(OutputDef), outputDefs[aPin]);
        if (isDebug(DEBUG_FULL))
        {
            outputDefs[aPin].printDef(M_DEBUG_LOAD, systemMgr.getModuleId(false), aPin);
        }
    }


    /** Save an Output's definition to EEPROM.
     */
    void saveOutput(uint8_t aPin)
    {
        if (aPin < OUTPUT_PIN_MAX)
        {
            EEPROM.put(getBase() + aPin * sizeof(OutputDef), outputDefs[aPin]);
            if (isDebug(DEBUG_FULL))
            {
                outputDefs[aPin].printDef(M_DEBUG_SAVE, systemMgr.getModuleId(false), aPin);
            }
        }
    }


    /** Is the given Output type a servo type?
     *  ie: SERVO or SIGNAL.
     */
    bool isServo(uint8_t aType)
    {
        return    (aType == OUTPUT_TYPE_SERVO)
               || (aType == OUTPUT_TYPE_SIGNAL);
    }


    /** Is the specified pin a double-led?
     *  The identified pin is legal, a double-led type, and the previous pin is a LED.
     */
    bool isDoubleLed(uint8_t aPin)
    {
        return    (aPin > 0)
               && (aPin < OUTPUT_PIN_MAX)
               && (outputDefs[aPin - 1].getType() == OUTPUT_TYPE_LED)
               && (   (outputDefs[aPin].getType() == OUTPUT_TYPE_LED_3)
                   || (outputDefs[aPin].getType() == OUTPUT_TYPE_LED_4)
                   || (outputDefs[aPin].getType() == OUTPUT_TYPE_ROAD_UK)
                   || (outputDefs[aPin].getType() == OUTPUT_TYPE_ROAD_RW));
    }
};


/** Singleton instance of OutputMgr.
 *  In EEPROM immediately after the end of SystemMgr.
 */
OutputMgr outputMgr(systemMgr.getEnd());


#endif
