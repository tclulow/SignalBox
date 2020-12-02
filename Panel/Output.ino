/** Output
 */


/** Read an Output's data from an OutputModule.
 */
void readOutput(uint8_t aNode, uint8_t aPin)
{
    int error = 0;
    
    outputNode = aNode;
    outputPin  = aPin;

    Wire.beginTransmission(systemData.i2cOutputBaseID + outputNode);
    Wire.write(COMMS_CMD_READ | outputPin);
    error = Wire.endTransmission();

    if (error)
    {
        systemFail(M_MCP_ERROR, error, DELAY_READ);
    }
    else if ((error = Wire.requestFrom(systemData.i2cOutputBaseID + outputNode, OUTPUT_WRITE_LEN)) != OUTPUT_WRITE_LEN)
    {
        systemFail(M_MCP_COMMS, error, DELAY_READ);
    }
    else
    {
        // Read the outputDef from the OutputModule.
        outputDef.read();
    }
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
    Wire.endTransmission();
}

 
