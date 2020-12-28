/** Constants for Panel.
 */
#ifndef _Panel_h
#define _Panel_h

// A pin that will force calibration at start-up
#define PIN_CALIBRATE    11


/** Map the Input and Output nodes.
 */
void mapHardware();


/** Process all the Input's Outputs.
 */
void processInputOutputs(boolean aNewState);


/** Process an Input's n'th Output, setting it to the given state.
 *  Accumulate delay before or after movement depending on direction outputs are being processed.
 */
uint8_t processInputOutput(int aIndex, uint8_t aState, uint8_t aDelay);


/** Send a command to an output node.
 *  Return error code if any.
 *  Forward reference required for Configure class.
 */
int sendOutputCommand(uint8_t aValue, uint8_t aPace, uint8_t aDelay, uint8_t aState);


/** Sends the current debug level to all the connected outputs.
 */
void sendDebugLevel();


#endif
