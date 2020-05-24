/** PROGMEM messages
 */
#ifndef _Messages_h
#define _Messages_h

const char M_SOFTWARE[]       PROGMEM = "Panel   EzyBus";
const char M_VERSION[]        PROGMEM = "v0.1    May 2020";

const char M_CONFIG[]         PROGMEM = "Configure";
const char M_SCAN_HARDWARE[]  PROGMEM = "Scan";
const char M_OUTPUT[]         PROGMEM = "Output";
const char M_INPUT[]          PROGMEM = "Input ";
const char M_INIT_INPUTS[]    PROGMEM = "Init inputs";

const char M_HI[]             PROGMEM = "Hi";
const char M_LO[]             PROGMEM = "Lo";
const char M_MOD[]            PROGMEM = "Mod";
const char M_PIN[]            PROGMEM = "Pin";


/** Gets a PROGMEM message and copies it to a buffer
 *  for regular usage.
 *  
 *  Note the buffer is a singleton and will be overwritten by the next call.
 */
//char* getMessage(PGM_P progmemMessage)
//{
//  static char buffer[16];
//  strcpy_P(buffer, progmemMessage);
//  return buffer;
//}

#endif
