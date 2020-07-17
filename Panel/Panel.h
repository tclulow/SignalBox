/** Constants for Panel.
 */
#ifndef _Panel_h
#define _Panel_h

// A pin that will force calibration at start-up
#define PIN_CALIBRATE    11

// Useful characters
const char CHAR_SPACE   = ' ';
const char CHAR_TAB     = '\t';
const char CHAR_NEWLINE = '\n';
const char CHAR_RETURN  = '\r';
const char CHAR_NULL    = 0;

const char CHAR_HASH    = '#';
const char CHAR_DOT     = '.';
const char CHAR_COLON   = ':';
const char CHAR_LEFT    = '<';
const char CHAR_RIGHT   = '>';
const char CHAR_STAR    = '*';

const char HEX_CHARS[]  = "0123456789abcdef";
const char EDIT_CHARS[] = "ABC";


/** Process all the Input's Outputs.
 */
void processInputOutputs(uint8_t aNewState);


/** Send a command to an output node.
 *  Return error code if any.
 *  Forward reference required for Configure class.
 */
int sendOutputCommand(uint8_t aValue, uint8_t aPace, uint8_t aDelay, uint8_t aState);


#endif
