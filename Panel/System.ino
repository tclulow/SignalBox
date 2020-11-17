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


/** Report a system failure.
 */
void systemFail(PGM_P aMessage, int aValue, int aDelay)
{
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
