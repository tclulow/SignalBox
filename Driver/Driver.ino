 // Multi Sweep, Duane B
// Using the servo library created by Michael Margolis 
// to control 12 Servos with one Arduino Uno

#include <Servo.h>
// Sample sketch for driving 12 Servos from an Arduino UNO, servos are attached to digital pins 2,3,4,5,6,7,8,9,10,11,12,13

#define CONNECTED_SERVOS 12

// macro just adds two - the first servo is attached to digital pin 2, this gives us upto 12 servos - digital 2 to 13
#define SERVO_TO_PIN(x) (x+2)

Servo myServos[CONNECTED_SERVOS];

#define COUNT_DOWN -1
#define COUNT_UP +1
#define INCREMENT 10 // move in steps of 10 milliseconds

int nPulseWidth = DEFAULT_PULSE_WIDTH ; // 1500, defined in servo.h
int nDirection = COUNT_UP;

volatile unsigned long ulStart = 0;
volatile unsigned long ulStartToEnd = 0;

void setup()
{
  // attach the servos
  for(int nServo = 0;nServo < CONNECTED_SERVOS;nServo++)
  {
    myServos[nServo].attach(SERVO_TO_PIN(nServo));
  }
 
  // the library sets all servos to 1500 ms pulse width by default, this is center for a steering servo
  Serial.begin(9600);
  Serial.println("Completed setup"); 
}

void loop()
{
  delay(10);  // give the servos time to move after each update
 
  if(ulStartToEnd)
  {
    Serial.println(ulStartToEnd);
    ulStartToEnd = 0;
  }
 
  nPulseWidth += nDirection * INCREMENT;
 
  if(nPulseWidth >= 2000)
  {
    nPulseWidth = 2000;
    nDirection = COUNT_DOWN;
  }
 
  if(nPulseWidth <= 1000)
  {
    nPulseWidth = 1000;
    nDirection = COUNT_UP;
  }
 
  for(int nServo = 0;nServo < CONNECTED_SERVOS;nServo++)
  {
    myServos[nServo].writeMicroseconds(nPulseWidth);
  }
}
