/** Input data.
 */
#ifndef _Debug_h
#define _Debug_h

#define DEBUG_OFF       0      // No debugging messages.
#define DEBUG_LOW       1      // Debugging messages - low level.
#define DEBUG_HIGH      2      // Debugging messages - high level.
#define DEBUG_PAUSE     3      // Debugging messages with pause for acknowledgement.
#define DEBUG_MAX       4      // Maximum debug option.


/** Is debugging enabled (at a particular level)?
 */
boolean debugEnabled(int aLevel)
{
  return aLevel <= systemData.debugLevel;
}


/** Pause for user-input if so configured.
 */
void debugPause()
{
  if (systemData.debugLevel >= DEBUG_PAUSE)
  {
    switch (waitForButton())
    {
      case BUTTON_NONE:   break;
      case BUTTON_UP:     break;
      case BUTTON_DOWN:   break;
      case BUTTON_SELECT: break;
      case BUTTON_LEFT:   systemData.debugLevel = DEBUG_OFF;
                          saveSystemData();
                          break;
      case BUTTON_RIGHT:  break;
    }

    waitForButtonRelease();
  }
}


#endif
