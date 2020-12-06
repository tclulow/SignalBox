/** Common data.
 */

#ifndef _Common_h
#define _Common_h


#define SERIAL_SPEED  115200    // Speed of the serial port.
#define DELAY_START     2000    // Pause during start-up to avoid swamping Serial IO.
#define DELAY_BLINK      250    // Blink interval when showing version number.


// Useful characters
const char CHAR_SPACE   = ' ';
const char CHAR_TAB     = '\t';
const char CHAR_NEWLINE = '\n';
const char CHAR_RETURN  = '\r';
const char CHAR_NULL    = 0;

const char CHAR_HASH    = '#';
const char CHAR_DOT     = '.';
const char CHAR_COLON   = ':';
const char CHAR_LEFT    = '<';
const char CHAR_RIGHT   = '>';
const char CHAR_STAR    = '*';
const char CHAR_ZERO    = '0';
const char CHAR_NINE    = '9';
const char CHAR_LOWER_A = 'a';


// i2c node numbers.
#define DEFAULT_I2C_CONTROLLER_ID   0x10    // Controller ID.
#define DEFAULT_I2C_INPUT_BASE_ID   0x20    // Input nodes' base ID.
#define DEFAULT_I2C_OUTPUT_BASE_ID  0x50    // Output nodes' base ID.


/** Show version number by flashing LED
 *  and reporting it on Serial output.
 */
void initialise()
{
    pinMode(LED_BUILTIN, OUTPUT);   // Configure the on-board LED pin for output
    delay(DELAY_START);             // Wait to avoid programmer conflicts.
    Serial.begin(SERIAL_SPEED);     // Serial IO.
    
    for (int ind = 0; ind < strlen_P(M_VERSION); ind++)
    {
        char ch = pgm_read_byte_near(M_VERSION + ind);
        if (ch >= CHAR_ZERO && ch <= CHAR_NINE)
        {
            while (ch-- > CHAR_ZERO)
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
    Serial.print(CHAR_SPACE);
    Serial.print(PGMT(M_VERSION));
    Serial.print(CHAR_SPACE);
    Serial.print(PGMT(M_VERSION_DATE));
    Serial.println();
}

#endif
