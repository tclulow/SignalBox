/** System.
 */


/** Load SystemData from EEPROM
 *  Return true if valid
 */
boolean loadSystemData()
{
    EEPROM.get(SYSTEM_BASE, systemData);
    systemData.debugLevel = DEBUG_BRIEF;     // TODO - remove this.
    return systemData.magic != MAGIC_NUMBER;
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
