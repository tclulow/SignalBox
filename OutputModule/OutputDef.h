/** Output definition.
 *  @file
 *
 *  (c)Copyright Tony Clulow  2021    tony.clulow@pentadtech.com
 *
 *  This work is licensed under the:
 *      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 *      http://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 *  For commercial use, please contact the original copyright holder(s) to agree licensing terms.
 */

#ifndef OutputDef_h
#define OutputDef_h


// Output nodes.
#define OUTPUT_PIN_MAX              8   // 8 outputs to each node.
#define OUTPUT_PIN_MASK             7   // 3 bits for 8 pins withing an output node.
#define OUTPUT_NODE_MAX            32   // Maximum nodes.
#define OUTPUT_NODE_HALF           16   // Half the maximum nodes.
#define OUTPUT_NODE_MASK         0x1f   // 5 bits for 32 nodes.
#define OUTPUT_NODE_SHIFT           3   // Shift output number this amount to get a node number.

// Output options maxima.
#define OUTPUT_SERVO_MAX          180   // Maximum value an angle output parameter can take.
#define OUTPUT_LED_MAX            255   // Maximum value a LED can take.
#define OUTPUT_HI_LO_SIZE           3   // Maximum digits in a Hi/Lo display.

// Masks for type, state and pace options within the outputDef.
#define OUTPUT_STATE_MASK        0x80   // On or off, switched or not switched, 0 = lo, 1 = hi.
#define OUTPUT_TYPE_MASK         0x0f   // Output type mask (4 bits).
#define OUTPUT_PACE_MASK         0x0f   // Pace is 4 bits.
#define OUTPUT_PACE_SHIFT           4   // Pace is multiplied by 16 (shifted left 4 bits).

// Masks for locks.
#define OUTPUT_LOCK_MAX             4   // Four locks of each type (Hi/Lo).

// Wire response message lengths.
#define OUTPUT_MOVE_LOCK_LEN        2   // Two bytes used to move a node's locks.

// Defaults when initialising.
#define OUTPUT_DEFAULT_LO          90   // Default low  position is 90 degrees.
#define OUTPUT_DEFAULT_HI          90   // Default high position is 90 degrees.
#define OUTPUT_DEFAULT_PACE       0xc   // Default pace is mid-range.
#define OUTPUT_DEFAULT_RESET      0x0   // Default reset is none.

// Output types.
#define OUTPUT_TYPE_NONE         0x00   // Placeholder to mark "no type".
#define OUTPUT_TYPE_SERVO        0x01   // Output is a servo.
#define OUTPUT_TYPE_SIGNAL       0x02   // Output is a signal.
#define OUTPUT_TYPE_LED          0x03   // Output is a LED or other IO device.
#define OUTPUT_TYPE_LED_4        0x04   // Output is a LED 4-aspect paired with next output.
#define OUTPUT_TYPE_ROAD_UK      0x05   // Output is a LED 3-aspect UK road signal.
#define OUTPUT_TYPE_ROAD_RW      0x06   // Output is a LED 3-aspect (Rest of World) road signal.
#define OUTPUT_TYPE_FLASH        0x07   // Output is a flashing LED.
#define OUTPUT_TYPE_BLINK        0x08   // Output is a blinking LED.
#define OUTPUT_TYPE_RANDOM       0x09   // Output is a random LED.
#define OUTPUT_TYPE_MAX          0x0A   // Limit of output types.


 /** Definition of an Output.
 */
class OutputDef
{
    private:

    uint8_t type  = 0;                  // The type of the output, see OUTPUT_TYPE_...
    uint8_t lo    = 0;                  // The Output's Lo setting.
    uint8_t hi    = 0;                  // The Output's Hi setting.
    uint8_t pace  = 0;                  // The pace at which the output moves.
    uint8_t reset = 0;                  // The reset interval for the Output.

    uint8_t locks = 0;                  // The enabled locks.
    uint8_t lockLo[OUTPUT_LOCK_MAX];    // Outputs that lock this output Lo.
    uint8_t lockHi[OUTPUT_LOCK_MAX];    // Outputs that lock this output Hi.
    uint8_t lockState = 0;              // Lock is against output being Hi (else Lo).


    public:

    /** Is the Output one of the servo type?
     */
    bool isServo()
    {
        return    (getType() == OUTPUT_TYPE_SERVO)
               || (getType() == OUTPUT_TYPE_SIGNAL);
    }


    /** Is the Output one of the LED types?
     */
    bool isLed()
    {
        return    getType() == OUTPUT_TYPE_LED
               || getType() == OUTPUT_TYPE_LED_4
               || getType() == OUTPUT_TYPE_ROAD_UK
               || getType() == OUTPUT_TYPE_ROAD_RW;
    }


    /** Is the Output one of the flashing types?
     */
    bool isFlasher()
    {
        return    (getType() == OUTPUT_TYPE_FLASH)
               || (getType() == OUTPUT_TYPE_BLINK);
    }


    /** Is the output the RANDOM type
     */
    bool isRandom()
    {
        return getType() == OUTPUT_TYPE_RANDOM;
    }


    /** Is the output a PWM type?
     *  ie it uses PWM to control its intensity.
     */
    bool isPwm()
    {
        return    isLed()
               || isFlasher()
               || isRandom();
    }


    /** Gets the pace as steps.
     *  From 0 -> f to 240 -> 0.
     */
    uint8_t getPaceAsSteps()
    {
        return (OUTPUT_PACE_MASK - getPace()) << OUTPUT_PACE_SHIFT;
    }


    /** Set all an Output's data.
     */
    void set(uint8_t aType, bool aState, uint8_t aLo, uint8_t aHi, uint8_t aPace, uint8_t aReset)
    {
        setType(aType);
        setState(aState);
        setLo(aLo);
        setHi(aHi);
        setPace(aPace);
        setReset(aReset);
    }


    /** Clear all locks.
     */
    void clearLocks()
    {
        locks = 0;
    }


    /** Set the indicated lock active (or not).
     */
    void setLock(bool aHi, uint8_t aIndex, bool aState)
    {
        if (aState)
        {
            locks |=  (1 << (aIndex + (aHi ? OUTPUT_LOCK_MAX : 0)));
        }
        else
        {
            locks &= ~(1 << (aIndex + (aHi ? OUTPUT_LOCK_MAX : 0)));
        }
    }


    /** Is the given lock defined?
     */
    bool isLock(bool aHi, uint8_t aIndex)
    {
        return (locks & (1 << (aIndex + (aHi ? OUTPUT_LOCK_MAX : 0)))) != 0;
    }


    /** Get the selected lock's node.
     */
    uint8_t getLockNode(bool aHi, uint8_t aIndex)
    {
        uint8_t node = aHi ? lockHi[aIndex] : lockLo[aIndex];

        return (node >> OUTPUT_NODE_SHIFT) & OUTPUT_NODE_MASK;
    }


    /** Set the selected lock's node.
     */
    void setLockNode(bool aHi, uint8_t aIndex, uint8_t aNode)
    {
        if (aHi)
        {
            lockHi[aIndex] = (lockHi[aIndex] & ~(OUTPUT_NODE_MASK << OUTPUT_NODE_SHIFT)) | ((aNode & OUTPUT_NODE_MASK) << OUTPUT_NODE_SHIFT);
        }
        else
        {
            lockLo[aIndex] = (lockLo[aIndex] & ~(OUTPUT_NODE_MASK << OUTPUT_NODE_SHIFT)) | ((aNode & OUTPUT_NODE_MASK) << OUTPUT_NODE_SHIFT);
        }
    }


    /** Get the selected lock's pin.
     */
    uint8_t getLockPin(bool aHi, uint8_t aIndex)
    {
        uint8_t pin = aHi ? lockHi[aIndex] : lockLo[aIndex];

        return pin & OUTPUT_PIN_MASK;
    }


    /** Set the selected lock's pin.
     */
    void setLockPin(bool aHi, uint8_t aIndex, uint8_t aPin)
    {
        if (aHi)
        {
            lockHi[aIndex] = (lockHi[aIndex] & ~OUTPUT_PIN_MASK) | (aPin & OUTPUT_PIN_MASK);
        }
        else
        {
            lockLo[aIndex] = (lockLo[aIndex] & ~OUTPUT_PIN_MASK) | (aPin & OUTPUT_PIN_MASK);
        }
    }


    /** Get the selected lock's lock state.
     *  That's the state the other output must be in to enforce this lock.
     *  return true = Hi, false = Lo.
     */
    bool getLockState(bool aHi, uint8_t aIndex)
    {
        return (lockState & (1 << (aIndex + (aHi ? OUTPUT_LOCK_MAX : 0)))) != 0;
    }


    /** Get the selected locks lock state.
     *  That's the state the other output must be in to enforce this lock.
     */
    void setLockState(bool aHi, uint8_t aIndex, bool aState)
    {
        if (aState)
        {
            lockState |=  (1 << (aIndex + (aHi ? OUTPUT_LOCK_MAX : 0)));
        }
        else
        {
            lockState &= ~(1 << (aIndex + (aHi ? OUTPUT_LOCK_MAX : 0)));
        }
    }


    /** How many locks are defined for the output.
     */
    uint8_t getLockCount(bool aHi)
    {
        uint8_t count = 0;

        for (uint8_t index = 0; index < OUTPUT_LOCK_MAX; index++)
        {
            if (isLock(aHi, index))
            {
                count+= 1;
            }
        }

        return count;
    }


    /** Write an Output down the I2C bus.
     *  Must be the same order as read().
     */
    void write()
    {
        i2cComms.sendByte(type);
        i2cComms.sendByte(lo);
        i2cComms.sendByte(hi);
        i2cComms.sendByte(pace);
        i2cComms.sendByte(reset);

        i2cComms.sendByte(locks);
        for (uint8_t index = 0; index < OUTPUT_LOCK_MAX; index++)
        {
            i2cComms.sendByte(lockLo[index]);
            i2cComms.sendByte(lockHi[index]);
        }
        i2cComms.sendByte(lockState);
    }


    /** Read an Output from the I2C bus.
     *  Must be the same order as write().
     */
    void read()
    {
        type  = i2cComms.readByte();
        lo    = i2cComms.readByte();
        hi    = i2cComms.readByte();
        pace  = i2cComms.readByte();
        reset = i2cComms.readByte();

        locks = i2cComms.readByte();
        for (uint8_t index = 0; index < OUTPUT_LOCK_MAX; index++)
        {
            lockLo[index] = i2cComms.readByte();
            lockHi[index] = i2cComms.readByte();
        }
        lockState = i2cComms.readByte();
    }


    /** Prints an Output's definition.
     *  Only when debug level is high enough (at callers discretion).
     */
    void printDef(PGM_P aHeader, uint8_t aNode, uint8_t aPin)
    {
        Serial.print(millis());
        Serial.print(CHAR_TAB);
        Serial.print(PGMT(aHeader));
        Serial.print(HEX_CHARS[aNode & OUTPUT_NODE_MASK]);
        Serial.print(aPin);
        Serial.print(PGMT(M_DEBUG_TYPE));
        Serial.print(PGMT(M_OUTPUT_TYPES[getType() & OUTPUT_TYPE_MASK]));
        Serial.print(PGMT(M_DEBUG_STATE));
        Serial.print(PGMT(getState() ? M_HI : M_LO));
        Serial.print(PGMT(M_DEBUG_LO));
        Serial.print(getLo(),    HEX);
        Serial.print(PGMT(M_DEBUG_HI));
        Serial.print(getHi(),    HEX);
        Serial.print(PGMT(M_DEBUG_PACE));
        Serial.print(getPace(),  HEX);
        Serial.print(PGMT(M_DEBUG_RESET_AT));
        Serial.print(getReset(), HEX);

        for (uint8_t hi = 0; hi < 2; hi++)
        {
            Serial.print(PGMT(hi ? M_DEBUG_HI : M_DEBUG_LOCK_LO));
            for (uint8_t index = 0; index < OUTPUT_LOCK_MAX; index++)
            {
                if (isLock(hi, index))
                {
                    Serial.print(getLockState(hi, index) ? '^' : 'v');
                    Serial.print(getLockNode(hi, index), HEX);
                    Serial.print(getLockPin(hi, index), HEX);
                }
                else
                {
                    Serial.print(CHAR_SPACE);
                    Serial.print(CHAR_DOT);
                    Serial.print(CHAR_DOT);
                }
                Serial.print(CHAR_SPACE);
            }
        }

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


    /** Gets the Output's state.
     *  Hi - true.
     *  Lo - false.
     */
    bool getState()
    {
        return (type & OUTPUT_STATE_MASK) != 0;
    }


    /** Sets the Output's state.
     *  Hi - Non zero.
     *  Lo - zero.
     */
    void setState(bool aState)
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
        return pace & OUTPUT_PACE_MASK;
    }


    /** Sets the Output's pace.
     */
    void setPace(uint8_t aPace)
    {
        pace = aPace & OUTPUT_PACE_MASK;
    }


    /** Gets the Output's type.
     */
    uint8_t getReset()
    {
        return reset;
    }


    /** Set's the Output's reset time.
     */
    void setReset(uint8_t aReset)
    {
        reset = aReset;
    }


    /** Gets the reset value as a character.
     *  Space if zero.
     *  Hex character if 0x1 - 0xf.
     *  Hash symbol if greater.
     */
    char getResetCh()
    {
        char ch = CHAR_SPACE;

        if (getReset() > 0)
        {
            if (getReset() < 0x10)
            {
                ch = HEX_CHARS[reset];
            }
            else
            {
                ch = CHAR_HASH;
            }
        }

        return ch;
    }
};


#endif
