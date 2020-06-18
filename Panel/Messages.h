/** PROGMEM messages
 */
#ifndef _Messages_h
#define _Messages_h

// Incantation to convert a PROGMEM string to something usable by other finctions.
#define PGMT(pgm_ptr) ( reinterpret_cast< const __FlashStringHelper * >( pgm_ptr ) )


const char M_SOFTWARE[]        PROGMEM = "Panel";
const char M_VERSION[]         PROGMEM = "v0.2";
const char M_VERSION_DATE[]    PROGMEM = "Jun 2020";

const char M_CONFIRM[]         PROGMEM = "Confirm? Sel=Yes";
const char M_CANCEL[]          PROGMEM = "Cancel?  Sel=Yes";
const char M_SAVED[]           PROGMEM = "Change saved    ";
const char M_CANCELLED[]       PROGMEM = "Change cancelled";
const char M_FAILURE[]         PROGMEM = "System error";
const char M_TODO[]            PROGMEM = "TODO";

const char M_CALIBRATE[]       PROGMEM = "Calibrate button";
const char M_PRESS[]           PROGMEM = "Press";
const char M_SELECT[]          PROGMEM = "Select";
const char M_LEFT[]            PROGMEM = "Left";
const char M_DOWN[]            PROGMEM = "Down";
const char M_UP[]              PROGMEM = "Up";
const char M_RIGHT[]           PROGMEM = "Right";

const char M_SETUP[]           PROGMEM = "Setup";
const char M_INITIALISING[]    PROGMEM = "Initialising";
const char M_EZY_FOUND[]       PROGMEM = "EzyBus  detected";
const char M_EZY_UPDATE[]      PROGMEM = "Update?  Sel=Yes";
const char M_EZY_UPDATING[]    PROGMEM = "EzyBus  updating";

const char M_SCAN_NODES[]      PROGMEM = "Nodes";
const char M_NO_INPUTS[]       PROGMEM = "No input nodes";
const char M_NO_OUTPUTS[]      PROGMEM = "No output nodes";
const char M_INIT_INPUTS[]     PROGMEM = "Init inputs";

const char M_DEFAULT_INPUTS[]  PROGMEM = "Default inputs";
const char M_DEFAULT_OUTPUTS[] PROGMEM = "Default outputs";
const char M_EXPORTING[]       PROGMEM = "Exporting";
const char M_WAITING[]         PROGMEM = "Waiting";

const char M_CONFIG[]          PROGMEM = "Configure";
const char M_SYSTEM[]          PROGMEM = "System";
const char M_DETAIL[]          PROGMEM = "Detail";
const char M_PARAMS[]          PROGMEM = "Params";
const char M_OUTPUT[]          PROGMEM = "Output";
const char M_INPUT[]           PROGMEM = "Input ";
const char M_EXPORT[]          PROGMEM = "Export";
const char M_IMPORT[]          PROGMEM = "Import";
const char M_ALL[]             PROGMEM = "All   ";

const char M_DEBUG[]           PROGMEM = "Debug ";
const char M_SYS_I2C[]         PROGMEM = "i2cID ";

const char M_IMPORT_INPUT[]    PROGMEM = "Input";
const char M_IMPORT_SERVO[]    PROGMEM = "Servo";
const char M_IMPORT_SIGNAL[]   PROGMEM = "Signal";
const char M_IMPORT_LED[]      PROGMEM = "LED";

const char M_ID_CONSOLE[]      PROGMEM = "Con";
const char M_ID_INPUT[]        PROGMEM = "Inp";
const char M_ID_OUTPUT[]       PROGMEM = "Out";

const char M_OFF[]             PROGMEM = "Off  ";
const char M_LOW[]             PROGMEM = "Low  ";
const char M_HIGH[]            PROGMEM = "High ";
const char M_PAUSE[]           PROGMEM = "Pause";

const char M_MOD[]             PROGMEM = "Mod";
const char M_PIN[]             PROGMEM = "Pin";

const char M_BUTTON[]          PROGMEM = "Button";
const char M_TOGGLE[]          PROGMEM = "Toggle";
const char M_DISABLED[]        PROGMEM = "..";

const char M_SERVO[]           PROGMEM = "Servo ";
const char M_SIGNAL[]          PROGMEM = "Signal";
const char M_LED[]             PROGMEM = "LED   ";

const char M_HI[]              PROGMEM = "Hi";
const char M_LO[]              PROGMEM = "Lo";
const char M_PACE[]            PROGMEM = "Spd";
const char M_DELAY[]           PROGMEM = "Delay";



// Headers for the exports
const char M_EXPORT_SYSTEM[]   PROGMEM = "#System\tVersion\tDetail\tControl\tInput\tOutput";
const char M_EXPORT_INPUT[]    PROGMEM = "#Input\tNode\tPin\tType\tOutput1\tOutput2\tOutput3";
const char M_EXPORT_OUTPUT[]   PROGMEM = "#Output\tNode\tPin\tType\tLo\tHi\tSpd\tDelay";


// Array of (pointers to) certain messages.
const char* const M_BUTTONS[]        = { M_SELECT, M_LEFT, M_DOWN, M_UP, M_RIGHT };
const char* const M_TOP_MENU[]       = { M_SYSTEM, M_INPUT, M_OUTPUT, M_EXPORT, M_IMPORT };
const char* const M_SYS_TYPES[]      = { M_DEBUG, M_SYS_I2C };
const char* const M_REPORT_TYPES[]   = { M_ALL, M_SYSTEM, M_INPUT, M_OUTPUT };
const char* const M_I2C_PROMPTS[]    = { M_ID_CONSOLE, M_ID_INPUT, M_ID_OUTPUT };
const char* const M_DEBUG_PROMPTS[]  = { M_OFF, M_LOW, M_HIGH, M_PAUSE };

const char* const M_OUTPUT_PROMPTS[] = { M_LO, M_HI, M_PACE };
const char* const M_INPUT_TYPES[]    = { M_BUTTON, M_TOGGLE };
const char* const M_OUTPUT_TYPES[]   = { M_SERVO, M_SIGNAL, M_LED };



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
