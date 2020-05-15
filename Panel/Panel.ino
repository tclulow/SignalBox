/*

  The circuit:
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

const char message[] = "A message";


void setup()
{
  Serial.begin(115200);

  lcdInit();

  // Print a message to the LCD.
  lcd.print(message);
  delay(2000);
  
  lcd.setCursor(0, 1);
  // lcdPrint(message_p);
  lcd.print(getMessage(message_p));
  delay(2000);
}

void loop()
{
  lcd.setCursor(10, 1);
  lcd.print(millis());
}
 
