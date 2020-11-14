/** Is debugging enabled (at a particular level)?
 */
boolean debugEnabled(int aLevel)
{
  return aLevel <= systemData.debugLevel;
}


/** Pause for user-input if so configured.
 *  Use buttons to adjust debug level.
 */
void debugPause()
{
  if (systemData.debugLevel >= DEBUG_PAUSE)
  {
    switch (waitForButton())
    {
      case BUTTON_NONE:   break;
      case BUTTON_UP:     systemData.debugLevel += 1;
                          if (systemData.debugLevel >= DEBUG_MAX)
                          {
                            systemData.debugLevel = DEBUG_MAX - 1;
                          }
                          saveSystemData();
                          break;
      case BUTTON_DOWN:   systemData.debugLevel -= 1;
                          saveSystemData();
                          break;
      case BUTTON_LEFT:   systemData.debugLevel = 0;
                          saveSystemData();
                          announce();
                          break;
      case BUTTON_RIGHT:  configure.run();
                          break;
      case BUTTON_SELECT: break;
    }

    waitForButtonRelease();
  }
}
