/** OutputModule.
 *  @file
 *
 *  (c)Copyright Tony Clulow  2021-26   antony.clulow@gmail.com
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
#include <EEPROM.h>
#include <SoftwareSerial.h>


static const uint8_t IR_PIN        = 11;        // Remote control IR pin.
static const uint8_t BT_PIN_RX     = 10;        // Bluetooth RX pin.
static const uint8_t BT_PIN_TX     = 9;         // Bluetooth TX pin.
static const uint8_t INPUT_PIN_MAX = 16;        // 16 inputs to the node. Must match InputDef.INPUT_PIN_MAX.
static const long    DELAY_PROGRAM = 5000;      // Give up waiting for proramming after this interval (msecs).
static const long    DELAY_RESET   = 1000;      // Reset inputs after this interval.
static const long    DELAY_WAITING = 20;        // Fast flicker when waiting for programming.


/** Handle IR commands from a remote controller
 *  and/or commands from a BT receiver.
 *  Extends Persisted to store IR codes.
 */
class Remote: public Persisted
{
    private:

    unsigned long expiry = millis();
    SoftwareSerial btSerial{BT_PIN_RX, BT_PIN_TX};  // Rx, Tx

    // Structure holding the programmed state.
    struct Codes
    {   
        public:
        uint16_t address = 0;                       // Address and command for IR receiver.
        uint16_t command = 0;

        char     bluetoothCommand = 0x00;           // Bluetooth commands.
    };

    uint16_t flags = 0xffff;                        // All 16 inputs are high by default.
    Codes inputCodes[INPUT_PIN_MAX];                // The programmed input codes.
    

    public:

    /** Constructor.
     */
    Remote(uint16_t aBase) : Persisted(aBase)
    {
        size = sizeof(inputCodes);
    }


    /** Initialise.
     */
    void init(boolean aFirstRun)
    {
        if (aFirstRun)
        {
            // First run, initialise the IR codes.
            for (int i = 0; i < INPUT_PIN_MAX; i++)
            {
                inputCodes[i].address = 0;
                inputCodes[i].command = 0;
                inputCodes[i].bluetoothCommand = ' ';
            }
            EEPROM.put(getBase(), inputCodes);
        }
        else
        {
            // Load the saved IR codes.
            EEPROM.get(getBase(), inputCodes);
        }

        // Configure the on-board LED pin for output
        pinMode(LED_BUILTIN, OUTPUT);       

        // Initialise the IR Receiver.
        IrReceiver.begin(IR_PIN, ENABLE_LED_FEEDBACK);
        IrReceiver.printActiveIRProtocols(&Serial);

        // Initialise Bluetooth (Default speed of HC-05).
        btSerial.begin(9600);

        programIrCodes();
    }


    /** Update the state.
     */
    void update()
    {
        irUpdate();
        btUpdate();
    }


    /** Get the flags.
     */
    uint16_t getFlags()
    {
        return flags;
    }


    private:

    /** Update IR inputs.
     */
    void irUpdate()
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
                    auto tDecodedRawData = IrReceiver.decodedIRData.decodedRawData;     // uint32_t on 8 and 16 bit CPUs and uint64_t on 32 and 64 bit CPUs
                    Serial.print(F("Raw data received are 0x"));
                    Serial.println(tDecodedRawData);
                }

                // if (IrReceiver.decodedIRData.protocol != UNKNOWN)
                {
                    // The info output for a successful receive
                    IrReceiver.printIRResultShort(&Serial);
                    IrReceiver.printIRSendUsage(&Serial);
                }
            }
            for (uint16_t pin = 0; pin < INPUT_PIN_MAX; pin++)
            {
                if (   (inputCodes[pin].address == IrReceiver.lastDecodedAddress)
                    && (inputCodes[pin].command == IrReceiver.lastDecodedCommand))
                {
                    flags &= ~(1 << pin);
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


    /** Update BT inputs.
     */
    void btUpdate()
    {
        if (btSerial.available())
        {
            char ch = btSerial.read();
            for (uint16_t pin = 0; pin < INPUT_PIN_MAX; pin++)
            {
                if (ch == inputCodes[pin].bluetoothCommand)
                {
                    flags &= ~(1 << pin);
                }
            }

        }
    }


    /** Program IR codes */
    void programIrCodes()
    {
        expiry = millis() + DELAY_PROGRAM;
        uint8_t pin = 0;

        flash(pin, DELAY_BLINK_LONG);

        // Loop waiting for a programming event.
        while (   (pin < INPUT_PIN_MAX)
               && (expiry < millis()))
        {
            flash(1, DELAY_WAITING);

            if (IrReceiver.decode())
            {
                inputCodes[pin].address = IrReceiver.lastDecodedAddress;
                inputCodes[pin].command = IrReceiver.lastDecodedCommand;
                inputCodes[pin].bluetoothCommand = 0x00;
                flash(++pin, DELAY_BLINK);
                IrReceiver.resume();
                expiry = millis() + DELAY_PROGRAM;
            }

            if (btSerial.available())
            {
                char ch = btSerial.read();
                if (   (ch >= 0x20)
                    && (ch <= 0x7e))
                {
                    inputCodes[pin].address = 0;
                    inputCodes[pin].command = 0;
                    inputCodes[pin].bluetoothCommand = ch;
                    flash(++pin, DELAY_BLINK);
                    expiry = millis() + DELAY_PROGRAM;
                }
            }
        }

        // Update input codes if they've been changed.
        if (pin > 0)
        {
            EEPROM.put(getBase(), inputCodes);
        }
    }


    /** Flash the LED pin a number of times at the given interval.
     */
    void flash(uint8_t aFlashes, long aInterval)
    {
        while (aFlashes-- > 0)                      // n flashes.
        {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(aInterval);
            digitalWrite(LED_BUILTIN, LOW);
            delay(aInterval);
        }
    }
};


/** Singleton instance of Remote.
 *  In EEPROM immediately after the end of SystemMgr.
 */
Remote remote(systemMgr.getEnd());

#endif
