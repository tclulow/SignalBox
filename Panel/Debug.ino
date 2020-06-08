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
                          announce();
                          break;
      case BUTTON_RIGHT:  break;
    }

    waitForButtonRelease();
  }
}
