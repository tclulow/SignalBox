/** Input data.
 */


/** Load an Input's data from EEPROM.
 */
void loadInput(int aInput)
{
    loadInput((aInput >> INPUT_NODE_SHIFT) & INPUT_NODE_MASK, aInput & INPUT_PIN_MASK);
}


/** Load an Input's data from EEPROM.
 */
void loadInput(int aNode, int aPin)
{
    uint32_t mask = ((long)INPUT_TYPE_MASK) << (aPin << INPUT_TYPE_SHIFT);

    inputNumber = ((aNode & INPUT_NODE_MASK) << INPUT_NODE_SHIFT) | (aPin & INPUT_PIN_MASK);

    EEPROM.get(INPUT_BASE + (inputNumber * INPUT_SIZE), inputData);
    EEPROM.get(TYPES_BASE + (aNode       * TYPES_SIZE), inputTypes);
    inputType = (inputTypes >> (aPin << INPUT_TYPE_SHIFT)) & INPUT_TYPE_MASK;
    
//  Serial.print("Load inputTypes[");
//  Serial.print(inputNumber, HEX);
//  Serial.print(": ");
//  Serial.print(aNode, HEX);
//  Serial.print(",");
//  Serial.print(aPin, HEX);
//  Serial.print("] @");
//  Serial.print(TYPES_BASE + (aNode * TYPES_SIZE), HEX);
//  Serial.print(" = ");
//  Serial.print(inputTypes, HEX);
//  Serial.print(" (");
//  Serial.print(inputType, HEX);
//  Serial.print(")");
//  Serial.println();
}


/** Save an Input's data to EEPROM.
 *  Data in inputNumber and inputData.
 */
void saveInput()
{
    if (inputNumber < INPUT_MAX)
    {
        int      node = (inputNumber >> INPUT_NODE_SHIFT) & INPUT_NODE_MASK;
        int      pin  = (inputNumber                    ) & INPUT_PIN_MASK;
        uint32_t mask = ((long)INPUT_TYPE_MASK) << (pin << INPUT_TYPE_SHIFT);
        
        inputTypes = (inputTypes & ~mask) | ((((long)inputType) << (pin << INPUT_TYPE_SHIFT)) & mask);
        EEPROM.put(INPUT_BASE + (inputNumber * INPUT_SIZE), inputData);
        EEPROM.put(TYPES_BASE + (node        * TYPES_SIZE), inputTypes);

//        Serial.print("Save Input ");
//        Serial.print(inputNumber, HEX);
//        Serial.print(": output=");
//        Serial.print(inputData.output[0], HEX);
//        Serial.print(CHAR_DOT);
//        Serial.print(inputData.output[1], HEX);
//        Serial.print(CHAR_DOT);
//        Serial.print(inputData.output[2], HEX);
//        Serial.print(" @ ");
//        Serial.print(INPUT_BASE + (inputNumber * INPUT_SIZE), HEX);
//        Serial.println();
        
//        Serial.print("Save inputTypes[");
//        Serial.print(inputNumber, HEX);
//        Serial.print(": ");
//        Serial.print(node, HEX);
//        Serial.print(",");
//        Serial.print(pin, HEX);
//        Serial.print("] @");
//        Serial.print(TYPES_BASE + (node * TYPES_SIZE), HEX);
//        Serial.print(", mask=");
//        Serial.print(mask, HEX);
//        Serial.print(" = ");
//        Serial.print(inputTypes, HEX);
//        Serial.print(" (");
//        Serial.print(inputType, HEX);
//        Serial.print(")");
//        Serial.println();
    }
}


/** Record the presence of an InputNode in the map.
 */
void setInputNodePresent(int aNode)
{
    inputNodes |= (1 << aNode); 
}


/** Is an Input node present?
 *  Look for input's node in inputNodes.
 */
boolean isInputNode(int aNode)
{
    return (aNode < INPUT_NODE_MAX) && (inputNodes & (1 << aNode));
}


/** Is an Input present?
 *  Look for input's node in inputNodes.
 */
boolean isInput(int aInput)
{
    return isInputNode(aInput >> INPUT_NODE_SHIFT);
}
