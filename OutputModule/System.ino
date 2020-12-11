/** System.
 */


/** Load SystemData from EEPROM
 *  Return true if valid
 */
boolean loadSystemData()
{
    EEPROM.get(SYSTEM_BASE, systemData);
    // systemData.i2cModuleID = I2C_MODULE_ID_JUMPERS;
    return systemData.magic != MAGIC_NUMBER;
}


/** Save SystemData.
 */
void saveSystemData()
{
    EEPROM.put(SYSTEM_BASE, systemData);
}


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
        Serial.print(PGMT(M_MODULE_ID));
        Serial.print(CHAR_SPACE);
        Serial.print(systemData.i2cOutputBaseID + moduleId, HEX);
        Serial.println();
    }

    return (aIncludeBase ? systemData.i2cOutputBaseID : 0) + moduleId;
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
