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


/** Send a command to an output node.
 *  Return error code if any.
 *  Forward reference required for Configure class.
 */
int sendOutputCommand(uint8_t aValue, uint8_t aPace, uint8_t aDelay, uint8_t aState);


#endif
