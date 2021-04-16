/** PROGMEM messages
 *
 *
 *  (c)Copyright Tony Clulow  2021    tony.clulow@pentadtech.com
 *
 *  This work is licensed under the:
 *      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 *      http://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 *  For commercial use, please contact the original copyright holder(s) to agree licensing terms
 */
 
#ifndef Messages_h
#define Messages_h


// Incantation to convert a PROGMEM string to something usable by other functions.
#define PGMT(pgm_ptr) ( reinterpret_cast< const __FlashStringHelper * >( pgm_ptr ) )


// Software versioning.
#if MASTER
    const char M_SOFTWARE[]     PROGMEM = "SignalBox";
#else
    const char M_SOFTWARE[]     PROGMEM = "Output module";
#endif
const char M_VERSION[]          PROGMEM = "v3.3.1";        // See also system.VERSION.
const char M_VERSION_DATE[]     PROGMEM = "Apr 21";
const char M_STARTUP[]          PROGMEM = "Startup";
const char M_UPDATE[]           PROGMEM = "Update ";
const char M_UNKNOWN[]          PROGMEM = "Unknown";


// General messages (shared)
const char M_SYSTEM[]           PROGMEM = "System";
const char M_INPUT[]            PROGMEM = "Input";
const char M_OUTPUT[]           PROGMEM = "Output";
const char M_TYPES[]            PROGMEM = "Types";


// Configuration - Output.
const char M_NEW_NODE_NO[]      PROGMEM = "New node#";
const char M_NONE[]             PROGMEM = "None";
const char M_SERVO[]            PROGMEM = "Servo";
const char M_SIGNAL[]           PROGMEM = "Signal";
const char M_LED[]              PROGMEM = "LED";
const char M_LED_4[]            PROGMEM = "LED_4";
const char M_ROAD_UK[]          PROGMEM = "RoadUK";
const char M_ROAD_RW[]          PROGMEM = "RoadRW";
const char M_FLASH[]            PROGMEM = "Flash";
const char M_BLINK[]            PROGMEM = "Blink";
const char M_RANDOM[]           PROGMEM = "Random";
const char M_RFU[]              PROGMEM = "RFU";

const char M_HI[]               PROGMEM = "Hi";
const char M_LO[]               PROGMEM = "Lo";
const char M_PACE[]             PROGMEM = "Spd";
const char M_DELAY[]            PROGMEM = "Delay";
const char M_RESET[]            PROGMEM = "Reset";

const char* const M_OUTPUT_TYPES[]   = { M_NONE,  M_SERVO,  M_SIGNAL, M_LED, M_LED_4, M_ROAD_UK, M_ROAD_RW, M_FLASH, 
                                         M_BLINK, M_RANDOM, M_RFU,    M_RFU, M_RFU,   M_RFU,     M_RFU,     M_RFU };


#if MASTER

    // General messages.
    const char M_CONFIRM[]          PROGMEM = "Confirm? Sel=Yes";
    const char M_CANCEL[]           PROGMEM = "Cancel?  Sel=Yes";
    const char M_CONFIRMED[]        PROGMEM = "Change confirmed";
    const char M_CANCELLED[]        PROGMEM = "Change cancelled";
    const char M_ABANDONED[]        PROGMEM = "Abandoned";
    const char M_FAILURE[]          PROGMEM = "System error";
    const char M_INTERRUPT[]        PROGMEM = "Interrupt"; 
    const char M_RENUMBER[]         PROGMEM = "Renumber";
    const char M_SCANNING[]         PROGMEM = "Scanning inputs";
    // const char M_TODO[]          PROGMEM = "TODO";


    // Calibrate LCD buttons.
    const char M_CALIBRATE[]        PROGMEM = "Calibrate button";
    const char M_PRESS[]            PROGMEM = "Press";
    const char M_SELECT[]           PROGMEM = "Select";
    const char M_LEFT[]             PROGMEM = "Left";
    const char M_DOWN[]             PROGMEM = "Down";
    const char M_UP[]               PROGMEM = "Up";
    const char M_RIGHT[]            PROGMEM = "Right";
    const char M_SEQUENCE[]         PROGMEM = "Out of sequence";

    
    // Start-up and first-run messages.
    const char M_SETUP[]            PROGMEM = "Setup";
    const char M_INITIALISING[]     PROGMEM = "Initialising";
    const char M_EZY_FOUND[]        PROGMEM = "EzyBus  detected";
    const char M_EZY_UPDATE[]       PROGMEM = "Update?  Sel=Yes";
    const char M_EZY_UPDATING[]     PROGMEM = "EzyBus  updating";
    
    const char M_NODES[]            PROGMEM = "Nodes";
    const char M_NO_INPUT[]         PROGMEM = "No input node";
    const char M_NO_OUTPUT[]        PROGMEM = "No output node";
    const char M_INIT_INPUTS[]      PROGMEM = "Init inputs";

    
    // Import/export actions.
    const char M_EXPORTING[]        PROGMEM = "Exporting";
    const char M_WAITING[]          PROGMEM = "Waiting";


    // Configuration - general.
    const char M_LOCK[]             PROGMEM = "Lock";
    const char M_CONFIG[]           PROGMEM = "Configure";
    const char M_DETAIL[]           PROGMEM = "Detail";
    const char M_PARAMS[]           PROGMEM = "Params";
    const char M_EXPORT[]           PROGMEM = "Export";
    const char M_IMPORT[]           PROGMEM = "Import";
    const char M_ALL[]              PROGMEM = "All";

    
    // Configuration - System.
    const char M_REPORT[]           PROGMEM = "Report";
    const char M_SYS_I2C[]          PROGMEM = "i2cID";
    const char M_DEBUG[]            PROGMEM = "Debug";
    const char M_IDENT[]            PROGMEM = "Ident";

    const char M_SHORT[]            PROGMEM = "Short";
    const char M_LONG[]             PROGMEM = "Long";
    const char M_PAUSE[]            PROGMEM = "Pause";
    
    const char M_ERRORS[]           PROGMEM = "Errors";
    const char M_BRIEF[]            PROGMEM = "Brief";
    const char M_FULL[]             PROGMEM = "Full";
    
    const char M_ID_CONTROLLER[]    PROGMEM = "Con";
    const char M_ID_INPUT[]         PROGMEM = "Inp";
    const char M_ID_OUTPUT[]        PROGMEM = "Out";

    const char M_VS[]               PROGMEM = " vs ";


    // Configuration - Input.
    const char M_TOGGLE[]           PROGMEM = "Toggle";
    const char M_ON_OFF[]           PROGMEM = "On_Off";
    const char M_ON[]               PROGMEM = "On";
    const char M_OFF[]              PROGMEM = "Off";
    
    
    // Headers for the exports
    const char M_EXPORT_SYSTEM[]    PROGMEM = "#System\tVersion\tDetail\tControl\tInput\tOutput\tReport";
    const char M_EXPORT_INPUT[]     PROGMEM = "#Input\tNode\tPin\tType";
    const char M_EXPORT_INPUT_OUT[] PROGMEM = "\tOutput";
    const char M_EXPORT_OUTPUT[]    PROGMEM = "#Output\tNode\tPin\tType\tLo\tHi\tSpd\tReset";
    const char M_EXPORT_LOCKS[]     PROGMEM = "#Lock\tNode\tPin";
    const char M_EXPORT_LOCK[]      PROGMEM = "\tLock";


    // Array of (pointers to) certain messages.
    const char* const M_BUTTONS[]        = { M_NONE, M_SELECT, M_LEFT, M_DOWN, M_UP, M_RIGHT };
    const char* const M_TOP_MENU[]       = { M_SYSTEM, M_INPUT, M_OUTPUT, M_LOCK, M_EXPORT, M_IMPORT };
    const char* const M_SYS_TYPES[]      = { M_REPORT, M_SYS_I2C, M_NODES, M_IDENT, M_DEBUG };
    const char* const M_EXPORT_TYPES[]   = { M_ALL, M_SYSTEM, M_INPUT, M_OUTPUT, M_LOCK };
    const char* const M_I2C_PROMPTS[]    = { M_ID_CONTROLLER, M_ID_INPUT, M_ID_OUTPUT };
    const char* const M_REPORT_PROMPTS[] = { M_NONE, M_SHORT, M_LONG, M_PAUSE };
    const char* const M_DEBUG_PROMPTS[]  = { M_NONE, M_ERRORS, M_BRIEF, M_DETAIL, M_FULL };
    const char* const M_INPUT_TYPES[]    = { M_TOGGLE, M_ON_OFF, M_ON,  M_OFF };
    
#endif

    
// Common debug messages.

const char M_DEBUG_DEBUG[]      PROGMEM = "Debug";
const char M_DEBUG_I2C[]        PROGMEM = "I2C";
const char M_DEBUG_LOAD[]       PROGMEM = "Load";
const char M_DEBUG_MOVE[]       PROGMEM = "Move";
const char M_DEBUG_READ[]       PROGMEM = "Read";
const char M_DEBUG_REPORT[]     PROGMEM = "Report";
const char M_DEBUG_RESET[]      PROGMEM = "Reset";
const char M_DEBUG_SAVE[]       PROGMEM = "Save";
const char M_DEBUG_SEND[]       PROGMEM = "Send";
const char M_DEBUG_SET_LO[]     PROGMEM = "SetLo";
const char M_DEBUG_SET_HI[]     PROGMEM = "SetHi";
const char M_DEBUG_STATES[]     PROGMEM = "States";
const char M_DEBUG_SYSTEM[]     PROGMEM = "System";
const char M_DEBUG_WRITE[]      PROGMEM = "Write";

const char M_DEBUG_COMMAND[]    PROGMEM = ", cmd=";
const char M_DEBUG_DELAY_TO[]   PROGMEM = ", delayTo=";
const char M_DEBUG_HI[]         PROGMEM = ", hi=";
const char M_DEBUG_LO[]         PROGMEM = ", lo=";
const char M_DEBUG_LOCK_HI[]    PROGMEM = ", lockHi=";
const char M_DEBUG_LOCK_LO[]    PROGMEM = ", lockLo=";
const char M_DEBUG_NODE[]       PROGMEM = ", node=";
const char M_DEBUG_PACE[]       PROGMEM = ", pace=";
const char M_DEBUG_RESET_AT[]   PROGMEM = ", resetAt=";
const char M_DEBUG_STATE[]      PROGMEM = ", state=";
const char M_DEBUG_TARGET[]     PROGMEM = ", target=";
const char M_DEBUG_TO[]         PROGMEM = ", to=";
const char M_DEBUG_TYPE[]       PROGMEM = ", type=";
const char M_DEBUG_VALUE[]      PROGMEM = ", value=";

const char* const M_DEBUG_COMMANDS[]   = { M_DEBUG_SYSTEM, M_DEBUG_DEBUG, M_DEBUG_SET_LO, M_DEBUG_SET_HI, M_DEBUG_READ, M_DEBUG_WRITE, M_DEBUG_SAVE, M_DEBUG_RESET,
                                           M_RFU, M_RFU, M_RFU, M_RFU, M_RFU, M_RFU, M_RFU, M_NONE };

#if MASTER

    // Master-only debug messages.
    const char M_DEBUG_BUTTON[]     PROGMEM = "Button";

    const char M_DEBUG_OUTPUTS[]    PROGMEM = ", outputs=";
    const char M_DEBUG_PIN[]        PROGMEM = ", pin=";
    const char M_DEBUG_RETURN[]     PROGMEM = ", ret=";

#else

    // Non-master debug messages.
    const char M_DEBUG_ACTION[]     PROGMEM = "Action";
    const char M_DEBUG_INIT[]       PROGMEM = "Init";
    const char M_DEBUG_MODULE[]     PROGMEM = "Module";
    const char M_DEBUG_RECEIPT[]    PROGMEM = "Receipt";
    const char M_DEBUG_REQUEST[]    PROGMEM = "Request";
    const char M_DEBUG_TRIGGER[]    PROGMEM = "Trigger";
    const char M_DEBUG_UNEXPECTED[] PROGMEM = "Unexpected";

    const char M_DEBUG_ALT[]        PROGMEM = ", alt=";
    const char M_DEBUG_LEN[]        PROGMEM = ", len=";
    const char M_DEBUG_OPTION[]     PROGMEM = ", opt=";
    const char M_DEBUG_START[]      PROGMEM = ", start=";
    const char M_DEBUG_STEP[]       PROGMEM = ", step=";
    const char M_DEBUG_STEPS[]      PROGMEM = ", steps=";

#endif

#endif
