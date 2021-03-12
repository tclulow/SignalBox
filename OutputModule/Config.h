/** Config data.
 *
 *
 *  (c)Copyright Tony Clulow  2021  tony.clulow@pentadtech.com
 *
 *  This work is licensed under the:
 *      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 *      http://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 *   For commercial use, please contact the original copyright holder(s) to agree licensing terms
 */
#ifndef _Config_h
#define _Config_h


#define SERIAL_SPEED   19200    // Speed of the serial port.

#define DELAY_START              2000   // Pause during start-up to avoid swamping Serial IO.
#define DELAY_BLINK               250   // Blink interval when showing version number.
#define DELAY_BLINK_LONG          750   // Blink to show zero, or gap between sections.

#define DELAY_READ               2000   // Delay in msecs when the LCD may need to be read by an operator.
#define DELAY_BUTTON_WAIT          50   // Delay when waiting for button state to change - debounce.
#define DELAY_BUTTON_DELAY        250   // Delay before auto-repeating button.
#define DELAY_BUTTON_REPEAT       100   // Auto-repeat button when held continuously.

#define DELAY_MULTIPLIER        1000L   // Multiply delay values by this amount (convert to seconds).

#define STEP_INPUT_SCAN            50   // Steps in msecs between scans of the input switches.
#define STEP_HEARTBEAT            200   // Steps in msecs between changes of the heartbeat indicator.
#define STEP_SERVO                 25   // Delay (msecs) between steps of a Servo.
#define STEP_LED                    5   // Delay (msecs) between steps of a LED.
#define STEP_FLASH                 10   // Delay (msecs) between steps of flashes of a FLASH or BLINK.

#define MAX_PACE                  124   // Maximum pace value.
#define PACE_STEPS                128   // Pace adjustment when converting to steps.

#define SIGNAL_PAUSE_DELAY        250   // Max msec delay when raising a signal.
#define SIGNAL_PAUSE_RESTART      500   // Max msec delay when restarting to raise a signal.
#define SIGNAL_PAUSE_PERCENTAGE    33   // Percentage of travel a Signal may fall.
#define SIGNAL_BOUNCE_CHANCE       66   // Percentage chance a signal may bounce.
#define SIGNAL_BOUNCE_PERCENTAGE   15   // Percentage of travel a Signal may bounce.

#define RANDOM_HI_CHANCE           60   // Chance that a RANDOM Hi output illuminates its LED.
#define RANDOM_LO_CHANCE           40   // Chance that a RANDOM Lo output illuminates its LED.

#define JUMPER_PINS                 4   // Four jumpers.
#define IO_PINS                     8   // Eight IO pins.
#define OUTPUT_BUILTIN_PIN          6   // IO Output 6 is pin 13, the LED_BUILTIN.
#define ANALOG_PIN_FIRST           A0   // First analog pin. 
#define ANALOG_PIN_CUTOFF       0x200   // When usinging analog pin for digital purposes, cutoff at this value (half of full range 0-3ff).     


#if !MASTER

// The module jumper pins
const uint8_t jumperPins[JUMPER_PINS] = { 1, 0, A7, A6 };

// The signal IO pins.
const uint8_t sigPins[IO_PINS]        = { 4, 5, 6, 7, 8, 9, 10, 11 };

// The digital IO pins.
const uint8_t ioPins[IO_PINS]         = { 3, 2, A3, A2, A1, A0, 13, 12 };


#endif

#endif
