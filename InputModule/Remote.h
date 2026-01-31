/** OutputModule.
 *  @file
 *
 *  (c)Copyright Tony Clulow  2021  tony.clulow@pentadtech.com
 *
 *  This work is licensed under the:
 *      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 *      http://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 *  For commercial use, please contact the original copyright holder(s) to agree licensing terms.
 *
 */

#ifndef Remote_h
#define Remote_h

#include <IRremote.hpp>


static const uint8_t REMOTE_PIN  = 11;          // Remote control IR pin.


/** Handle IR commands from a remote controller.
 */
class Remote
{
    public:

    /** Constructor.
     */
    Remote()
    {

    }


    /** Initialise.
     */
    void init()
    {
        IrReceiver.begin(REMOTE_PIN, ENABLE_LED_FEEDBACK);
        IrReceiver.printActiveIRProtocols(&Serial);
    }


    /** Update the state.
     */
    void update()
    {
        if (IrReceiver.decode())
        {
            if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_WAS_OVERFLOW)
            {
                Serial.println(F("Receive overflow."));
            }
            else
            {
                // Print info
                // if (IrReceiver.decodedIRData.protocol == UNKNOWN)
                {
                    Serial.println(F("Received noise or an unknown (or not yet enabled) protocol"));
                    IrReceiver.printIRResultRawFormatted(&Serial, true);
                    auto tDecodedRawData = IrReceiver.decodedIRData.decodedRawData; // uint32_t on 8 and 16 bit CPUs and uint64_t on 32 and 64 bit CPUs
                    Serial.print(F("Raw data received are 0x"));
                    Serial.println(tDecodedRawData);
                }

                if (IrReceiver.decodedIRData.protocol != UNKNOWN)
                {
                    // The info output for a successful receive
                    IrReceiver.printIRResultShort(&Serial);
                    IrReceiver.printIRSendUsage(&Serial);
                }
            }
        
            // Restart decoder after command received.
            IrReceiver.resume();

            // Do something with the command.
            if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT)
            {
                Serial.println(F("Repeat received. Here you can repeat the same action as before."));
            }
            else
            {
                Serial.print(F("Address=0x"));
                Serial.print(IrReceiver.decodedIRData.address, HEX);
                Serial.print(F(", Command=0x"));
                Serial.print(IrReceiver.decodedIRData.command, HEX);
                Serial.println();
            }
        }
    }
};


// Singleton instance
Remote remote;

#endif
