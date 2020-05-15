// EzyBus Output Module
// Author : Davy Dick
// Created : 22/03/18
// Last updated : 13/4/19
// I2C slave for 8 servos 
// and 8 relays, LEDs. etc.

#include <Wire.h>
#include <VarSpeedServo.h> 

VarSpeedServo  servo1;   
VarSpeedServo  servo2;   
VarSpeedServo  servo3;    
VarSpeedServo  servo4;    
VarSpeedServo  servo5;   
VarSpeedServo  servo6;   
VarSpeedServo  servo7;   
VarSpeedServo  servo8;  

volatile byte pin;
volatile byte mode;        // Servo, Bounce or I/O
volatile byte angle;       // Servo angles of 2 to 180, 0 and 1 reserved for I/O
volatile byte servo_speed; // 0=full speed, 1-255 slower to faster 
                           // speed differences above 127 can't be noticed
byte switch_state;  
int J3, J4;                
volatile byte offset=0;
volatile int  IO_pin[8]={3,2,A3,A2,A1,A0,13,12};
volatile byte ModuleAddress=80;   // Unique address for first I2C slave module
//volatile unsigned long looper;


void setup(){
  pinMode(4,OUTPUT);   // Servo 1
  pinMode(5,OUTPUT);   // Servo 2
  pinMode(6,OUTPUT);   // Servo 3
  pinMode(7,OUTPUT);   // Servo 4
  pinMode(8,OUTPUT);   // Servo 5
  pinMode(9,OUTPUT);   // Servo 6
  pinMode(10,OUTPUT);  // Servo 7
  pinMode(11,OUTPUT);  // Servo 8
  servo1.attach(4);  delay(20);
  servo2.attach(5);  delay(20);
  servo3.attach(6);  delay(20);
  servo4.attach(7);  delay(20);
  servo5.attach(8);  delay(20); 
  servo6.attach(9);  delay(20);
  servo7.attach(10); delay(20); 
  servo8.attach(11); 
  pinMode(1, INPUT);   // J1
  pinMode(0, INPUT);   // J2
  pinMode(A6,INPUT);   // J3
  pinMode(A7,INPUT);   // J4
  pinMode(3,OUTPUT);   // Digital I/O 1 
  pinMode(2,OUTPUT);   // Digital I/O 2
  pinMode(A3,OUTPUT);  // Digital I/O 3
  pinMode(A2,OUTPUT);  // Digital I/O 4
  pinMode(A1,OUTPUT);  // Digital I/O 5
  pinMode(A0,OUTPUT);  // Digital I/O 6
  pinMode(13,OUTPUT);  // Digital I/O 7
  pinMode(12,OUTPUT);  // Digital I/O 8
  J3 = analogRead(A6);  
  if (J3 < 100) {J3 = 0;} else {J3 = 1;}
  J4 = analogRead(A7);
  if (J4 < 100) {J4 = 0;} else {J4 = 1;}
  offset = digitalRead(1) + 2*digitalRead(0) + 4*J3 + 8*J4;
  ModuleAddress = ModuleAddress + offset;  
  Wire.begin(ModuleAddress);    // Connect to I2C network as node 0x50 + offset
  Wire.onReceive(NewCommand);   // Execute NewCommand when data received from master
}

void loop(){
delay(100);
}

void NewCommand(int ignore)
{
  pin   = Wire.read();
  angle = Wire.read();
  servo_speed = Wire.read();
  switch_state= Wire.read();   
  switch(pin)
  {         // false means don't wait until rotation finishes
  case 0:
     servo1.write(angle, servo_speed, false);   break;
  case 1:
     servo2.write(angle, servo_speed, false);   break;
  case 2:
     servo3.write(angle, servo_speed, false);   break;
  case 3:
     servo4.write(angle, servo_speed, false);   break;
  case 4:
     servo5.write(angle, servo_speed, false);   break;
  case 5:
     servo6.write(angle, servo_speed, false);   break;
  case 6:
     servo7.write(angle, servo_speed, false);   break;
  case 7:
     servo8.write(angle, servo_speed, false);   break;
  }
  digitalWrite(IO_pin[pin],switch_state);
}
