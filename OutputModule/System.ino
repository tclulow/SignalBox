/** System data.
 */


/** Load SystemData from EEPROM.
 *  Return true if valid, else don't load and return false.
 */
boolean loadSystemData()
{
    EEPROM.get(SYSTEM_BASE, systemData.magic);  // Check the magic number

    if (systemData.magic == MAGIC_NUMBER)
    {
        EEPROM.get(SYSTEM_BASE, systemData);
    }

    return systemData.magic == MAGIC_NUMBER;
}


/** Save SystemData.
 */
void saveSystemData()
{
    EEPROM.put(SYSTEM_BASE, systemData);

    debugSystemData();
}


void debugSystemData()
{
    if (isDebug(DEBUG_BRIEF))
    {
        Serial.print(millis());
        Serial.print(CHAR_TAB);
        Serial.print(PGMT(M_DEBUG_SYSTEM));
        Serial.print(CHAR_SPACE);
        Serial.print(PGMT(M_DEBUG_I2C));
        Serial.print(CHAR_SPACE);
        Serial.print(systemData.i2cControllerID, HEX);
        Serial.print(CHAR_SPACE);
        Serial.print(systemData.i2cInputBaseID,  HEX);
        Serial.print(CHAR_SPACE);
        Serial.print(systemData.i2cOutputBaseID, HEX);
        Serial.print(CHAR_SPACE);
        Serial.print(systemData.i2cModuleID,     HEX);

        Serial.print(CHAR_SPACE);
        Serial.print(PGMT(M_DEBUG_DEBUG));
        Serial.print(CHAR_SPACE);
        Serial.print(systemData.debugLevel,      HEX);
        Serial.print(CHAR_SPACE);
        Serial.print(PGMT(M_DEBUG_REPORT));
        Serial.print(CHAR_SPACE);
        Serial.print(systemData.reportLevel,     HEX);
        Serial.println();

        if (isDebug(DEBUG_FULL))
        {
            dumpMemory();
        }
    }
}


/** Is debugging enabled at this level?
 */
boolean isDebug(uint8_t aLevel)
{
    return systemData.debugLevel >= aLevel;
}


/** Gets the debugging level.
 */
uint8_t getDebug()
{
    return systemData.debugLevel;
}


/** Sets the debugging level.
 */
void setDebug(uint8_t aLevel)
{
    systemData.debugLevel = aLevel;
}


/** Show version number by flashing LED
 *  and reporting it on Serial output.
 */
void flashVersion()
{
    pinMode(LED_BUILTIN, OUTPUT);   // Configure the on-board LED pin for output
    
    for (int ind = 0; ind < strlen_P(M_VERSION); ind++)
    {
        char ch = pgm_read_byte_near(M_VERSION + ind);
        if (ch >= CHAR_ZERO && ch <= CHAR_NINE)
        {
            while (ch-- > CHAR_ZERO)
            {
                digitalWrite(LED_BUILTIN, HIGH);
                delay(DELAY_BLINK);
                digitalWrite(LED_BUILTIN, LOW);
                delay(DELAY_BLINK);
            }
        }
        else
        {
            delay(DELAY_BLINK * 2);
        }
    }

    if (isDebug(DEBUG_NONE))
    {
        Serial.print(PGMT(M_SOFTWARE));
        Serial.print(CHAR_SPACE);
        Serial.print(PGMT(M_VERSION));
        Serial.print(CHAR_SPACE);
        Serial.print(PGMT(M_VERSION_DATE));
        Serial.println();
    }
    
    #if !MASTER
    // Flash module number.
    delay(DELAY_BLINK * 3);
    for (uint8_t mask = 1; mask <= 0x08; mask <<= 1)
    {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(DELAY_BLINK * (getModuleId(false) & mask ? 3 : 1));
        digitalWrite(LED_BUILTIN, LOW);
        delay(DELAY_BLINK);
    }
    #endif
}


#if MASTER

/** Report a system failure.
 */
void systemFail(PGM_P aMessage, int aValue, int aDelay)
{
    Serial.print(PGMT(M_FAILURE));
    Serial.print(CHAR_TAB);
    Serial.print(PGMT(aMessage));
    Serial.print(CHAR_SPACE);
    Serial.print(aValue, HEX);
    Serial.println();
    
    lcd.clear();
    lcd.printAt(LCD_COL_START, LCD_ROW_TOP, M_FAILURE);
    lcd.printAt(LCD_COL_START, LCD_ROW_BOT, aMessage);
    lcd.printAtHex(LCD_COLS - 2,  LCD_ROW_BOT, aValue, 2);

    if (aDelay > 0)
    {
        delay(aDelay);
    }
    else
    {
        waitForButton();
        lcd.clear();
        waitForButtonRelease();
    }
}


/** Is an EzyBus setup detected?
 */
boolean ezyBusDetected()
{
    return EEPROM.read(EZY_MAGIC_ADDR) == EZY_MAGIC;
}


/** Make sure Ezybus won't recognise the (Panel) setup.
 */
void ezyBusClear()
{
    if (ezyBusDetected())
    {
        EEPROM.put(EZY_MAGIC_ADDR, EZY_MAGIC + 1);      // Corrupt the EzyBus magic number slightly.
    }
}


#else

/** Gets the output module ID.
 *  Either by hardware jumperss of from EEPROM.
 */
uint8_t getModuleId(boolean aIncludeBase)
{
    uint8_t moduleId = systemData.i2cModuleID;

    if (moduleId > OUTPUT_NODE_MAX)
    {
        // Configure i2c from jumpers.
        moduleId = 0;
        for (int pin = 0, mask=1; pin < JUMPER_PINS; pin++, mask <<= 1)
        {
            if (   (   (jumperPins[pin] >= ANALOG_PIN_FIRST)
                    && (analogRead(jumperPins[pin]) > ANALOG_PIN_CUTOFF))
                || (   (jumperPins[pin] <  ANALOG_PIN_FIRST)
                    && (false)      // TODO - handle digital pins on TxRx
                    && (digitalRead(jumperPins[pin]))))
            {
                moduleId |= mask;
            }
//            Serial.print(millis());
//            Serial.print(CHAR_TAB);
//            Serial.print("Jumper ");
//            Serial.print(jumperPins[pin], HEX);
//            Serial.print(": digital=");
//            Serial.print(digitalRead(jumperPins[pin]), HEX);
//            Serial.print(", analog=");
//            Serial.print(analogRead(jumperPins[pin]), HEX);
//            Serial.print(". ID=");
//            Serial.print(systemData.i2cModuleId, HEX);
//            Serial.println();
        }
    }

    // Announce module ID
    if (   (isDebug(DEBUG_BRIEF))
        && (aIncludeBase))
    {
        Serial.print(millis());
        Serial.print(CHAR_TAB);
        Serial.print(PGMT(M_DEBUG_MODULE));
        Serial.print(CHAR_SPACE);
        Serial.print(systemData.i2cOutputBaseID + moduleId, HEX);
        Serial.println();
    }

    return (aIncludeBase ? systemData.i2cOutputBaseID : 0) + moduleId;
}


#endif

/** Dump a range of the EEPROM memory.
 */
void dumpMemory(PGM_P aMessage, int aStart, int aEnd)
{
    Serial.print(CHAR_HASH);
    Serial.print(CHAR_SPACE);
    Serial.println(PGMT(aMessage));
    
    for (int base = aStart; base < aEnd; base += 16)
    {
        Serial.print(CHAR_HASH);
        Serial.print(CHAR_SPACE);
        printHex(base, 4);
        Serial.print(CHAR_COLON);
        
        for (int offs = 0; offs < 16; offs++)
        {
            Serial.print(CHAR_SPACE);
            printHex(EEPROM.read(base + offs), 2);
        }

        Serial.print(CHAR_SPACE);
        Serial.print(CHAR_SPACE);
        Serial.print(CHAR_SPACE);

        for (int offs = 0; offs < 16; offs++)
        {
            char ch = EEPROM.read(base + offs);
            if (   (ch >= CHAR_SPACE)
                && (ch <= CHAR_TILDE))
            {
                Serial.print(ch);    
            }
            else
            {
                Serial.print(CHAR_DOT);
            }
        }

        Serial.println();
    }
}


/** Dump all the EEPROM memory.
 */
void dumpMemory()
{
    dumpMemory(M_SYSTEM, SYSTEM_BASE, SYSTEM_END);
    Serial.println();
#if OUTPUT_BASE
    dumpMemory(M_OUTPUT, OUTPUT_BASE, OUTPUT_END);
    Serial.println();
#endif
#if TYPES_BASE
    dumpMemory(M_TYPES,  TYPES_BASE,  TYPES_END);
    Serial.println();
#endif
#if INPUT_BASE
    dumpMemory(M_INPUT,  INPUT_BASE,  INPUT_END);
    Serial.println();
#endif
}
