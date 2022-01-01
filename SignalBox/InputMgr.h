/** Input manager.
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

#ifndef InputMgr_h
#define InputMgr_h


// Input types and definitions saved in EEPROM
#define INPUT_TYPES_SIZE    sizeof(uint32_t)                    // Size of Input types.
#define INPUT_SIZE          (sizeof(InputDef))                  // Size of an InputDef.
#define INPUT_MAX           (INPUT_NODE_MAX * INPUT_PIN_MAX)    // Maximum inputs.


/** Variables for working with an Input.
 *  Global for convenience.
 */
uint16_t   inputNodes  = 0;                 // Bit map of Input nodes present.
uint8_t    inputNumber = 0;                 // Current Input number.
InputDef   inputDef;                        // Definition of the current Input.
uint32_t   inputTypes  = 0L;                // The types of the Inputs. 2 bits per pin, 16 pins per node = 32 bits.
uint8_t    inputType   = 0;                 // Type of the current Input (2 bits, INPUT_TYPE_MASK).


/** Record the presence of an InputNode in the map.
 */
void setInputNodePresent(uint8_t aNode, boolean aState)
{
    if (aState)
    {
        inputNodes |= (1 << aNode);
    }
    else
    {
        inputNodes &= ~(1 << aNode);
    }
}


/** Is an Input node present?
 */
boolean isInputNodePresent(uint8_t aNode)
{
    // Look for input's node in inputNodes flags.
    return (aNode < INPUT_NODE_MAX) && (inputNodes & (1 << aNode));
}


/** An InputMgr (extends Persisted) for persisting InputDefs in EEPROM.
 */
class InputMgr: public Persisted
{
    private:

    uint16_t baseInputs;                    // Base of InputDefs.


    public:

    /** An InputMgr.
     */
    InputMgr(uint16_t aBase) : Persisted(aBase)
    {
        baseInputs = base + INPUT_TYPES_SIZE * INPUT_NODE_MAX;                // Immediately after Input types.
        size = INPUT_TYPES_SIZE * INPUT_NODE_MAX + INPUT_SIZE * INPUT_MAX;    // Types + definitions.
    }


    /** Load an Input's data from EEPROM.
     */
    void loadInput(uint8_t aInput)
    {
        loadInput((aInput >> INPUT_NODE_SHIFT) & INPUT_NODE_MASK, aInput & INPUT_PIN_MASK);
    }


    /** Load an Input's data from EEPROM.
     */
    void loadInput(uint8_t aNode, uint8_t aPin)
    {
        inputNumber = ((aNode & INPUT_NODE_MASK) << INPUT_NODE_SHIFT) | (aPin & INPUT_PIN_MASK);

        EEPROM.get(base       + (aNode       * INPUT_TYPES_SIZE), inputTypes);
        EEPROM.get(baseInputs + (inputNumber * INPUT_SIZE), inputDef);
        inputType = (inputTypes >> (aPin << INPUT_TYPE_SHIFT)) & INPUT_TYPE_MASK;

        if (isDebug(DEBUG_DETAIL))
        {
            Serial.print(millis());
            Serial.print(CHAR_TAB);
            Serial.print(PGMT(M_DEBUG_LOAD));
            Serial.print(CHAR_SPACE);
            Serial.print(inputNumber, HEX);
            Serial.print(PGMT(M_DEBUG_TYPE));
            Serial.print(PGMT(M_INPUT_TYPES[inputType]));
            Serial.print(PGMT(M_DEBUG_OUTPUTS));
            for (uint8_t index = 0; index < INPUT_OUTPUT_MAX; index++)
            {
                Serial.print(CHAR_SPACE);
                Serial.print(inputDef.getOutputNode(index), HEX);
                Serial.print(inputDef.getOutputPin(index), HEX);
                if (inputDef.isDelay(index))
                {
                    Serial.print(CHAR_STAR);
                }
                else
                {
                    Serial.print(CHAR_SPACE);
                }
            }
            Serial.println();

//            Serial.print(millis());
//            Serial.print("\tLoad types ");
//            Serial.print(aNode, HEX);
//            Serial.print(CHAR_COMMA);
//            Serial.print(aPin, HEX);
//            Serial.print(" @");
//            Serial.print(TYPES_BASE + (aNode * INPUT_TYPES_SIZE), HEX);
//            Serial.print(" = ");
//            Serial.print(inputTypes, HEX);
//            Serial.print(" (");
//            Serial.print(inputType, HEX);
//            Serial.print(")");
//            Serial.println();
        }
    }


    /** Save an Input's data to EEPROM.
     *  Data in inputNumber and inputDef.
     */
    void saveInput()
    {
        if (inputNumber < INPUT_MAX)
        {
            uint8_t  node = (inputNumber >> INPUT_NODE_SHIFT) & INPUT_NODE_MASK;
            uint8_t  pin  = (inputNumber                    ) & INPUT_PIN_MASK;
            uint32_t mask = ((long)INPUT_TYPE_MASK) << (pin << INPUT_TYPE_SHIFT);

            inputTypes = (inputTypes & ~mask) | ((((long)inputType) << (pin << INPUT_TYPE_SHIFT)) & mask);
            EEPROM.put(base       + (node        * INPUT_TYPES_SIZE), inputTypes);
            EEPROM.put(baseInputs + (inputNumber * INPUT_SIZE), inputDef);

            if (isDebug(DEBUG_DETAIL))
            {
                Serial.print(millis());
                Serial.print(CHAR_TAB);
                Serial.print(PGMT(M_DEBUG_SAVE));
                Serial.print(CHAR_SPACE);
                Serial.print(inputNumber, HEX);
                Serial.print(PGMT(M_DEBUG_TYPE));
                Serial.print(PGMT(M_INPUT_TYPES[inputType]));
                Serial.print(PGMT(M_DEBUG_OUTPUTS));
                for (uint8_t index = 0; index < INPUT_OUTPUT_MAX; index++)
                {
                    Serial.print(CHAR_SPACE);
                    Serial.print(inputDef.getOutputNode(index), HEX);
                    Serial.print(inputDef.getOutputPin(index), HEX);
                    if (inputDef.isDelay(index))
                    {
                        Serial.print(CHAR_STAR);
                    }
                    else
                    {
                        Serial.print(CHAR_SPACE);
                    }
                }
                Serial.println();

//                Serial.print(millis());
//                Serial.print("\tSave types ");
//                Serial.print(node, HEX);
//                Serial.print(",");
//                Serial.print(pin, HEX);
//                Serial.print("] @");
//                Serial.print(TYPES_BASE + (node * INPUT_TYPES_SIZE), HEX);
//                Serial.print(", mask=");
//                Serial.print(mask, HEX);
//                Serial.print(" = ");
//                Serial.print(inputTypes, HEX);
//                Serial.print(" (");
//                Serial.print(inputType, HEX);
//                Serial.print(")");
//                Serial.println();
            }
        }
    }
};


/** Singleton instance of the class.
 *  In EEPROM immediately after the end of SystemMgr.
 */
InputMgr inputMgr(systemMgr.getEnd());


#endif
