# Build notes

## Sketches

There are two sketches:
* SignalBox for the master module which runs on a Uno with an LCD shield.
* OutputModule for the output modules which runs on Nanos.

Seven source files are duplicated across both sketches and are identical for both.

## Libraries

Libraries used by the sketches are:

Name              | Purpose 
----------------- | -------
EEPROM            | Reading and writing to EEPROM memory.
Wire              | To handle i2c communications. 
Servo             | To control Servos. 
LiquidCrystal     | For driving an LCD shield attached to the Uno.
LiquidCrystal_I2C | For driving an LCD attached by i2c.

The LiquidCrystal_I2C library can be removed by editing the LCD_I2C flag in the Config.h files.

## Configuration

Some aspects of the code can be modified by adjusting configuration constants defined in the Config.h files.
Note, there are two such files, one for each sketch and they're identical.

Parameter    | Purpose
------------ | -------
LCD_I2C      | Disable the I2C LCD code.
SERIAL_SPEED | Specify the speed that all serial IO should run at.

There are also various tuning parameters that can be adjusted here.

## PCBs

There are two version of the output module PCB. The original takes a Uno on a daughter board, the new one uses a DIP ATmega328 chip.

### Old PCB

The old PCB is easy to program using the Arduino IDE and a USB cable. 

The software for the Unos is designed for the original EzyBus PCBs which had the address jumpers on pins 1, 0, A7 and A6. The code normally ignores the digital pins one and zero as these are also used for serial-IO and setting jumpers here will disable the comms. The code can be adjusted to use these pins but then the Nano must be (re)programmed out of the PCB as the jumpers on these pins (J1 and J2) will disable programming on the PCB.

If J1 and J2 are disabled, then only J3 and J4 are used and will set the ID to 0, 4, 8 or C (Hexadecimal). For more than four modules either enable the IO pins for J1 and J2 or set the modules to use the software-allocated ID as described in the manual.

### New PCB

The new PCB  requires some additional hardware to program. A Serial-TTL converter and a few external components to make the ATmega328 enter programming mode.

If using the new EzyBus PCBs or kits, then the jumpers have been moved to (servo) pins 4, 5, 6 and 7. The Config.h files must be edited to reflect this.
Change this:

    const uint8_t jumperPins[JUMPER_PINS] = { 0xff, 0xff, A7, A6 };
    
to this:

    const uint8_t jumperPins[JUMPER_PINS] = { 4, 5, 6, 7 };

Note that these pins are shared with the outputs (the software reads these pins as inputs to read the jumpers at start-up, then configures them as outputs to drive the attached hardware (Servos, LEDs etc). If no jmpers are set, then this will give the module a hardware ID of 0xf (ie all jumpers default to high) but if a sufficiently low-impedance output is connected (and a LED with dropper resistor is often low enough) then these pins will read low (and change the i2c ID accordingly).

Either ensure that only Servos are connected to outputs 4, 5, 6, and 7, or use software-allocated ID as described in the manual.
If using software-allocation, perform this operation with no outputs connected and seperately from the rest of the system if the default ID (0xf) would clash with an existing module.



 
