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
