/** Constants for SignalBox.
 *
 *
 *  (c)Copyright Tony Clulow  2021    tony.clulow@pentadtech.com
 *
 *  This work is licensed under the:
 *      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 *      http://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 *  For commercial use, please contact the original copyright holder(s) to agree licensing terms.
 */
 
#ifndef SignalBox_h
#define SignalBox_h


/** Record state of input switches. Referenced by Configure object.
 */
uint16_t currentSwitchState[INPUT_NODE_MAX];    // Current state of inputs.


/** Is an LCD shield present?
 */
boolean hasLcdShield = LCD_SHIELD;              // An LCD shield is present.


/** Scan for attached hardware (Input and Output nodes).
 */
void scanHardware();


/** Read the pins of a InputNode.
 *  Return the state of the pins, 16 bits, both ports.
 *  Return current state if there's a communication error, 
 *  this will prevent any actions being performed.
 */
uint16_t readInputNode(uint8_t aNode);


/** Scan all the Inputs.
 *  Parameter indicates if Configuration is in progress.
 */
void scanInputs(boolean aConfiguration);


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
