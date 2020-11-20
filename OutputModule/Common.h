/** Common data.
 */
#ifndef _Common_h
#define _Common_h


#define DELAY_START   2000          // Pause during start-up to avoid swamping Serial IO.
#define DELAY_BLINK   250           // Blink interval when showing version number.


// i2c node numbers.
#define DEFAULT_I2C_CONTROLLER_ID   0x10    // Controller ID.
#define DEFAULT_I2C_INPUT_BASE_ID   0x20    // Input nodes' base ID.
#define DEFAULT_I2C_OUTPUT_BASE_ID  0x50    // Output nodes' base ID.


// Output information that's shared with the output module.
#define OUTPUT_TYPE_MASK       0x0f   // Only four bits are used for the type of output.
#define OUTPUT_TYPE_SERVO      0x00   // Output is a servo.
#define OUTPUT_TYPE_SIGNAL     0x01   // Output is a signal.
#define OUTPUT_TYPE_LED        0x02   // Output is a LED or other IO device.
#define OUTPUT_TYPE_MAX        0x03   // Limit of output types.
#define OUTPUT_TYPE_NONE       0x0f   // Placeholder to mark "no type".
#define OUTPUT_TYPE_SHIFT         3   // Output type is shifted 3 bits when communicating to the OutputModule.
#define OUTPUT_PIN_MASK        0x07   // Mask to get output pin within a node.


/** Show version number by flashing LED
 *  and reporting it on Serial output.
 */
void initialise()
{
    pinMode(LED_BUILTIN, OUTPUT);   // Configure the on-board LED pin for output
    delay(DELAY_START);             // Wait to avoid programmer conflicts.
    Serial.begin(115200);           // Serial IO.
    
    for (int ind = 0;ind < strlen_P(M_VERSION); ind++)
    {
        char ch = pgm_read_byte_near(M_VERSION + ind);
        if (ch >= '1' && ch <= '9')
        {
            while (ch-- >= '1')
            {
                digitalWrite(LED_BUILTIN, HIGH);
                delay(DELAY_BLINK);
                digitalWrite(LED_BUILTIN, LOW);
                delay(DELAY_BLINK);
            }
        }
        else
        {
            delay(DELAY_BLINK * 2);
        }
    }

    Serial.print(PGMT(M_SOFTWARE));
    Serial.print(M_SPACE);
    Serial.print(PGMT(M_VERSION));
    Serial.print(M_SPACE);
    Serial.print(PGMT(M_VERSION_DATE));
    Serial.println();
}

#endif
