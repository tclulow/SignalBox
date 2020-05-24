/** Buttons
 */
#ifndef _Buttons_h
#define _Buttons_h


// The analog button
#define BUTTON_ANALOG 0


// Button values.
#define BUTTON_NONE   0
#define BUTTON_RIGHT  1
#define BUTTON_UP     2
#define BUTTON_DOWN   3
#define BUTTON_LEFT   4
#define BUTTON_SELECT 5


/** Read the input button pressed.
 *  Return one of the constants above.
 */
int readButton()
{
  int value = analogRead(BUTTON_ANALOG);

  #if DEBUG
  static int previous = 0;
  if (value != previous)
  {
    previous = value;
    Serial.print(millis());
    Serial.print(" ");
    Serial.println(value);
  }
  #endif
  
  if (value < 60) 
  {
    return BUTTON_RIGHT;
  }
  else if (value < 200)
  {
    return BUTTON_UP;
  }
  else if (value < 400)
  {
    return BUTTON_DOWN;
  }
  else if (value < 600)
  {
    return BUTTON_LEFT;
  }
  else if (value < 800)
  {
    return BUTTON_SELECT;
  }

  return BUTTON_NONE;
}

#endif
