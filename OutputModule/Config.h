/** Config data.
 */
#ifndef _Config_h
#define _Config_h

#define DEBUG true


#define STEP_SERVO           50  // Delay (msecs) between steps of a Servo.
#define STEP_LED             10  // Delay (msecs) between steps of a LED.
#define STEP_FLASH           10  // Delay (msecs) between steps of flashes of a FLASH or BLINK.
#define STEP_FLICKER_MASK  0x0c  // Mask 2 bits (not right-most which are always 0) to gererate 1-in-4 chance of flicker.
#define MAX_PACE            124  // Maximum pace value.
#define PACE_STEPS          128  // Pace adjustment when converting to steps.

#define JUMPER_PINS           4  // Four jumpers.
#define IO_PINS               8  // Eight IO pins.
#define OUTPUT_BASE_PIN       4  // Outputs attached to this pin and the next 7 more.
#define ANALOG_PIN_FIRST     A0  // First analog pin. 
#define ANALOG_PIN_CUTOFF 0x200  // When usinging analog pin for digital purposes, cutoff at this value (half of full range 0-3ff).     


#define DELAY_MULTIPLIER  1000  // Multiply delay values by this amount (convert to seconds).


// The module jumper pins
const uint8_t jumperPins[JUMPER_PINS] = { 1, 0, A7, A6 };

// The digital IO pins.
const uint8_t ioPins[IO_PINS]         = { 3, 2, A3, A2, A1, A0, 13, 12 };


#endif
