/** Input data.
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

 #include "All.h"


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
    uint32_t mask = ((long)INPUT_TYPE_MASK) << (aPin << INPUT_TYPE_SHIFT);

    inputNumber = ((aNode & INPUT_NODE_MASK) << INPUT_NODE_SHIFT) | (aPin & INPUT_PIN_MASK);

    EEPROM.get(INPUT_BASE + (inputNumber * INPUT_SIZE), inputDef);
    EEPROM.get(TYPES_BASE + (aNode       * TYPES_SIZE), inputTypes);
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

//        Serial.print(millis());
//        Serial.print("\tLoad types ");
//        Serial.print(aNode, HEX);
//        Serial.print(CHAR_COMMA);
//        Serial.print(aPin, HEX);
//        Serial.print(" @");
//        Serial.print(TYPES_BASE + (aNode * TYPES_SIZE), HEX);
//        Serial.print(" = ");
//        Serial.print(inputTypes, HEX);
//        Serial.print(" (");
//        Serial.print(inputType, HEX);
//        Serial.print(")");
//        Serial.println();
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
        EEPROM.put(INPUT_BASE + (inputNumber * INPUT_SIZE), inputDef);
        EEPROM.put(TYPES_BASE + (node        * TYPES_SIZE), inputTypes);

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
            
//            Serial.print(millis());
//            Serial.print("\tSave types ");
//            Serial.print(node, HEX);
//            Serial.print(",");
//            Serial.print(pin, HEX);
//            Serial.print("] @");
//            Serial.print(TYPES_BASE + (node * TYPES_SIZE), HEX);
//            Serial.print(", mask=");
//            Serial.print(mask, HEX);
//            Serial.print(" = ");
//            Serial.print(inputTypes, HEX);
//            Serial.print(" (");
//            Serial.print(inputType, HEX);
//            Serial.print(")");
//            Serial.println();
        }
    }
}


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
