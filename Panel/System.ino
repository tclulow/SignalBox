/** System data.
 */


/** Load SystemData from EEPROM
 *  Return true if valid
 */
boolean loadSystemData()
{
    EEPROM.get(SYSTEM_BASE, systemData);
    return systemData.magic == MAGIC_NUMBER;
}


/** Save SystemData.
 */
void saveSystemData()
{
    EEPROM.put(SYSTEM_BASE, systemData);
}


/** Is debugging enabled at this level?
 */
boolean isDebug(uint8_t aLevel)
{
    return systemData.debugLevel >= aLevel;
}


/** Sets the debugging level.
 */
void setDebug(uint8_t aLevel)
{
    systemData.debugLevel = aLevel;
    saveSystemData();
}


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

        Serial.println();
    }
}


/** Dump all the EEPROM memory.
 */
void dumpMemory()
{
    dumpMemory(M_SYSTEM, SYSTEM_BASE, SYSTEM_END);
    Serial.println();
    dumpMemory(M_INPUT,  INPUT_BASE,  INPUT_END);
    Serial.println();
    dumpMemory(M_TYPES,  TYPES_BASE,  TYPES_END);
    Serial.println();
}
