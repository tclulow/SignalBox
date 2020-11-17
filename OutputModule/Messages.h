/** PROGMEM messages
 */
#ifndef _Messages_h
#define _Messages_h


// Incantation to convert a PROGMEM string to something usable by other finctions.
#define PGMT(pgm_ptr) ( reinterpret_cast< const __FlashStringHelper * >( pgm_ptr ) )


// Software versioning.
const char M_SOFTWARE[]        PROGMEM = "Panel Output";
const char M_VERSION[]         PROGMEM = "v0.4";
const char M_VERSION_DATE[]    PROGMEM = "Nov 2020";

const char M_SPACE = ' ';


#endif
