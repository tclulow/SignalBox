/** Input data.
 */


/** Load an Input's data from EEPROM.
 */
void loadInput(int aInput)
{
  if (aInput < INPUT_MAX)
  {
    inputNumber = aInput;
    EEPROM.get(INPUT_BASE + inputNumber * INPUT_SIZE, inputData);
  }
}


/** Load an Input's data from EEPROM.
 */
void loadInput(int aNode, int aInput)
{
  loadInput((aNode << INPUT_NODE_SHIFT) + (aInput & INPUT_PIN_MASK));
}


/** Save an Input's data to EEPROM.
 *  Data in inputNumber and inputData.
 */
void saveInput()
{
  if (inputNumber < INPUT_MAX)
  {
    EEPROM.put(INPUT_BASE + inputNumber * INPUT_SIZE, inputData);
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
