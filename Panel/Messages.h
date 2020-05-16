/** PROGMEM messages
 */
#ifndef _MESSAGES_H
#define _MESSAGES_H
 
const char M_SCAN_HARDWARE[]  PROGMEM = "Scan hardware";
const char M_SERVO[]          PROGMEM = "Servo";
const char M_SWITCH[]         PROGMEM = "Switch";

/** Gets a PROGMEM message and copies it to a buffer
 *  for regular usage.
 *  
 *  Note the buffer is a singleton and will be overwritten by the next call.
 */
char* getMessage(PGM_P progmemMessage)
{
  static char buffer[16];
  strcpy_P(buffer, progmemMessage);
  return buffer;
}

#endif
