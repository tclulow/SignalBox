/** Debug data.
 */
#ifndef _Debug_h
#define _Debug_h

// Common debug messages.
const char M_DEBUG_LOAD[]       PROGMEM = "Load";
const char M_DEBUG_SAVE[]       PROGMEM = "Save";
const char M_DEBUG_SEND[]       PROGMEM = "Send";
const char M_DEBUG_STATES[]     PROGMEM = "States";
const char M_DEBUG_SYSTEM[]     PROGMEM = "System";
const char M_DEBUG_WRITE[]      PROGMEM = "Write";

const char M_DEBUG_DELAY[]      PROGMEM = ", delay=";
const char M_DEBUG_HI[]         PROGMEM = ", hi=";
const char M_DEBUG_LO[]         PROGMEM = ", lo=";
const char M_DEBUG_PACE[]       PROGMEM = ", pace=";
const char M_DEBUG_STATE[]      PROGMEM = ", state=";
const char M_DEBUG_TARGET[]     PROGMEM = ", target=";
const char M_DEBUG_TYPE[]       PROGMEM = ", type=";
const char M_DEBUG_VALUE[]      PROGMEM = ", value=";


#if MASTER

// Master-only debug messages.
const char M_DEBUG_BUTTON[]     PROGMEM = "Button";
const char M_DEBUG_READ[]       PROGMEM = "Read";
const char M_DEBUG_RENUMBER[]   PROGMEM = "Renumber";
const char M_DEBUG_RESET[]      PROGMEM = "Reset";

const char M_DEBUG_INPUTS[]     PROGMEM = ", inputs=";
const char M_DEBUG_NODE[]       PROGMEM = ", node=";
const char M_DEBUG_OUTPUTS[]    PROGMEM = ", outputs=";
const char M_DEBUG_PIN[]        PROGMEM = ", pin=";
const char M_DEBUG_RETURN[]     PROGMEM = ", ret=";


#else

// Non-master debug messages.
const char M_DEBUG_ACTION[]     PROGMEM = "Action";
const char M_DEBUG_INIT[]       PROGMEM = "Init";
const char M_DEBUG_MODULE[]     PROGMEM = "Module";
const char M_DEBUG_MOVE[]       PROGMEM = "Move";
const char M_DEBUG_RECEIPT[]    PROGMEM = "Receipt";
const char M_DEBUG_REQUEST[]    PROGMEM = "Request";
const char M_DEBUG_TRIGGER[]    PROGMEM = "Trigger";
const char M_DEBUG_UNEXPECTED[] PROGMEM = "Unexpected";
const char M_DEBUG_UNKNOWN[]    PROGMEM = "Unknown";

const char M_DEBUG_ALT[]        PROGMEM = ", alt=";
const char M_DEBUG_COMMAND[]    PROGMEM = ", cmd=";
const char M_DEBUG_LEN[]        PROGMEM = ", len=";
const char M_DEBUG_OPTION[]     PROGMEM = ", opt=";
const char M_DEBUG_START[]      PROGMEM = ", start=";
const char M_DEBUG_STEP[]       PROGMEM = ", step=";
const char M_DEBUG_STEPS[]      PROGMEM = ", steps=";



#endif

#endif
