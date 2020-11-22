/** PROGMEM messages
 */
#ifndef _Messages_h
#define _Messages_h


// Incantation to convert a PROGMEM string to something usable by other finctions.
#define PGMT(pgm_ptr) ( reinterpret_cast< const __FlashStringHelper * >( pgm_ptr ) )


// Software versioning.
const char M_SOFTWARE[]        PROGMEM = "Panel";
const char M_VERSION[]         PROGMEM = "v1.2";
const char M_VERSION_DATE[]    PROGMEM = "Nov 2020";
const char M_UPDATE[]          PROGMEM = "Update";

// General messages.
const char M_CONFIRM[]         PROGMEM = "Confirm? Sel=Yes";
const char M_CANCEL[]          PROGMEM = "Cancel?  Sel=Yes";
const char M_SAVED[]           PROGMEM = "Change saved    ";
const char M_CANCELLED[]       PROGMEM = "Change cancelled";
const char M_FAILURE[]         PROGMEM = "System error";
// const char M_TODO[]            PROGMEM = "TODO";

// Calibrate LCD buttons.
const char M_CALIBRATE[]       PROGMEM = "Calibrate button";
const char M_PRESS[]           PROGMEM = "Press";
const char M_SELECT[]          PROGMEM = "Select";
const char M_LEFT[]            PROGMEM = "Left";
const char M_DOWN[]            PROGMEM = "Down";
const char M_UP[]              PROGMEM = "Up";
const char M_RIGHT[]           PROGMEM = "Right";

// Start-up and first-run messages.
const char M_SETUP[]           PROGMEM = "Setup";
const char M_INITIALISING[]    PROGMEM = "Initialising";
const char M_EZY_FOUND[]       PROGMEM = "EzyBus  detected";
const char M_EZY_UPDATE[]      PROGMEM = "Update?  Sel=Yes";
const char M_EZY_UPDATING[]    PROGMEM = "EzyBus  updating";

const char M_SCAN_NODES[]      PROGMEM = "Nodes";
const char M_NO_INPUTS[]       PROGMEM = "No input nodes";
const char M_NO_OUTPUTS[]      PROGMEM = "No output nodes";
const char M_INIT_INPUTS[]     PROGMEM = "Init inputs";

// Import/export actions.
const char M_EXPORTING[]       PROGMEM = "Exporting";
const char M_WAITING[]         PROGMEM = "Waiting";

// Configuration - general.
const char M_CONFIG[]          PROGMEM = "Configure";
const char M_SYSTEM[]          PROGMEM = "System";
const char M_DETAIL[]          PROGMEM = "Detail";
const char M_PARAMS[]          PROGMEM = "Params";
const char M_OUTPUT[]          PROGMEM = "Output";
const char M_INPUT[]           PROGMEM = "Input";
const char M_EXPORT[]          PROGMEM = "Export";
const char M_IMPORT[]          PROGMEM = "Import";
const char M_ALL[]             PROGMEM = "All";
const char M_TYPES[]           PROGMEM = "Types";

// Configuration - System.
const char M_REPORT[]          PROGMEM = "Report";
const char M_SYS_I2C[]         PROGMEM = "i2cID";

const char M_ID_CONTROLLER[]   PROGMEM = "Con";
const char M_ID_INPUT[]        PROGMEM = "Inp";
const char M_ID_OUTPUT[]       PROGMEM = "Out";

const char M_NONE[]            PROGMEM = "None";
const char M_SHORT[]           PROGMEM = "Short";
const char M_LONG[]            PROGMEM = "Long";
const char M_PAUSE[]           PROGMEM = "Pause";

// const char M_MOD[]             PROGMEM = "Mod";
// const char M_PIN[]             PROGMEM = "Pin";

// Configuration - Input.
const char M_TOGGLE[]          PROGMEM = "Toggle";
const char M_ON_OFF[]          PROGMEM = "On/Off";
const char M_ON[]              PROGMEM = "On";
const char M_OFF[]             PROGMEM = "Off";
const char M_DISABLED[]        PROGMEM = "..";

// Configuration - Output.
const char M_SERVO[]           PROGMEM = "Servo";
const char M_SIGNAL[]          PROGMEM = "Signal";
const char M_LED[]             PROGMEM = "LED";
const char M_RFU[]             PROGMEM = "RFU";

const char M_HI[]              PROGMEM = "Hi";
const char M_LO[]              PROGMEM = "Lo";
const char M_PACE[]            PROGMEM = "Spd";
const char M_DELAY[]           PROGMEM = "Delay";

// MCP interface.
const char M_MCP_ERROR[]       PROGMEM = "MCP error";
const char M_MCP_COMMS[]       PROGMEM = "MCP comms len"; 


// Headers for the exports
const char M_EXPORT_SYSTEM[]   PROGMEM = "#System\tVersion\tDetail\tControl\tInput\tOutput\tReport";
const char M_EXPORT_INPUT[]    PROGMEM = "#Input\tNode\tPin\tType\tOutputA\tOutputB\tOutputC";
const char M_EXPORT_OUTPUT[]   PROGMEM = "#Output\tNode\tPin\tType\tLo\tHi\tSpd\tDelay";


// Array of (pointers to) certain messages.
const char* const M_BUTTONS[]        = { M_SELECT, M_LEFT, M_DOWN, M_UP, M_RIGHT };
const char* const M_TOP_MENU[]       = { M_SYSTEM, M_INPUT, M_OUTPUT, M_EXPORT, M_IMPORT };
const char* const M_SYS_TYPES[]      = { M_REPORT, M_SYS_I2C };
const char* const M_REPORT_TYPES[]   = { M_ALL, M_SYSTEM, M_INPUT, M_OUTPUT };
const char* const M_I2C_PROMPTS[]    = { M_ID_CONTROLLER, M_ID_INPUT, M_ID_OUTPUT };
const char* const M_REPORT_PROMPTS[] = { M_NONE, M_SHORT, M_LONG, M_PAUSE };

const char* const M_INPUT_TYPES[]    = { M_TOGGLE, M_ON_OFF, M_ON,  M_OFF };
const char* const M_OUTPUT_TYPES[]   = { M_SERVO,  M_SIGNAL, M_LED, M_RFU, M_RFU, M_RFU, M_RFU, M_RFU, M_RFU, M_RFU, M_RFU, M_RFU, M_RFU, M_RFU, M_RFU, M_RFU };


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
