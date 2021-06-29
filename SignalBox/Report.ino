/** Reporting.
 *
 *
 *  (c)Copyright Tony Clulow  2021    tony.clulow@pentadtech.com
 *
 *  This work is licensed under the:
 *      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 *      http://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 *  For commercial use, please contact the original copyright holder(s) to agree licensing terms.
 */

#include "All.h"


/** Is reporting enabled (at a particular level)?
 */
boolean isReportEnabled(uint8_t aLevel)
{
    return aLevel <= systemData.reportLevel;
}


/** Length of time to wait for depending on the reporting level.
 */
int getReportDelay()
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
        switch (waitForButtonPress())
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
        disp.clearRow(LCD_COL_START, LCD_ROW_BOT);
        disp.printProgStrAt(LCD_COL_START,  LCD_ROW_BOT, M_REPORT);
        disp.printProgStrAt(LCD_COL_REPORT_PARAM, LCD_ROW_BOT, M_REPORT_PROMPTS[systemData.reportLevel], LCD_LEN_OPTION);

        waitForButtonRelease();
        disp.clearRow(LCD_COL_START, LCD_ROW_BOT);
    }
}
