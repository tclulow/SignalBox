/** PROGMEM messages
 */
#ifndef _Messages_h
#define _Messages_h
 
const char M_SERVO[]          PROGMEM = "Servo";
const char M_SWITCH[]         PROGMEM = "Switch";
const char M_SCAN_HARDWARE[]  PROGMEM = "Scan hardware";
const char M_INIT_SWITCHES[]  PROGMEM = "Init switches";


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
