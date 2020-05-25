/** PROGMEM messages
 */
#ifndef _Messages_h
#define _Messages_h

const char M_SOFTWARE[]        PROGMEM = "Panel   EzyBus  ";
const char M_VERSION[]         PROGMEM = "v0.1    May 2020";

const char M_CONFIRM[]         PROGMEM = "Confirm? Sel=Yes";
const char M_CANCEL[]          PROGMEM = "Cancel?  Sel=Yes";
const char M_SAVED[]           PROGMEM = "Change saved    ";
const char M_CANCELLED[]       PROGMEM = "Change cancelled";

const char M_NO_INPUTS[]       PROGMEM = "No input module";
const char M_NO_OUTPUTS[]      PROGMEM = "No output module";
const char M_FIRST_RUN[]       PROGMEM = "First run";
const char M_DEFAULT_INPUTS[]  PROGMEM = "Default inputs";
const char M_DEFAULT_OUTPUTS[] PROGMEM = "Default outputs";

const char M_SCAN_HARDWARE[]   PROGMEM = "Scan";
const char M_INIT_INPUTS[]     PROGMEM = "Init inputs";

const char M_CONFIG[]          PROGMEM = "Configure";
const char M_OUTPUT[]          PROGMEM = "Output";
const char M_INPUT[]           PROGMEM = "Input ";
const char M_MOD[]             PROGMEM = "Mod";
const char M_PIN[]             PROGMEM = "Pin";
const char M_BUTTON[]          PROGMEM = "Button";
const char M_TOGGLE[]          PROGMEM = "Toggle";
const char M_DISABLED[]        PROGMEM = "..";

const char M_HI[]              PROGMEM = "Hi";
const char M_LO[]              PROGMEM = "Lo";

// Array of (pointers to) output types.
const char M_NONE[]            PROGMEM = "None  ";
const char M_SERVO[]           PROGMEM = "Servo ";
const char M_SIGNAL[]          PROGMEM = "Signal";
const char M_LED[]             PROGMEM = "LED   ";
 
const char* const M_OUTPUT_TYPES[] = { M_NONE, M_SERVO, M_SIGNAL, M_LED };


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
