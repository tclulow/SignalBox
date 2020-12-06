/** Output
 */


/** Read an Output's data from an OutputModule.
 */
void readOutput(uint8_t aNode, uint8_t aPin)
{
    if (!isOutputNode(aNode))
    {
        outputDef.set(OUTPUT_TYPE_NONE, false, OUTPUT_DEFAULT_LO, OUTPUT_DEFAULT_HI, OUTPUT_DEFAULT_PACE, 0);
    }
    else
    {
        int error = 0;

        outputNode = aNode;
        outputPin  = aPin;

        #if DEBUG
            Serial.print(millis());
            Serial.print("\tRead node=");
            Serial.print(outputNode, HEX);
            Serial.print(", pin=");
            Serial.print(outputPin, HEX);
            Serial.println();
        #endif
    
        Wire.beginTransmission(systemData.i2cOutputBaseID + outputNode);
        Wire.write(COMMS_CMD_READ | outputPin);
        error = Wire.endTransmission();
    
        if (error)
        {
            systemFail(M_MCP_ERROR, error, DELAY_READ);
            outputDef.set(OUTPUT_TYPE_NONE, false, OUTPUT_DEFAULT_LO, OUTPUT_DEFAULT_HI, OUTPUT_DEFAULT_PACE, 0);
        }
        else if ((error = Wire.requestFrom(systemData.i2cOutputBaseID + outputNode, OUTPUT_WRITE_LEN)) != OUTPUT_WRITE_LEN)
        {
            systemFail(M_MCP_COMMS, error, DELAY_READ);
            outputDef.set(OUTPUT_TYPE_NONE, false, OUTPUT_DEFAULT_LO, OUTPUT_DEFAULT_HI, OUTPUT_DEFAULT_PACE, 0);
        }
        else
        {
            // Read the outputDef from the OutputModule.
            outputDef.read();
            outputDef.printDef("Output", outputPin);
        }
    }
}


/** Read an Output's data from an OutputModule.
 */
void readOutput(uint8_t aOutputNumber)
{
    readOutput((aOutputNumber >> OUTPUT_NODE_SHIFT) & OUTPUT_NODE_MASK, aOutputNumber & OUTPUT_PIN_MASK);
}


/** Write current Output's data to its OutputModule.
 *  With save it if so requested.
 */
void writeOutput(boolean aSave)
{
    #if DEBUG
        Serial.print(millis());
        Serial.print("\tRead node=");
        Serial.print(outputNode, HEX);
        Serial.print(", pin=");
        Serial.print(outputPin, HEX);
        Serial.println();
        outputDef.printDef((aSave ? "Save" : "Write"), outputPin);
    #endif

    Wire.beginTransmission(systemData.i2cOutputBaseID + outputNode);
    Wire.write((aSave ? COMMS_CMD_SAVE : COMMS_CMD_WRITE) | outputPin);
    outputDef.write();
    Wire.endTransmission();
}


/** Write a change of state to the Output module.
 */
void writeOutputState(boolean aState, uint8_t aDelay)
{
    Wire.beginTransmission(systemData.i2cOutputBaseID + outputNode);
    Wire.write((aState ? COMMS_CMD_SET_HI : COMMS_CMD_SET_LO) | outputPin);
    Wire.write(aDelay);
    Wire.endTransmission();
}


/** Reset current Output. 
 *  And then reload its definition.
 */
void resetOutput()
{
    #if DEBUG
        Serial.print(millis());
        Serial.print("\tReset node=");
        Serial.print(outputNode, HEX);
        Serial.print(", pin=");
        Serial.print(outputPin, HEX);
        Serial.println();
        outputDef.printDef("Reset", outputPin);
    #endif

    Wire.beginTransmission(systemData.i2cOutputBaseID + outputNode);
    Wire.write(COMMS_CMD_RESET | outputPin);
    Wire.endTransmission();

    // Reload the Output now it's been reset.
    readOutput(outputNode, outputPin);
}

/** Read the states of the given node's Outputs.
 *  Save in OutputStates.
 *  If fails, return a character indicating the error.
 */
char readOutputStates(uint8_t aNode)
{
    char error = 0;
    
    Wire.beginTransmission(systemData.i2cOutputBaseID + aNode);
    Wire.write(COMMS_CMD_STATES);
    if (Wire.endTransmission())
    {
        error = CHAR_DOT;       // No such node on the bus.
    }
    else if (Wire.requestFrom(systemData.i2cOutputBaseID + aNode, OUTPUT_STATE_LEN) != OUTPUT_STATE_LEN)
    {
        error = CHAR_HASH;
    }
    else
    {
        int states = Wire.read();

        #if DEBUG
            Serial.print(millis());
            Serial.print("\tState ");
            Serial.print(aNode, HEX);
            Serial.print(CHAR_SPACE);
            Serial.print(states, HEX);
            Serial.println();
        #endif
        
        if (states < 0)
        {
            error = CHAR_STAR;
        }
        else
        {
            setOutputNodePresent(aNode);
            setOutputStates(aNode, states);
        }
    }

    return error;
}
