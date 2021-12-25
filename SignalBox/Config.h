/** Config data.
 *  @file
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

#ifndef Config_h
#define Config_h


/** Configuration constants.
 */
 
// Serial IO
#define SERIAL_SPEED            19200   // Speed of the serial port.


// Attached LCD displays.
#define LCD_SHIELD              false   // Assume LCD shield present (or not). If false, use LCD_SHIELD_DETECT_PIN.
#define LCD_SHIELD_DETECT_PIN      11   // Use this pin (must be low) to detect presence of LCD shield. If zero, don't detect.
#define LCD_I2C                  true   // Include code for LCD connected by I2C.

#define LCD_RS                      8   // LCD shield pins.
#define LCD_ENABLE                  9
#define LCD_D4                      4
#define LCD_D5                      5
#define LCD_D6                      6
#define LCD_D7                      7

#define INTERLOCK_WARNING_PIN      13   // When interlocks prevent an operation, set this pin high. If zero, no warning is shown.
#define INTERLOCK_WARNING_TIME   2000   // Duration (msecs) of interlock warning.
#define INTERLOCK_BUZZER_PIN       12   // Buzzer pin to sound interlock warning on (about 900+ bytes of extra code). Zero disables buzzer tones.
#define INTERLOCK_BUZZER_FREQ1    196   // Frequency of warning first tone.
#define INTERLOCK_BUZZER_FREQ2    131   // Frequency of warning first tone.
#define INTERLOCK_BUZZER_TIME1    200   // Duration (msecs) of interlock warning first tone.
#define INTERLOCK_BUZZER_TIME2    300   // Duration (msecs) of interlock warning second tone.


// Delays
#define DELAY_BLINK               250   // Blink interval when showing version number.
#define DELAY_BLINK_LONG          750   // Blink to show zero, or gap between sections.
#define DELAY_READ               2000   // Delay in msecs when the LCD may need to be read by an operator.
#define DELAY_FAIL               5000   // Delay for failure messages.

#define DELAY_BUTTON_WAIT          50   // Delay when waiting for button state to change - debounce.
#define DELAY_BUTTON_DELAY        250   // Delay before auto-repeating button.
#define DELAY_BUTTON_REPEAT       100   // Auto-repeat button when held continuously.

#define DELAY_MULTIPLIER        1000L   // Multiply delay values by this amount (convert to seconds).


// Steps (all in msecs)
#define STEP_HARDWARE_SCAN     10000L   // Scan for new hardware every 10 seconds.
#define STEP_INPUT_SCAN           50L   // Scan the input switches.
#define STEP_GATEWAY             100L   // Scan the gateway.
#define STEP_HEARTBEAT           200L   // Refresh heartbeat indicator.
#define STEP_SERVO                25L   // Step Servos.
#define STEP_LED                   5L   // Step LEDs.
#define STEP_FLASH                10L   // Step flashers (FLASH or BLINK).


// Operational constants
#define MAX_PACE                  124   // Maximum pace value.
#define PACE_STEPS                128   // Pace adjustment when converting to steps.

#define SIGNAL_PAUSE_CHANCE        80   // Percentage chance a signal may pause when being raised.
#define SIGNAL_PAUSE_DELAY        250   // Max msec delay when raising a signal.
#define SIGNAL_PAUSE_RESTART      500   // Max msec delay when restarting to raise a signal.
#define SIGNAL_PAUSE_PERCENTAGE    33   // Percentage of travel a Signal may fall.
#define SIGNAL_BOUNCE_CHANCE       66   // Percentage chance a signal may bounce.
#define SIGNAL_BOUNCE_PERCENTAGE   15   // Percentage of travel a Signal may bounce.

#define LED_FLICKER_CHANCE         25   // Percentage chance flickering LED will switch.

#define RANDOM_HI_CHANCE           60   // Chance that a RANDOM Hi output illuminates its LED.
#define RANDOM_LO_CHANCE           40   // Chance that a RANDOM Lo output illuminates its LED.

#define PWM_TICK                   -1   // PWM tick adjustment to give maximum frequency to PWM signal.
#define PWM_INC                  0x10   // Increment between pins to ensure even distribution.

#define JUMPER_PINS                 4   // Four jumpers.
#define IO_PINS                     8   // Eight IO pins.
#define OUTPUT_BUILTIN_PIN          6   // ioPins 6 is Arduino pin 13, the LED_BUILTIN.
#define ANALOG_PIN_FIRST           A0   // First analog pin.
#define ANALOG_PIN_LAST            A7   // Last analog pin.
#define ANALOG_PIN_CUTOFF       0x200   // When usinging analog pin for digital purposes, cutoff at this value (half of full range 0-3ff).


#if SB_CONTROLLER

// Alternate pins that can be used to control the menus. First entry unused.
const uint8_t BUTTON_PINS[] = { 0xff, A1, A2, A3, 2, 3 };


#elif SB_OUTPUT_MODULE

// The module jumper pins. 0xff means don't use.
// const uint8_t jumperPins[JUMPER_PINS] = { 1, 0, A7, A6 };
const uint8_t jumperPins[JUMPER_PINS] = { 0xff, 0xff, A7, A6 };

// Alternate jumper pins for new output module.
// const uint8_t jumperPins[JUMPER_PINS] = { 4, 5, 6, 7 };

// The signal IO pins.
const uint8_t sigPins[IO_PINS]        = { 4, 5, 6, 7, 8, 9, 10, 11 };

// The digital IO pins.
const uint8_t ioPins[IO_PINS]         = { 3, 2, A3, A2, A1, A0, 13, 12 };

#endif

#endif
