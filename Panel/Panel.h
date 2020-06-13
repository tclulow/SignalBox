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
const char CHAR_LEFT    = '<';
const char CHAR_RIGHT   = '>';
const char CHAR_STAR    = '*';

const char HEX_CHARS[]  = "0123456789abcdef";
const char EDIT_CHARS[] = "ABC";


/** Send a command to an output node.
 *  Return error code if any.
 *  Forward reference required for Configure class.
 */
int sendOutputCommand(int aValue, int aPace, int aState);

#endif
