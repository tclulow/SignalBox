/** Output data.
 */
#ifndef _Output_h
#define _Output_h

// Output nodes.
#define OUTPUT_PIN_MAX            8   // 8 outputs to each node.        Was OUTPUT_NODE_SIZE
#define OUTPUT_PIN_MASK           7   // 3 bits for 8 pins withing an output node.
#define OUTPUT_NODE_MAX          16   // Maximum nodes.
#define OUTPUT_NODE_MASK       0x0f   // 4 bits for 16 nodes.
#define OUTPUT_NODE_ALL_MASK 0xffff   // All output nodes present.
#define OUTPUT_NODE_SHIFT         3   // Shift output number this amount to get a node number.

// Output options maxima.
#define OUTPUT_SERVO_MAX        180   // Maximum value an angle output parameter can take.
#define OUTPUT_LED_MAX          255   // Maximum value a LED can take.
#define OUTPUT_HI_LO_SIZE         3   // Maximum digits in a Hi/Lo display.

// Masks for type, state, pace and delay options within the outputDef.
#define OUTPUT_STATE_MASK      0x80   // On or off, switched or not switched, 0 = lo, 1 = hi.    Was OUTPUT_STATE
#define OUTPUT_TYPE_MASK       0x0f   // Output type mask (4 bits).
#define OUTPUT_PACE_SHIFT         4   // Pace is in the left-most nibble.
#define OUTPUT_PACE_MASK       0x0f   // Pace is 4 bits.
#define OUTPUT_PACE_MULT          4   // Pace is multiplied by 16 (shifted left 4 bits).
#define OUTPUT_DELAY_MASK      0x0f   // Delay is right-most nibble of output.pace.

// Wire response message lengths.
#define OUTPUT_STATE_LEN          1   // One byte used to return a node's Outputs' states.
#define OUTPUT_WRITE_LEN          4   // Four bytes used to read/write OutputDef to/from OutputModule.

// Defaults when initialising
#define OUTPUT_DEFAULT_LO        90   // Default low  position is 90 degrees.
#define OUTPUT_DEFAULT_HI        90   // Default high position is 90 degrees.
#define OUTPUT_DEFAULT_PACE     0xc   // Default pace is mid-range.
#define OUTPUT_DEFAULT_DELAY    0x0   // Default delay is none.


// Output information that's shared with the output module.
#define OUTPUT_TYPE_NONE       0x00   // Placeholder to mark "no type".
#define OUTPUT_TYPE_SERVO      0x01   // Output is a servo.
#define OUTPUT_TYPE_SIGNAL     0x02   // Output is a signal.
#define OUTPUT_TYPE_LED        0x03   // Output is a LED or other IO device.
#define OUTPUT_TYPE_FLASH      0x04   // Output is a flashing LED.
#define OUTPUT_TYPE_BLINK      0x05   // Output is a blinking LED.
#define OUTPUT_TYPE_MAX        0x06   // Limit of output types.


/** Definition of an Output.
 */
class OutputDef
{
    private:

    uint8_t type  = 0;
    uint8_t lo    = 0;
    uint8_t hi    = 0;
    uint8_t pace  = 0;


    public:

    /** Is the Output one of the servo type?
     */
    boolean isServo()
    {
        return    (getType() == OUTPUT_TYPE_SERVO)
               || (getType() == OUTPUT_TYPE_SIGNAL);
    }


    /** Is the Output one of the LED types?
     */
    boolean isLed()
    {
        return getType() == OUTPUT_TYPE_LED;
    }


    /** Is the Output one of the flashing types?
     */
    boolean isFlasher()
    {
        return    (getType() == OUTPUT_TYPE_FLASH)
               || (getType() == OUTPUT_TYPE_BLINK);
    }


    /** Gets the target any movement is aiming for.
     */
    uint8_t getTarget()
    {
        return getState() ? hi : lo;
    }


    /** Gets the alt target any movement is aiming for.
     */
    uint8_t getAltTarget()
    {
        return getState() ? lo : hi;
    }


    /** Gets the pace as steps. 
     *  From 0 -> f to 240 -> 0.
     */
    uint8_t getPaceAsSteps()
    {
        return (OUTPUT_PACE_MASK - getPace()) << OUTPUT_PACE_MULT;
    }


    /** Set all an Output's data.
     */
    void set(uint8_t aType, uint8_t aState, uint8_t aLo, uint8_t aHi, uint8_t aPace, uint8_t aDelay)
    {
        setType(aType);
        setState(aState);
        setLo(aLo);
        setHi(aHi);
        setPace(aPace);
        setDelay(aDelay);        
    }


    /** Write an Output down the i2c bus.
     */
    void write()
    {
        Wire.write(type);
        Wire.write(lo);
        Wire.write(hi);
        Wire.write(pace);
    }


    /** Read an Output from the i2c bus.
     */
    void read()
    {
        type = Wire.read();
        lo   = Wire.read();
        hi   = Wire.read();
        pace = Wire.read();
    }


    /** Prints an Output's definition.
     */
    void printDef(char* aHeader, uint8_t aPin)
    {
        #if DEBUG
            Serial.print(millis());
            Serial.print("\t");
            Serial.print(aHeader);
            Serial.print(aPin);
            Serial.print("\ttype ");
            Serial.print(getType(),  HEX);
            Serial.print(", state ");
            Serial.print(getState(), HEX);
            Serial.print(", Lo ");
            Serial.print(getLo(),    HEX);
            Serial.print(", Hi ");
            Serial.print(getHi(),    HEX);
            Serial.print(", pace ");
            Serial.print(getPace(),  HEX);
            Serial.print(", delay ");
            Serial.print(getDelay(), HEX);
            Serial.println();
        #endif
    }

    
    /** Gets the output's type.
     */
    uint8_t getType()
    {
        return type & OUTPUT_TYPE_MASK;
    }


    /** Sets the Output's type.
     */
    void setType(uint8_t aType)
    {
        type = (type & OUTPUT_STATE_MASK) | (aType & OUTPUT_TYPE_MASK);
    }


    /** Gets; the Output's state.
     *  Hi - Non zero.
     *  Lo - zero.
     */
    uint8_t getState()
    {
        return type & OUTPUT_STATE_MASK;
    }


    /** Sets the Output's state.
     *  Hi - Non zero.
     *  Lo - zero.
     */
    void setState(uint8_t aState)
    {
        type = (aState ? OUTPUT_STATE_MASK : 0) | (type & OUTPUT_TYPE_MASK);
    }

    
    /** Gets the Output's Lo value.
     */
    uint8_t getLo()
    {
        return lo;
    }


    /** Sets the Output's Lo value.
     */
    void setLo(uint8_t aLo)
    {
        lo = aLo;
    }
    

    /** Gets the Output's Hi value.
     */
    uint8_t getHi()
    {
        return hi;
    }


    /** Sets the Output's Hi value.
     */
    void setHi(uint8_t aHi)
    {
        hi = aHi;
    }
    

    /** Gets the Output's pace.
     */
    uint8_t getPace()
    {
        return (pace >> OUTPUT_PACE_SHIFT) & OUTPUT_PACE_MASK;
    }


    /** Sets the Output's pace.
     */
    void setPace(uint8_t aPace)
    {
        pace = ((aPace & OUTPUT_PACE_MASK) << OUTPUT_PACE_SHIFT) | (pace & OUTPUT_DELAY_MASK);
    }
    
    
    /** Gets the Output's type.
     */
    uint8_t getDelay()
    {
        return pace & OUTPUT_DELAY_MASK;
    }


    /** Set's the Output's delay.
     */
    void setDelay(uint8_t aDelay)
    {
        pace = (aDelay & OUTPUT_DELAY_MASK) | (pace & (OUTPUT_PACE_MASK << OUTPUT_PACE_SHIFT));
    }
};


#ifdef OUTPUT_BASE      // Methods for loading/saving outputs to/from EEPROM in the OutputModule.


// The Outputs' data in RAM.
OutputDef outputDefs[OUTPUT_PIN_MAX];


/** Load an Output's definition from EEPROM.
 */
void loadOutput(int aPin)
{
    EEPROM.get(OUTPUT_BASE + aPin * sizeof(OutputDef), outputDefs[aPin]); 
    outputDefs[aPin].printDef("Load", aPin);
}


/** Save an Output's definition to EEPROM.
 */
void saveOutput(int aPin)
{
    if (aPin < OUTPUT_PIN_MAX)
    {
        EEPROM.put(OUTPUT_BASE + aPin * sizeof(OutputDef), outputDefs[aPin]);
        outputDefs[aPin].printDef("Save", aPin);
    }
}


/** Is the given Output type a servo type?
 *  ie: SERVO or SIGNAL.
 */
boolean isServo(uint8_t aType)
{
    return    (aType == OUTPUT_TYPE_SERVO)
           || (aType == OUTPUT_TYPE_SIGNAL);
}


#else   // Methods for loading/saving outputs to/from EEPROM in the OutputModule.


/** Variables for working with an Output.
 */
int        outputNodes  = 0;    // Bit map of Output nodes present.
uint8_t    outputNode   = 0;    // Current Output node.
uint8_t    outputPin    = 0;    // Current Output pin.
OutputDef  outputDef;           // Definition of current Output.

uint8_t    outputStates[OUTPUT_NODE_MAX];   // State of all the attached output module's Outputs.


/** Read an Output's data from an OutputModule.
 */
void readOutput(uint8_t aNode, uint8_t aPin);


/** Read an Output's data from an OutputModule.
 */
void readOutput(uint8_t aOutputNumber);


/** Write an Output's data to an OutputModule.
 *  And save it if so requested.
 */
void writeOutput(boolean aSave);


/** Write a change of state to the Output module.
 */
void writeOutputState(boolean aState, uint8_t aDelay);


/** Reset current Output. 
 *  And then reload its definition.
 */
void resetOutput();


/** Read the states of the given node's Outputs.
 *  Save in OutputStates.
 *  If fails, return a character indicating the error.
 */
char readOutputStates(uint8_t aNode);


/** Record the presence of an OutputNode in the map.
 */
void setOutputNodePresent(int aNode)
{
    outputNodes |= (1 << aNode); 
}


/** Gets the state of the given Output's given pin.
 */
boolean getOutputState(uint8_t aNode, uint8_t aPin)
{
    return outputStates[aNode] & (1 << aPin);
}


/** Sets the states of all the given node's Outputs.
 */
void setOutputStates(uint8_t aNode, uint8_t aStates)
{
    outputStates[aNode] = aStates;
}


/** Sets the state of the given node's Output pin.
 */
void setOutputState(uint8_t aNode, uint8_t aPin, boolean aState)
{
    uint8_t mask = 1 << aPin;
    
    outputStates[aNode] &= ~mask;
    if (aState)
    {
        outputStates[aNode] |= mask;
    }
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


#endif


#endif
