/** Config data.
 *
 *
 *  (c)Copyright Tony Clulow  2021  tony.clulow@pentadtech.com
 *
 *  This work is licensed under the:
 *      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 *      http://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 *  For commercial use, please contact the original copyright holder(s) to agree licensing terms.
 *
 *
 *  Pin usage:
 *  
 *  Pin     Master(Uno)                 Output(Nano)
 *  D0      Serial Rx.                  Serial Tx.      Could be jumper J2.
 *  D1      Serial Tx.                  Serial Rx.      Could be jumper J1.
 *  D2      Alternate up button.        IO pin 1.
 *  D3      Alternate right button.     IO pin 0.
 *  D4      LCD shield data D4.         Servo pin 0.    Alternate jumper J1.
 *  D5      LCD shield data D5.         Servo pin 1.    Alternate jumper J2.
 *  D6      LCD shield data D6.         Servo pin 2.    Alternate jumper J3.
 *  D7      LCD shield data D7.         Servo pin 3.    Alternate jumper J4.
 *  D8      LCD shield rs.              Servo pin 4.
 *  D9      LCD shield enable.          Servo pin 5.
 *  D10     LCD shield backlight.       Servo pin 6.
 *  D11     LCD shield detect.          Servo pin 7.
 *  D12     Not used.                   IO pin 7.
 *  D13     Flash firmare version.      IO pin 6.       Also flash firmware version.
 *  
 *  A0      LCD shield buttons.         IO pin 5.
 *  A1      Alternate select button.    IO pin 4.
 *  A2      Alternate left button.      IO pin 3.
 *  A3      Alternate down button.      IO pin 2.
 *  A4      I2C SDA.                    I2C SDA.
 *  A5      I2C SCL.                    I2C SCL.
 *  A6      Not available.              Jumper J4
 *  A7      Not available.              Jumper J3
 */
 
#ifndef Config_h
#define Config_h


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


// i2c node numbers.
#define I2C_CONTROLLER_ID        0x10   // Controller ID.
#define I2C_INPUT_BASE_ID        0x20   // Input nodes' base ID.
#define I2C_OUTPUT_BASE_ID       0x50   // Output nodes' base ID.
#define I2C_MODULE_ID_JUMPERS    0xff   // Use jumpers to decide module ID.

#define I2C_LCD_LO               0x27   // Range of IDs to scan for LCD I2C device.
#define I2C_LCD_HI               0x3F


// Delays
#define DELAY_BLINK               250   // Blink interval when showing version number.
#define DELAY_BLINK_LONG          750   // Blink to show zero, or gap between sections.
#define DELAY_READ               2000   // Delay in msecs when the LCD may need to be read by an operator.
#define DELAY_FAIL               5000   // Delay for failure messages.

#define DELAY_BUTTON_WAIT          50   // Delay when waiting for button state to change - debounce.
#define DELAY_BUTTON_DELAY        250   // Delay before auto-repeating button.
#define DELAY_BUTTON_REPEAT       100   // Auto-repeat button when held continuously.

#define DELAY_MULTIPLIER        1000L   // Multiply delay values by this amount (convert to seconds).


// Steps
#define STEP_HARDWARE_SCAN     10000L   // Re-scan for new hardware every 10 seconds.
#define STEP_INPUT_SCAN           50L   // Steps in msecs between scans of the input switches.
#define STEP_HEARTBEAT           200L   // Steps in msecs between changes of the heartbeat indicator.
#define STEP_SERVO                25L   // Delay (msecs) between steps of a Servo.
#define STEP_LED                   5L   // Delay (msecs) between steps of a LED.
#define STEP_FLASH                10L   // Delay (msecs) between steps of flashes of a FLASH or BLINK.


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


#if MASTER

// Alternate pins that can be used to control the menus. First entry unused.
const uint8_t BUTTON_PINS[] = { 0xff, A1, A2, A3, 2, 3 };

#else

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
