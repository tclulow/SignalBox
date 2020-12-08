/** Is reporting enabled (at a particular level)?
 */
boolean reportEnabled(int aLevel)
{
    return aLevel <= systemData.reportLevel;
}


/** Length of time to wait for depending on the reporting level.
 */
int reportDelay()
{
    return DELAY_READ * systemData.reportLevel;    
}


/** Pause for user-input if so configured.
 *  Use buttons to adjust report level.
 */
void reportPause()
{
    if (systemData.reportLevel >= REPORT_PAUSE)
    {
        switch (waitForButton())
        {
            case BUTTON_NONE:   break;
            case BUTTON_UP:     systemData.reportLevel = REPORT_LONG;
                                saveSystemData();
                                break;
            case BUTTON_DOWN:   systemData.reportLevel = REPORT_SHORT;
                                saveSystemData();
                                break;
            case BUTTON_LEFT:   systemData.reportLevel = 0;
                                saveSystemData();
                                break;
            case BUTTON_RIGHT:  configure.run();
                                break;
            case BUTTON_SELECT: break;
        }

        // Show (new) report level.
        lcd.clearRow(LCD_COL_START, LCD_ROW_BOT);
        lcd.printAt(LCD_COL_START,  LCD_ROW_BOT, M_REPORT);
        lcd.printAt(LCD_COL_REPORT_PARAM, LCD_ROW_BOT, M_REPORT_PROMPTS[systemData.reportLevel], LCD_LEN_OPTION);

        waitForButtonRelease();
    }
}
