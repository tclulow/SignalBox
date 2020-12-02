/** Output
 */


/** Read an Output's data from an OutputModule.
 */
void readOutput(uint8_t aNode, uint8_t aPin)
{
    outputNode = aNode;
    outputPin  = aPin;

    Wire.beginTransmission(systemData.i2cOutputBaseID + outputNode);
    Wire.write(COMMS_CMD_READ | outputPin);
    Wire.endTransmission();

    // Read 4-byte response.
    outputDef.read();
}


/** Read an Output's data from an OutputModule.
 */
void readOutput(uint8_t aOutputNumber)
{
    readOutput((aOutputNumber >> OUTPUT_NODE_SHIFT) & OUTPUT_NODE_MASK, aOutputNumber & OUTPUT_PIN_MASK);
}


/** Write an Output's data to an OutputModule.
 */
void writeOutput()
{
    Wire.beginTransmission(systemData.i2cOutputBaseID + outputNode);
    Wire.write(COMMS_CMD_WRITE | outputPin);
    outputDef.write();
    return Wire.endTransmission();
}

 
