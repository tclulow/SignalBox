/** Gateway
 *
 *
 *  (c)Copyright Tony Clulow  2021  tony.clulow@pentadtech.com
 *
 *  This work is licensed under the:
 *      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 *      http://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 *  For commercial use, please contact the original copyright holder(s) to agree licensing terms.
 */


#include <CBUS2515.h>           // CBUS 2515 driver and supporting classes
#include <CBUSParams.h>

#include "I2cComms.h"           // I2C comms for SignalBox


// Debug helper functions
#define DEBUG true              // Enable debugging

#if DEBUG
    #define debugPrint(aParam)     Serial.print(aParam)
    #define debugPrintHex(aParam)  Serial.print(aParam, HEX)
    #define debugPrintln(aParam)   Serial.println(aParam)
#else
    #define debugPrint(aParam)
    #define debugPrintHex(aParam)
    #define debugPrintln(aParam)
#endif


// Signalbox definitions
#define OUTPUT_NODE_MAX      32     // Length of output node array.
#define INPUT_NODE_MAX        8     // Length of input node array.


// CBUS definitions.
#define CBUS_MAJ               1    // major version
#define CBUS_MIN             ' '    // minor version
#define CBUS_BETA              0    // beta
#define CBUS_ID               99    // CBUS module ID

#define CBUS_PIN_INT           2    // Interupt pin.
#define CBUS_PIN_CS           10    // Chip select pin.
#define CBUS_PIN_SI            2    // SPI in pin.
#define CBUS_PIN_SO            2    // SPI out pin.
#define CBUS_PIN_CLK           2    // Clock pin.

#define CBUS_FREQ       8000000L     // CAN2515 board frequency
