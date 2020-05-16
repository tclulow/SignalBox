/** 
 *  Controll panel.
 *  
 *  LCD constants and functions.
 *
 * The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
 */

#include "Messages.h"
#include "Lcd.h"
#include "Panel.h"

// initialize the library with the numbers of the interface pins
// LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
LCD lcd(12, 11, 5, 4, 3, 2);


/** Setup the Arduino.
 */
void setup()
{
  Serial.begin(115200);

  lcd.begin(16, 2);

  // Print a message to the LCD.
  lcd.printAt(0, 0, message_p);
  delay(2000);
  
  lcd.setCursor(0, 1);
  // lcdPrint(message_p);
  lcd.print(getMessage(message_p));
  delay(2000);
}


/** Main loop.
 */
void loop()
{
  lcd.setCursor(10, 1);
  lcd.print(millis());
}
 
