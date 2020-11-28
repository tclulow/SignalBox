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

// Masks for type, state, pace and delay options withing the outputDef element.
#define OUTPUT_STATE_MASK      0x80   // On or off, switched or not switched, 0 = lo, 1 = hi.    Was OUTPUT_STATE
#define OUTPUT_TYPE_MASK       0x0f   // Output type mask (4 bits).
#define OUTPUT_PACE_SHIFT         4   // Pace is in the left-most nibble.
#define OUTPUT_PACE_MASK       0x0f   // Pace is 4 bits.
#define OUTPUT_PACE_MULT          3   // Pace is shifted by this amount (multiplied by 8).
#define OUTPUT_PACE_OFFSET        4   // Pace is offset by this amount (add 4).
#define OUTPUT_DELAY_MASK      0x0f   // Delay will be right-most nibble of output.pace.

// Defaults when initialising
#define OUTPUT_DEFAULT_LO        90   // Default low  position is 90 degrees.
#define OUTPUT_DEFAULT_HI        90   // Default high position is 90 degrees.
#define OUTPUT_DEFAULT_PACE     0xc   // Default pace is mid-range.
#define OUTPUT_DEFAULT_DELAY    0x0   // Default delay is none.


// Output information that's shared with the output module.
#define OUTPUT_TYPE_SERVO      0x00   // Output is a servo.
#define OUTPUT_TYPE_SIGNAL     0x01   // Output is a signal.
#define OUTPUT_TYPE_LED        0x02   // Output is a LED or other IO device.
#define OUTPUT_TYPE_FLASH      0x03   // Output is a flashing LED.
#define OUTPUT_TYPE_BLINK      0x04   // Output is a blinking LED.
#define OUTPUT_TYPE_MAX        0x05   // Limit of output types.
#define OUTPUT_TYPE_NONE       0x0f   // Placeholder to mark "no type".




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
    boolean isALed()
    {
        return    (getType() == OUTPUT_TYPE_LED)
               || (getType() == OUTPUT_TYPE_FLASH)
               || (getType() == OUTPUT_TYPE_BLINK);
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


    /** Gets the pace as adjusted from 0-f to 4-124.
     */
    uint8_t getAdjustedPace()
    {
        Serial.print("Adjusted pace=");
        Serial.print(getPace(),HEX);
        Serial.print(", shifted=");
        Serial.print(getPace() << OUTPUT_PACE_MULT, HEX);
        Serial.print(", adjusted=");
        Serial.print((getPace() << OUTPUT_PACE_MULT) + OUTPUT_PACE_OFFSET, HEX);
        Serial.println();
        return (getPace() << OUTPUT_PACE_MULT) + OUTPUT_PACE_OFFSET;
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


    /** Prints an Output's definition.
     */
    printDef(char* aHeader, uint8_t aPin)
    {
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
    

    /** Sets the Output's state.
     *  Hi - Non zero.
     *  Lo - zero.
     */
    void setState(uint8_t aState)
    {
        type = (aState ? OUTPUT_STATE_MASK : 0) | (type & OUTPUT_TYPE_MASK);
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


// Data for loading/saving outputs to/from EEPROM.
#ifdef OUTPUT_BASE

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

#endif

#endif
