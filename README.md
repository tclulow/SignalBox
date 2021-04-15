# SignalBox

SignalBox is software for controlling model railway signals, points and other accessories. It doesn’t attempt to control the running of trains at all. 

It use the same hardware as the EzyBus system: An Arduino Uno with LCD shield, Arduino Nanos to provide outputs (to Servos, LEDs etc) and MCP23-17 input expanders to read switches and buttons, all connected by an I2C bus. Although it uses exactly the same hardware as EzyBus, all the software is completely new.

It is positioned as a half-way house between the EzyBus system, and a CBUS one.

There is no need for a computer, all configuration can be done using the LCD panel. A computer can be connected to save and/or restore the configuration if desired.

## Features
Features of the system are (in brief):

* Multiple input types: toggle switches (SPST) and intermittent (non-latching) push buttons.
* Multiple inputs can be operated simultaneously.
* Multiple outputs can be operated by a single input - eg crossovers.
* Multiple inputs can operate the same output(s) - so multiple mimic-panels can control the same or similar outputs.
* Multiple output types, Servos, and various digital IOs including PWM.
* Servos, with configurable sweep and speed, with attached digital IO that switches at the mid-point of the servo travel.
* Signal “bounce” and “stutter” for semaphore signals.
* Variable-intensity LEDS (including fading) using PWM.
* Some other output types, eg flashing (varying speeds) or flickering LEDs.
* Four-aspect signals.
* Three-aspect UK road traffic lights.
* Three-aspect non-UK road traffic lights.
* Random outputs that go on or off at unpredictable intervals.
* Where multiple outputs are controlled by an input, the ability to delay the outputs, so for example crossing gates can operate one after the other.
* Automatic reset of inputs after a time delay (for signals that go red/danger with input from a TOTI but then revert to green sometime later).
* Interlocks that prevent certain outputs operating if other outputs are set incorrectly.

## Hardware
The system uses the EzyBus hardware, either the MERG kits (22 & 23) or self-assembled using the PCBs (kits 922 and 923) built exactly as described in the EzyBus manual.

There are two versions of the PCB kits, the older ones use an Arduino Nano, the new ones an Atmega328 chip (the same as found on a Nano) but don’t include a serial interface so programming is via the ICSP header or with a USB-TTL serial cable or adapter.

The software must be programmed onto the Uno (via the USB) and the Nanos (which is more difficult with the new kits). With a serial cable you can use either the Arduino IDE or the avrdude program directly.

Note that EzyBus numbers its input and output modules and their pins from 1 to 8 or 1 to 16 (0x10 hexadecimal). This software numbers from zero to 7 or 15 (0xF hexadecimal).

