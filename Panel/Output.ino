/** Output data.
 */


/** Load an Output's data from EEPROM.
 */
void loadOutput(int aOutput)
{
  if (aOutput < OUTPUT_MAX)
  {
    outputNumber = aOutput;
    EEPROM.get(OUTPUT_BASE + outputNumber * OUTPUT_SIZE, outputData); 
  }
}


/** Load an Output's data from EEPROM.
 */
void loadOutput(int aNode, int aOutput)
{
  loadOutput((aNode << OUTPUT_NODE_SHIFT) + (aOutput & OUTPUT_PIN_MASK));
}


/** Save an Output's data to EEPROM.
 *  Data in outputNumber and outputData.
 */
void saveOutput()
{
  if (outputNumber < OUTPUT_MAX)
  {
    EEPROM.put(OUTPUT_BASE + outputNumber * OUTPUT_SIZE, outputData);
  }
}


/** Record the presence of an OutputNode in the map.
 */
void setOutputNodePresent(int aNode)
{
  outputNodes |= (1 << aNode); 
}


/** Is an Output node present?
 *  Look for Output's node in outputNodes.
 */
boolean isOutputNode(int aNode)
{
  return (aNode < OUTPUT_NODE_MAX) && (outputNodes & (1 << aNode));
}


/** Is an Output present?
 *  Look for output's node in outputNodes.
 */
boolean isOutput(int aOutput)
{
  return isOutputNode(aOutput >> OUTPUT_NODE_SHIFT);
}
