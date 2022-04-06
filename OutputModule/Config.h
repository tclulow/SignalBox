/** Config data.
 *  @file
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


// Remove all debug code using this definition.
// #define isDebug(x) (false)

// Include serial handlers
#define SERIAL_CMRI             true    // Include serial CMRI processing.
#define SERIAL_COMMAND          true    // Include serial command processing.

// Gateway I2C node.
#define I2C_GATEWAY_ID          0x11    // Gateway ID (normally 0x11) or 0 to disable gateway code.

// Attached LCD displays.
#define LCD_I2C                 true    // Include code for LCD connected by I2C.
#define LCD_SHIELD              false   // Assume LCD shield present (or not). If false, use LCD_SHIELD_DETECT_PIN.
#define LCD_SHIELD_DETECT_PIN   11      // Use this pin (must be low) to detect presence of LCD shield. If zero, don't detect.


/** Configuration constants.
 */
 
// Serial IO
const long    SERIAL_SPEED             = 115200;   // Speed of the serial port.


// LCD shield pins.
const uint8_t LCD_RS                   =     8;
const uint8_t LCD_ENABLE               =     9;
const uint8_t LCD_D4                   =     4;
const uint8_t LCD_D5                   =     5;
const uint8_t LCD_D6                   =     6;
const uint8_t LCD_D7                   =     7;


// Interlocks
const uint8_t INTERLOCK_WARNING_PIN    =    13;   // When interlocks prevent an operation, set this pin high. If zero, no warning is shown.
const long    INTERLOCK_WARNING_TIME   =  2000;   // Duration (msecs) of interlock warning.
const uint8_t INTERLOCK_BUZZER_PIN     =    12;   // Buzzer pin to sound interlock warning on (about 900+ bytes of extra code). Zero disables buzzer tones.
const int     INTERLOCK_BUZZER_FREQ1   =   196;   // Frequency of warning first tone.
const int     INTERLOCK_BUZZER_FREQ2   =   131;   // Frequency of warning second tone.
const long    INTERLOCK_BUZZER_TIME1   =   200;   // Duration (msecs) of interlock warning first tone.
const long    INTERLOCK_BUZZER_TIME2   =   300;   // Duration (msecs) of interlock warning second tone.


// Delays
const long    DELAY_BLINK              =   250;   // Blink interval when showing version number.
const long    DELAY_BLINK_LONG         =   750;   // Blink to show zero, or gap between sections.
const long    DELAY_READ               =  2000;   // Delay in msecs when the LCD may need to be read by an operator.
const long    DELAY_FAIL               =  5000;   // Delay for failure messages.

const long    DELAY_BUTTON_WAIT        =    50;   // Delay when waiting for button state to change - debounce.
const long    DELAY_BUTTON_DELAY       =   250;   // Delay before auto-repeating button.
const long    DELAY_BUTTON_REPEAT      =   100;   // Auto-repeat button when held continuously.

const long    DELAY_MULTIPLIER         =  1000;   // Multiply delay values by this amount (convert to seconds).


// Steps (all in msecs)
const long    STEP_HARDWARE_SCAN       = 10000;   // Scan for new hardware every 10 seconds.
const long    STEP_INPUT_SCAN          =    50;   // Scan the input switches.
const long    STEP_GATEWAY             =   100;   // Scan the gateway.
const long    STEP_HEARTBEAT           =   200;   // Refresh heartbeat indicator.
const long    STEP_SERVO               =    25;   // Step Servos.
const long    STEP_LED                 =     5;   // Step LEDs.
const long    STEP_FLASH               =    10;   // Step flashers (FLASH or BLINK).


// Operational constants
const uint8_t PACE_STEPS               =   128;   // Pace adjustment when converting to steps.

const long    SIGNAL_PAUSE_CHANCE      =    80;   // Percentage chance a signal may pause when being raised.
const long    SIGNAL_PAUSE_DELAY       =   250;   // Max msec delay when raising a signal.
const long    SIGNAL_PAUSE_RESTART     =   500;   // Max msec delay when restarting to raise a signal.
const long    SIGNAL_PAUSE_PERCENTAGE  =    33;   // Percentage of travel a Signal may fall.
const long    SIGNAL_BOUNCE_CHANCE     =    66;   // Percentage chance a signal may bounce.
const long    SIGNAL_BOUNCE_PERCENTAGE =    15;   // Percentage of travel a Signal may bounce.

const long    LED_FLICKER_CHANCE       =    25;   // Percentage chance flickering LED will switch.

const long    RANDOM_HI_CHANCE         =    60;   // Chance that a RANDOM Hi output illuminates its LED.
const long    RANDOM_LO_CHANCE         =    40;   // Chance that a RANDOM Lo output illuminates its LED.

const long    PWM_TICK                 =     1;   // PWM tick adjustment to give maximum frequency to PWM signal.
const long    PWM_INC                  =  0x10;   // Increment between pins to ensure even distribution.

const uint8_t JUMPER_PINS              =     4;   // Four jumpers.
const uint8_t IO_PINS                  =     8;   // Eight IO pins.
const uint8_t OUTPUT_BUILTIN_PIN       =     6;   // ioPins 6 is Arduino pin 13, the LED_BUILTIN.
const uint8_t ANALOG_PIN_FIRST         =    A0;   // First analog pin.
const uint8_t ANALOG_PIN_LAST          =    A7;   // Last analog pin.
const int     ANALOG_PIN_CUTOFF        = 0x200;   // When using analog pin for digital purposes, cutoff at this value (half of full range 0-3ff).


#if SB_CONTROLLER

// Alternate pins that can be used to control the menus. First entry unused.
// Unused, Select, Left, Down, Up, right.
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
