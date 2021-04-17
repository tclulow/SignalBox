# Build notes

## Sketches

There are two sketches:
* SignalBox for the master module which runs on a Uno with an LCD shield.
* OutputModule for the output modules which run on Unos.

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



 
