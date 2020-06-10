/** System data.
 */
#ifndef _System_h
#define _System_h

// System Data saved in EEPROM
#define SYSTEM_BASE  INPUT_END                    // EEPROM base of System data.
#define SYSTEM_SIZE  sizeof(SystemData)           // Size of System Data.
#define SYSTEM_END   (SYSTEM_BASE + SYSTEM_SIZE)  // End of System EEPROM.

#define MAGIC_NUMBER 0x50616f6e                   // Majic number.
#define VERSION      0x0001                       // Version number of software.


/** Data describing an Output's operation.
 */
struct SystemData
{
  public:
  
  long    magic           = 0;    // Magic number to identify software.
  long    version         = 0;    // Software version number to identify upgrades.

  uint8_t i2cControllerID = 0;    // I2C node IDs.
  uint8_t i2cInputBaseID  = 0;
  uint8_t i2cOutputBaseID = 0;
  int8_t  debugLevel      = 0;    // Debug messages.

  int     buttons[6];             // Configuration of analog buttons.
  
  char    rfu[8]          = "RFU rfu";                 // RFU. 32 bytes in all.
};


/** An instance of the singleton.
 */
SystemData systemData;


/** Load SystemData from EEPROM
 *  Return true if valid
 */
boolean loadSystemData();


/** Save SystemData.
 */
void saveSystemData();


/** Report a system failure.
 */
void systemFail(PGM_P aMessage, int aValue)
{
  lcd.clear();
  lcd.printAt(LCD_COL_START, LCD_ROW_TOP, M_FAILURE);
  lcd.printAt(LCD_COL_START, LCD_ROW_BOT, aMessage);
  lcd.printAtHex(LCD_COLS - 2,  LCD_ROW_BOT, aValue, 2);
  
  waitForButton();
  lcd.clear();
  waitForButtonRelease();
}


#endif
