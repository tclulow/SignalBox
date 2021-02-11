/** Constants for SignalBox.
 *
 *
 *  (c)Copyright Tony Clulow  2021    tony.clulow@pentadtech.com
 *
 *  This work is licensed under the:
 *      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 *      http://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 *   For commercial use, please contact the original copyright holder(s) to agree licensing terms
 */
#ifndef _SignalBox_h
#define _SignalBox_h


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
uint8_t processInputOutput(uint8_t aIndex, uint8_t aState, uint8_t aDelay);


/** Send a command to an output node.
 *  Return error code if any.
 *  Forward reference required for Configure class.
 */
int sendOutputCommand(uint8_t aValue, uint8_t aPace, uint8_t aDelay, uint8_t aState);


/** Sends the current debug level to all the connected outputs.
 */
void sendDebugLevel();


#endif
