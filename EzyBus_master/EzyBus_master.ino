/*  
    Created : 3 Mar 2018
    Last update : 23 Mar 2019
    Author :  Davy Dick  
    Program written for a Uno
    Controls up to 8 servos on each Nano slave
    Rotation transmitted from 2 to 180
    Values stored in EEPROM as:
              0         1          2         3    
    Pin1    mode    LEFTvalue  RIGHTvalue  speed  
              4         5          6         7               
    Pin2    mode    LEFTvalue  RIGHTvalue  speed   
    etc.
    
    Mode 0 - servos for points and animations
    Mode 1   semaphore bounce
    Mode 2   digital I/O - either 0 or 1  */
    
#include <LiquidCrystal.h>
#include <Wire.h>
#include <EEPROM.h>

#define    MasterAddress     99   // Unique address for the master
#define    PortA_direction 0x00   // Register addresses of the MCP23017
#define    PortA_pullups   0x0C
#define    PortB_direction 0x01
#define    PortB_pullups   0x0D
#define    dataportA       0x12
const byte switchBaseAddress = 0x20;   // Base address of the MCP23017 modules
const byte servoBaseAddress  = 0x50;   // Base address of the Nano modules   

unsigned int servoMap    = 0;   // 16 bits, one per address
byte     switchMap       = 0;   // 8 bits, one per address
byte     ModuleNum       = 0;   // Maximum of 16 Nano modules can connect, each with 8 outputs
byte     UserModuleNum   = 1;   // Module number shown on LCD
byte     PinNum          = 0;   // Output pins on servo modules
byte     UserPinNum      = 1;   // Pin number shown on LCD
byte     mode            = 0;   // Point, signal bounce or I/O
byte     LEFT            = 0;   // Stores LEFT value of selected servo
byte     RIGHT           = 0;   // Stores RIGHT value of selected servo
byte     value;                 // Servo angles of 2 to 180, 0 and 1 reserved for I/O
byte     servo_speed;
//byte   offset;                // global use offset
int      diff;                  // Difference between a new reading and the previous reading
byte     changed_pin;           // Which pin has changed state
int      Mem_Address;           // EEPROM storage address
byte     TargetModule;          //
uint16_t new_input=0;           //
uint16_t previous_check[8];     // 
byte     switch_state;          // Either 0 or 1 for I/O, other numbers are second value for semaphore bounce
int      tempo;
byte     error;
byte     mid_point;
int      left_val;
int      right_val;
int      up_val;
int      down_val;
int      select_val;
int      calibrate_pin = 11;    // Pin on shield ICSP, used for calibrating buttons
byte     high_part, low_part;   // Breakdown of key value into bytes for EEPROM storage
byte     noshow=0;
byte     pinOut;
unsigned long  currentMillis;
unsigned long  previousMillis; 

const char string_0[] PROGMEM  = "Calibrating ... ";
const char string_1[] PROGMEM  = "Press Left  ";
const char string_2[] PROGMEM  = "Press Right ";
const char string_3[] PROGMEM  = "Press Up    ";
const char string_4[] PROGMEM  = "Press Down  ";
const char string_5[] PROGMEM  = "Press Select";
const char string_6[] PROGMEM  = "Left   =";
const char string_7[] PROGMEM  = "Right  =";
const char string_8[] PROGMEM  = "Up     =";
const char string_9[] PROGMEM  = "Down   =";
const char string_10[] PROGMEM = "Select =";
const char string_11[] PROGMEM = "Shield Buttons";
const char string_12[] PROGMEM = "Calibrated";
const char string_13[] PROGMEM = "All servos are ";
const char string_14[] PROGMEM = "now centred    ";
const char string_15[] PROGMEM = "Press LEFT to  ";
const char string_16[] PROGMEM = "centre all servos";
const char string_17[] PROGMEM = "Up to proceed";
const char string_18[] PROGMEM = "Down to abort   ";
const char string_19[] PROGMEM = "Reset aborted";
const char string_20[] PROGMEM = "Updating system";
const char string_21[] PROGMEM = "Please wait.....";
const char string_22[] PROGMEM = "Sending to ";
const char string_23[] PROGMEM = " > Pin:";
const char string_24[] PROGMEM = " Mode:";
const char string_25[] PROGMEM = "  Value:";
const char string_26[] PROGMEM = " Speed:";
const char string_27[] PROGMEM = "  Switch:";
const char string_28[] PROGMEM = "Module=";
const char string_29[] PROGMEM = " Pin=";
const char string_30[] PROGMEM = "Mode=";
const char string_31[] PROGMEM = " Val=";
const char string_32[] PROGMEM = " Point   ";
const char string_33[] PROGMEM = " Signal  ";
const char string_34[] PROGMEM = " Digital output";
const char string_35[] PROGMEM = " Module ";
const char string_36[] PROGMEM = "   Address at ";
const char string_37[] PROGMEM = "----------------";
const char string_38[] PROGMEM = "Right btn val=";
const char string_39[] PROGMEM = "Up btn value =";
const char string_40[] PROGMEM = "Dn btn value =";
const char string_41[] PROGMEM = "Left btn val =";
const char string_42[] PROGMEM = "Select btn val=";
const char string_43[] PROGMEM = "Left Value ";
const char string_44[] PROGMEM = "Up Value ";
const char string_45[] PROGMEM = "+ - then SELECT";
const char string_46[] PROGMEM = "Right Value";
const char string_47[] PROGMEM = " Module# + - ";
const char string_48[] PROGMEM = "SELECT when done";
const char string_49[] PROGMEM = "Speed:     + -  ";
const char string_50[] PROGMEM = "< Test  Adopt > ";
const char string_51[] PROGMEM = " Sending to";
const char string_52[] PROGMEM = "Pin Number + -  ";
const char string_53[] PROGMEM = "Sp: L: R: R=sve ";
const char string_54[] PROGMEM = "Sp: Up: Dn:  Rsv";
const char string_55[] PROGMEM = "set  dgtl R=save";
const char string_56[] PROGMEM = "Settings saved";
const char string_57[] PROGMEM = "L=Point R=Signal";
const char string_58[] PROGMEM = "Up=I/O Down=Exit";
const char string_59[] PROGMEM = "Configure a pin";
const char string_60[] PROGMEM = "L=Proceed R=Exit";
const char string_61[] PROGMEM = "Mem Address = ";
const char string_62[] PROGMEM = "Values must be";
const char string_63[] PROGMEM = "centred round 90";
const char string_64[] PROGMEM = "     EzyBus";
const char string_65[] PROGMEM = "      v1.0";
const char string_66[] PROGMEM = "Centering servos";
const char string_67[] PROGMEM = "servos - wait";
const char string_68[] PROGMEM = "  ";
const char string_69[] PROGMEM = " R=save";
const char string_70[] PROGMEM = "Your system settings are:";
const char string_71[] PROGMEM = "Left: ";
const char string_72[] PROGMEM = "Right: ";
const char string_73[] PROGMEM = "Up:   ";
const char string_74[] PROGMEM = "Down: ";
const char string_75[] PROGMEM = "Signal going up";
const char string_76[] PROGMEM = "Signal Down";
const char string_77[] PROGMEM = "Down value";
const char string_78[] PROGMEM = "Point";
const char string_79[] PROGMEM = "Digital";


const char* const string_table[] PROGMEM = {
  string_0, string_1, string_2, string_3, string_4, string_5,string_6, string_7, string_8, string_9,
  string_10, string_11, string_12, string_13, string_14, string_15, string_16, string_17, string_18, string_19,
  string_20, string_21, string_22, string_23, string_24, string_25, string_26, string_27, string_28, string_29,
  string_30, string_31, string_32, string_33, string_34, string_35, string_36, string_37, string_38, string_39,
  string_40, string_41, string_42, string_43, string_44, string_45, string_46, string_47, string_48, string_49,
  string_50, string_51, string_52, string_53, string_54, string_55, string_56, string_57, string_58, string_59,
  string_60, string_61, string_62, string_63, string_64, string_65, string_66, string_67, string_68, string_69,
  string_70, string_71, string_72, string_73, string_74, string_75, string_76, string_77, string_78, string_79
};

char buffer[16];

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);  // Pins used for the LCD
//  D10 is used to control the LCD backlight
//  Pins D20 and D21 are used for I2C
//  Pin D1 is the Tx pin and D13 connects to the on-board LED
//  Pin D11 is used to decide whether to calibrate the LCD shield's buttons
//  So, 12 of the digital pins are already allocated, leaving other digital pins free for future
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5
#define btnFAIL   6

int button_key  = 0;     // Stores the value of any button press on the shield

void getMessage(int i)
{
 strcpy_P(buffer, (char*)pgm_read_word(&(string_table[i]))); 
}
void lcdSay(int i)
{
  getMessage(i);lcd.print(buffer);
}
void serialSay(int i)
{
 getMessage(i);Serial.print(buffer); 
}
void serialSayln(int i)
{
 getMessage(i);Serial.println(buffer);
}
//-------------------------------------------------------------------------
void configure_switches()
{ 
  for(int x=0; x<8; x++)    // Make all switch pins inputs
  {
     if (bitRead(switchMap,x)==1)
     {
       Wire.beginTransmission(switchBaseAddress + x); 
       Wire.write(PortA_direction);
       Wire.write(0xFF);
       Wire.endTransmission();
       Wire.beginTransmission(switchBaseAddress + x);  
       Wire.write(PortB_direction);
       Wire.write(0xFF);
       Wire.endTransmission();
     }
  }   
  for(int x=0; x<8; x++)   // Make all switch pins pullups
  {  
     if(bitRead(switchMap,x)==1)
     {
        Wire.beginTransmission(switchBaseAddress + x);
        Wire.write (PortA_pullups);
        Wire.write(0xFF);
        Wire.endTransmission();  
        Wire.beginTransmission(switchBaseAddress + x);
        Wire.write(PortB_pullups);
        Wire.write(0xFF);
        Wire.endTransmission();  
     }   
  }   
  Serial.println("Switches configured");  
}
//-------------------------------------------------------------------------
void calibrate()
{                                                  // Calibrating the LCD shield's buttons
   lcd.setCursor(0,0); lcdSay(0); delay(1000);     // "Calibrating"
   lcd.setCursor(0,1); lcdSay(1);                  // "Press Left" 
   do{left_val = analogRead(0); delay(100);}       // Keep reading buttons
      while(left_val>1000);                        // until you press one
   do{tempo=analogRead(0); delay(100);} 
      while(tempo < 1000);                         // wait for finger off button
   delay(300);
   lcd.setCursor(0,1);lcdSay(2);                   //"Press Right"  
   do{right_val = analogRead(0); delay(100); }     // Keep reading buttons
      while(right_val>1000 );                      // until you press one
   do{tempo=analogRead(0); delay(100);} 
      while(tempo < 1000);                         // wait for finger off button
   delay(300);
   lcd.setCursor(0,1); lcdSay(3);                  //"Press Up"  
   do{up_val = analogRead(0);}                     // Keep reading buttons
     while(up_val>1000);                           // until you press one
   do{tempo=analogRead(0); delay(100);} 
     while(tempo < 1000);                          // wait for finger off button
   delay(300);
   lcd.setCursor(0,1);  lcdSay(4);                 //"Press Down"  
   do{down_val = analogRead(0);}  
     while(down_val>1000);   
   do{tempo=analogRead(0); delay(100);} 
     while(tempo < 1000);                          // wait for finger off button  
   delay(300);        
   lcd.setCursor(0,1); lcdSay(5);                  // "Press SELECT" 
   do{select_val = analogRead(0);} 
      while(select_val>1000);   
   do{tempo=analogRead(0); delay(100);} 
      while(tempo < 1000);                         // wait for finger off button
   // --------------------------------------------
   // Save final values to EEPROM  
   high_part= highByte(left_val);   EEPROM.write(550,high_part);
   low_part = lowByte(left_val);    EEPROM.write(551,low_part);
   high_part= highByte(right_val);  EEPROM.write(552,high_part);
   low_part = lowByte(right_val);   EEPROM.write(553,low_part); 
   high_part= highByte(up_val);     EEPROM.write(554,high_part);
   low_part = lowByte(up_val);      EEPROM.write(555,low_part);
   high_part= highByte(down_val);   EEPROM.write(556,high_part);
   low_part = lowByte(down_val);    EEPROM.write(557,low_part);
   high_part= highByte(select_val); EEPROM.write(558,high_part);
   low_part = lowByte(select_val);  EEPROM.write(559,low_part); 
   //Display values on serial monitor
   serialSay(37); serialSay(37); serialSayln(37); //"------------------------------------------------"
   serialSay(6);  Serial.println(left_val);       //" Left ="
   serialSay(7);  Serial.println(right_val);      //" Right = "
   serialSay(8);  Serial.println(up_val);         //" Up = "
   serialSay(9);  Serial.println(down_val);       //" Down =" 
   serialSay(10); Serial.println(select_val);     //" Select ="
   serialSay(37); serialSay(37); serialSayln(37); //"------------------------------------------------"  
   lcd.clear();  
   delay(500);
   lcd.setCursor(0,0); lcdSay(11);                //"Shield buttons"     
   lcd.setCursor(0,1); lcdSay(12);                //"Calibrated"  
   delay(2000);  
}
//--------------------------------------------------------------------------
void fetch_button_settings()
{
  left_val=   256*EEPROM.read(550)+EEPROM.read(551);
  right_val=  256*EEPROM.read(552)+EEPROM.read(553);
  up_val=     256*EEPROM.read(554)+EEPROM.read(555);
  down_val=   256*EEPROM.read(556)+EEPROM.read(557);
  select_val= 256*EEPROM.read(558)+EEPROM.read(559);  
}
//-------------------------------------------------------------------------
int read_button()  
{
   fetch_button_settings();
   int result = -1;
   int analogValue = analogRead(0);   analogValue = analogRead(0); 
   // read the value from the shield's keypad
   if (analogValue < 40)                                                 {result=btnRIGHT;}  // 0      0
   if ( (analogValue>50) && (analogValue < (down_val-50)) )              {result=btnUP;}     // 1     99
   if ( (analogValue>(up_val+50))   && (analogValue < (left_val-50)) )   {result=btnDOWN;}   // 2    256
   if ( (analogValue>(down_val+50)) && (analogValue < (select_val-50)) ) {result=btnLEFT;}   // 3    409
   if ( (analogValue>(left_val+50)) && (analogValue<1000)  )             {result=btnSELECT;} // 4    639
   if (analogValue > 1000)                                               {result = btnNONE;} // 5   1023
//   Serial.print("Value = "); Serial.print(analogValue); Serial.print("     ");
//   Serial.print("Result = ");  Serial.println(result);  
   return result;  }
//-------------------------------------------------------------------------
int read_switch(int addr) 
{               // Reads all 16 pins (port A and B) into a single 16 bits variable
  int ba = 0;
  byte a;
  Wire.beginTransmission(addr);    
  Wire.write(dataportA);
  Wire.requestFrom(addr, 2);  // read the current GPIO output latches
  a  = Wire.read();
  ba = Wire.read();
  error = Wire.endTransmission();    
  if (error == 0)
  {
    lcd.setCursor(15,0);  lcd.print("_");   delay(50); 
    lcd.setCursor(15,0);  lcd.print(" ");    
  }
  ba <<= 8;
  ba |= a;
  return ba;
}
//-------------------------------------------------------------------------
void send_command()
{  
   if(PinNum>7)
   {
     PinNum=PinNum - 8;
   }
   if(PinNum<9)                                 // Ignore any spurious triggers
   {
      serialSay(28); Serial.print(TargetModule-79);     // "Module="
      serialSay(29); Serial.print(PinNum+1);            // "Pin="  
      serialSay(24); Serial.print(mode);                // "Mode "
      serialSay(25); Serial.print(value);               // "value
      serialSay(26); Serial.print(servo_speed);         // "Speed 
      serialSay(27); Serial.println(switch_state);      // "switch
      Wire.beginTransmission(TargetModule);    // transmitting to chosen module
      Wire.write(PinNum);        
      Wire.write(value);  
      Wire.write(servo_speed);   
      Wire.write(switch_state);  
      Wire.endTransmission(); delay(100);
      if(noshow==false)
      {
         lcd.clear();
         lcd.setCursor(0,0); 
         lcdSay(28);lcd.print(TargetModule-79);   // "Module =" 
         lcdSay(29); lcd.print(PinNum+1);         // "Pin ="
         lcd.setCursor(0,1);          
         switch(mode)
         {
            case 0:
               lcdSay(78);                        // "Point"
            break;
            case 1:
               lcdSay(33);                        // "Signal  "
            break;
            case 2:
               lcdSay(79);                        // "Digital"
            break;
         }
         if(mode!=2)
         {
             lcdSay(31); lcd.print(value);        // "val ="      
         }
         if(value>99) 
         {
            lcd.print(" ");
         } 
         else 
         {
           lcdSay(68);
         }  
         if(mode==2)
         { 
            if (switch_state==1)
            {
                lcd.print("High"); 
            }   
            else
            {
               lcd.print("Low");
            }
         }
      else
      {
         if (switch_state==1)
         {
            lcd.print("Hi"); 
         }   
         else
         {
            lcd.print("Lo");
         }           
       }
     }
   }  
   delay(200);  // spaces out mutliple servo rotations
   previousMillis = currentMillis;
}
//-------------------------------------------------------------------------
void start_of_day()
{   
  for(byte servoModuleNum=0;servoModuleNum<15;servoModuleNum++)  // Look for each servo module
  { 
    if (bitRead(servoMap,servoModuleNum)==1)     // If the servo module exists
    {
      byte hilo = servoModuleNum % 2;            // Is it odd/even servo module? 0=even 1=odd  
      byte parent = servoModuleNum /2;
      if (bitRead(switchMap,parent)==1)          // If the servo module has a matching switch module
      { 
         //  serialSay(37); serialSay(37); serialSay(37); serialSayln(37);
         // Serial.print("Servo module "); Serial.print(servoModuleNum+1); Serial.print(" matches switch "); Serial.print(parent+1); 
         byte lookup = switchBaseAddress+parent;  //  Serial.print("  Address "); Serial.print(lookup);
         word switchModuleState = read_switch(lookup); 
              switchModuleState = read_switch(lookup); 
         //  Serial.print("   switchModuleState = "); Serial.println(switchModuleState,BIN);
         TargetModule=servoBaseAddress+servoModuleNum; 
         byte lb = 8 * hilo;
         byte ub = lb + 8 ;   
         //Serial.print("lower = "); Serial.print(lb);Serial.print("    upper = "); Serial.println(ub);
         for(byte pinOut = lb;pinOut < ub; pinOut++)
         {     
         PinNum = pinOut; 
         switch_state = bitRead(switchModuleState,PinNum);  
         if(PinNum>7){PinNum=PinNum-8;} 
         Mem_Address = 32*servoModuleNum+4*PinNum;    //pinOut;    
         // Serial.println("--------------------------------------------------------------");    
         // Serial.print("Mem_Address = "); Serial.println(Mem_Address);
         mode = EEPROM.read(Mem_Address);
         //Serial.print(servoModuleNum+1);Serial.print("/"); Serial.print(PinNum+1); Serial.print(" Switch state = "); Serial.println(switch_state);
           if (switch_state ==1)                              // Decide which rotation value to fetch from EEPROM 
           {
              value = EEPROM.read(Mem_Address+1);              // fetch LEFT rotation value from EEPROM
           }          
           else
           {
              value = EEPROM.read(Mem_Address+2);            // else fetch RIGHT rotation value from EEPROM
           }         
           servo_speed=EEPROM.read(Mem_Address+3);
           noshow=true;   send_command();   noshow=false;     // Send the servo message down the I2C bus 
           previous_check[PinNum]=value;
         }
       } 
     }  
   }   
   Serial.println("Update completed");
}
//--------------------------------------------------------------------------
void centre_servos()
{
   lcd.setCursor(0,0);lcdSay(15);         //"Press LEFT to"
   lcd.setCursor(0,1);lcdSay(16);         //"centre all servos" 
   delay(1000);
   do {button_key = read_button(); button_key = read_button(); }   
       while(button_key == btnNONE);       // Wait for a button press 

       Serial.println(button_key);
   if(button_key==btnLEFT)
    {  
      lcd.setCursor(0,0);lcdSay(17);      //"Up to proceed"
      lcd.setCursor(0,1);lcdSay(18);      //"Dn to Abort " 
      delay(1000);  
      do {button_key = read_button(); button_key = read_button();}   
         while(button_key == btnNONE );   // Wait for a button press
      if (button_key == btnUP)
      {
         for(int x=0; x<512; x=x+4)
         {    // Sets all servo settings to same rotation angle
           EEPROM.write(x,0);            // Sets all mode values to zero
           EEPROM.write(x+1,90);         // Sets all LEFT values to 90 degrees 
           EEPROM.write(x+2,90);         // Sets all RIGHT values to 90 degrees 
           EEPROM.write(x+3,65);         // Speed 1 to 125 in 32 steps (increments of 4)
         }      
         EEPROM.write(641,90);         // Informs that program has already been run
         start_of_day();
         lcd.setCursor(0,0);lcdSay(13);// "All Servos are "
         lcd.setCursor(0,1);lcdSay(14);// "now centred"
         delay(4000);
      }
    }   
    else 
    {
       lcd.clear();
       lcd.setCursor(0,0);lcdSay(19);    //"Reset Aborted"
       delay(2000); 
    }
}
//------------------------------------------------------------------------- 
void show_pin_settings()
{
  serialSayln(70);                                                        // "Your system settings are:"
  serialSay(37); serialSay(37); serialSay(37); serialSayln(37);
  for(byte servoModuleNum=0;servoModuleNum<15;servoModuleNum++)
  { 
    if (bitRead(servoMap,servoModuleNum)==1)                                 
    {
      serialSay(35);  Serial.println(servoModuleNum+1);                    //  "Module "
      for(byte printOut = 0;printOut < 32; printOut=printOut+4)
      { 
        serialSay(68); 
        Serial.print(" Pin "); Serial.print(printOut/4+1); serialSay(68);  //  "Pin "
        int offset = (servoModuleNum*32)+ printOut ;   //Serial.println(offset);
        byte info = EEPROM.read(offset);   
        if (info==0)  {serialSay(32);}                       // " Point   "   
        if (info==1)  {serialSay(33);}                       // "Signal "
        if (info==2)  {serialSay(34);}                       // " IO "
        byte LEFTval  = EEPROM.read(offset+1);
        byte RIGHTval = EEPROM.read(offset+2);
        if (info==0)                  // Point
        {
           serialSay(71);             // Left:
           if(LEFTval<100)
           {
              Serial.print(" ");
           }
           Serial.print(LEFTval); 
           serialSay(68);             //  "  "
           serialSay(72);             // Right:
           if(RIGHTval<100)
           {
              Serial.print(" ");
           }           
           Serial.print(RIGHTval); 
           serialSay(68);              //  "  "    
           serialSay(26);              // Speed
           Serial.println(EEPROM.read(offset+3));     
        }
        if (info==1)                   //  Signal
        {
           serialSay(73);              //  Up:
           if(LEFTval<100)
           {
              Serial.print(" ");
           }          
           Serial.print(LEFTval); 
           serialSay(68);              //  "  "  
           serialSay(74);              //  Down:
           serialSay(68);              //  "  "
           Serial.print(RIGHTval); 
           serialSayln(68);            //  "  "           
        }
        if (info==2)
        {
            Serial.println();
        }
        if((printOut/4)==7)
        {
           serialSay(37);serialSay(37);serialSay(37); serialSayln(37);
        }
      }   
    }   
  }
}

//------------------------------------------------------------------------- 
/*  void Display_Key_Values()
{  
  fetch_button_settings();
  Serial.println();
  serialSay(38); Serial.println(right_val);    //"Right button value"
  serialSay(39); Serial.println(up_val);       //"Up button value"
  serialSay(40); Serial.println(down_val);     //"Dn button value"
  serialSay(41); Serial.println(left_val);     //"Left Button value"
  serialSay(42); Serial.println(select_val); //"Select Button value"
}
*/

//----------------------------------------------------------------------------------
void SignalUp(byte up, byte down, byte range)
{
  lcd.clear();
  lcdSay(75);   // "Signal Up"
  lcd.setCursor(0,1);
  lcdSay(28);
  lcd.print(TargetModule-79);  
  lcdSay(29); 
  if(PinNum>7)
    { PinNum=PinNum - 8; }
  lcd.print(PinNum+1); 
  servo_speed=40;
  value = down+(range/2);    
  TX_only();
  delay(100);
  value = down+(range/2)-(range/5);
  TX_only();
  delay(100);
  value = up; 
  TX_only();
}
//-------------------------------------------------------------------------
void TX_only()
{
    Wire.beginTransmission(TargetModule);    // transmitting to chosen module
    Wire.write(PinNum);        
    Wire.write(value);  
    Wire.write(servo_speed);   
    Wire.write(switch_state);  
    Wire.endTransmission(); delay(100); 
}
//-------------------------------------------------------------------------
void SignalDown(int up, int down, int range)
{
  lcd.clear();
  lcdSay(76);   // "Signal Down"
  lcd.setCursor(0,1);
  lcdSay(28);
  lcd.print(TargetModule-79);  
  lcdSay(29); 
  if(PinNum>7)
    { PinNum=PinNum - 8; }
  lcd.print(PinNum+1);           
  servo_speed=0;
  value = down;         // fully down
  TX_only();
  value=down+range/3;   // 1st bounce up
  TX_only();
  value = down;         // fully down again
  TX_only();
  value=down+range/4;   // 2nd bounce up
  TX_only();
  value = down;         // fully down again
  TX_only();
}
//-------------------------------------------------------------------------
void display_and_save()
{  
   //  Display chosen settings - press RIGHT button to proceed
   lcd.clear();
   lcd.setCursor(0,0);  lcd.print("Module ");lcd.print(ModuleNum+1); lcd.print("/");  lcd.print(PinNum+1); 
   switch (mode){
     case 0: {lcd.print(" Sp:");  lcd.print(servo_speed); 
              lcd.setCursor(0,1); lcd.print("L:");  lcd.print(LEFT);   
              lcd.print(" R:");   lcd.print(RIGHT); lcdSay(69); break;}          //" R=save"
     case 1: {lcd.print(" Sp:");  lcd.print(servo_speed); 
              lcd.setCursor(0,1); lcd.print("Up:"); lcd.print(LEFT);   
              lcd.print(" Down:");lcd.print(RIGHT); lcdSay(69); break;}          //" R=save"
     case 2: {lcd.print(" set"); 
              lcd.setCursor(0,1); lcd.print("digital ");  lcdSay(69); break;}    //" R=save"
   }
   do {button_key = read_button(); delay(100);}   
   while(button_key != btnRIGHT);           // Wait for user to read screen then press RIGHT button
   Mem_Address = 32*(ModuleNum)+4*(PinNum); 
   EEPROM.write(Mem_Address,mode);          // Save the values to EEPROM
   EEPROM.write(Mem_Address+1,LEFT);
   EEPROM.write(Mem_Address+2,RIGHT);
   EEPROM.write(Mem_Address+3,servo_speed);
   lcd.clear();                             // Give feedback to user that the changes have been saved
   for(int x=0; x <16; x++) 
   {
      lcd.setCursor(x,0); lcd.print("-");  delay(70);
   }
   lcd.setCursor(0,1); lcdSay(56);      // "Settings Saved"
   switch_state = bitRead(new_input,PinNum); // fetch status of pin
   if (switch_state == 1)
    {value=LEFT;}
   else
     {value=RIGHT;}
   send_command();
   Serial.print("TargetModule = "); Serial.print(TargetModule); 
   Serial.print("  Pin= "); Serial.print(PinNum+1);  
   Serial.print("  Value= "); Serial.print(value); 
   Serial.print("  Speed= "); Serial.print(servo_speed);
   Serial.print("  Switch state = "); Serial.print(switch_state);
   TX_only();
   logo();             
}
//-------------------------------------------------------------------------
void get_speed()
{  
   lcd.setCursor(0,0); lcdSay(49);   //" Speed   +-"
   lcd.setCursor(0,1); lcdSay(50);   //"<Test  Adopt >"
   servo_speed = EEPROM.read(Mem_Address+3);         // Fetch existing speed value
   do 
   {
      button_key = read_button();    
      if(button_key == btnUP && servo_speed < 122) {servo_speed=servo_speed+4;}
      if(button_key == btnDOWN && servo_speed > 4) {servo_speed=servo_speed-4;}  
      lcd.setCursor(7,0);    lcd.print(servo_speed); lcd.print(" ");
      if (button_key == btnLEFT)
      {                    // Chosen to test servo speed
         do 
         {
             button_key = read_button();
         }  
         while(button_key == btnLEFT);                  // Wait for finger off button
                   Serial.print("LEFT = "); Serial.print(LEFT); Serial.print("   RIGHT  = "); Serial.println(RIGHT);
         TargetModule = servoBaseAddress+ModuleNum;     // Calculate which module to send the command to
         //         serialSay(51); Serial.println(TargetModule);   // "Sending to"
         Wire.beginTransmission(TargetModule);          // transmiting to chosen module 
         Wire.write(PinNum);    Wire.write(LEFT);   Wire.write(servo_speed);     Wire.write(switch_state); 
         Wire.endTransmission();
         int distance = abs(LEFT-RIGHT);            Serial.print("Speed = ");Serial.println(servo_speed);      Serial.print(" Distance =  "); Serial.println(distance);
         int holdit = ((distance/servo_speed)*400)+ 2000; Serial.print(" Holdit   =  "); Serial.println(holdit);
         delay(holdit);
         Wire.beginTransmission(TargetModule);          // transmiting to chosen module 
         Wire.write(PinNum);     Wire.write(RIGHT);  Wire.write(servo_speed);  Wire.write(switch_state);   
         Wire.endTransmission();
      }
      delay(500);
   }   
   while(button_key != btnRIGHT);
   do {button_key = read_button();} 
      while(button_key == btnRIGHT);                    // Wait for finger off button
}
//-------------------------------------------------------------------------
void get_mode()
{
   lcd.setCursor(0,0); lcdSay(57);//   "L=Point R=Signal"
   lcd.setCursor(0,1); lcdSay(58);//   "Up=I/O Down=Exit"
   do 
   {
      button_key = read_button();
   }
   while(button_key != btnLEFT && button_key != btnRIGHT && button_key != btnUP && button_key != btnDOWN );
   if (button_key == btnLEFT)  {mode = 0;}   // Normal point
   if (button_key == btnRIGHT) {mode = 1;}   // Signal bounce
   if (button_key == btnUP)    {mode = 2;}   // I/O
   if (button_key == btnDOWN)  {mode =99;}   
}
//-------------------------------------------------------------------------
void get_pin()
{
  //  Get the Pin number
  //  ---------------------              
   lcd.setCursor(0,0); lcdSay(52);                    //"Pin number"
   lcd.setCursor(15,0);    lcd.print(UserPinNum); 
   do 
   {
      button_key = read_button();  
      if (button_key == btnUP   && UserPinNum<8) 
         {UserPinNum++;}   
      if (button_key == btnDOWN && UserPinNum >1) 
         {UserPinNum--;}   
      lcd.setCursor(15,0);   lcd.print(UserPinNum); lcd.print(" ");
      delay(300); 
   } 
   while(button_key != btnSELECT);   
   PinNum=UserPinNum-1;
   do {button_key = read_button();}   
      while(button_key == btnSELECT);            // Wait for finger off the button 
}
//-------------------------------------------------------------------------
void get_module()
{
  //  Get the module number
  //  ---------------------
   UserModuleNum=1;
   lcd.clear();
   lcd.setCursor(0,0); lcdSay(47);                         // "Module# +-"
   lcd.setCursor(0,1); lcdSay(48);                         // "Select when done"
   lcd.setCursor(14,0);    lcd.print(UserModuleNum);
   do 
   {
      button_key = read_button();                        //  Keep going round the loop
      if (button_key == btnUP && UserModuleNum <16)  
         {UserModuleNum++;}                               //  Increase the module number
      if (button_key == btnDOWN && UserModuleNum >1) 
         {UserModuleNum--;}                              //  Decrease the module number
      lcd.setCursor(14,0); 
      lcd.print(UserModuleNum); lcd.print(" ");           //  Display the module number currently selected
      delay(300);
   }
   while(button_key != btnSELECT);                         //  Until the SELECT button is pressed
   do {button_key = read_button();} 
      while(button_key == btnSELECT);                       // Wait for finger off the button 
   ModuleNum = UserModuleNum-1;
  }
//-------------------------------------------------------------------------
void get_LEFT_setting()
{   
   LEFT = 90;                                    // Move servo to mid-position
   noshow=1; send_command(); noshow=0;         
   lcd.clear();
   delay(500);
   lcd.setCursor(0,0);     
   if(mode==0) {lcdSay(43);}                     // "Left Value"
   if(mode==1) {lcdSay(44);}                     // "Up Value"
   lcd.setCursor(0,1); lcdSay(45);               // "+- then select"
   do 
   {                                             
      button_key = read_button();  delay(200);   // Get the new LEFT value
      if((button_key == btnUP) && (LEFT < 180))  // Keep LEFT within range of 2 to 180
      {
         LEFT++; 
      }    
      if(button_key == btnDOWN && LEFT >1)   
      {
         LEFT--; 
      }   
      lcd.setCursor(11,0);   lcd.print(LEFT); lcdSay(68);    // "  "      
      value=LEFT;  noshow=1; send_command(); noshow=0;       // Move servo to new position 
   }
   while(button_key != btnSELECT);                           // Stop when user presses SELECT
   do 
     {
        button_key = read_button(); delay(100);
     }   
   while(button_key == btnSELECT);                           // Wait for finger off the SELECT button 
   delay(200);
}
//-----------------------------------------------------------------------------
 void get_RIGHT_setting()
 {
   RIGHT = 90;                                  // Move servo to mid-position
   noshow=1; send_command(); noshow=0;     
   lcd.clear();
   lcd.setCursor(0,0);  
   if(mode==0) {lcdSay(46);}                   //"Right value" 
   if(mode==1) {lcdSay(77);}                   //"Down value"
   lcd.setCursor(0,1); lcdSay(45);             // "+ - then Select"
   do 
   {                                           // Get the new RIGHT value  
      button_key = read_button();              // Keep RIGHT within range
      if(button_key == btnUP && ((mode== 0 && RIGHT<180) || (mode==1 && RIGHT<LEFT))) 
      {
         RIGHT++; 
      }  
      if(button_key == btnDOWN && RIGHT>1) 
      {
         RIGHT--; 
      }  
      delay(200);
      lcd.setCursor(12,0);  lcd.print(RIGHT); lcdSay(68);    //"  "
      value=RIGHT; noshow=1; send_command(); noshow=0;       // Move servo to new position 
     //     delay(100);
   }
   while(button_key != btnSELECT);                           // Stop when user presses SELECT
   do 
   {
       button_key = read_button();delay(100);
   }   
   while(button_key == btnSELECT);                           // Wait for finger off the SELECT button 
}
//-------------------------------------------------------------------------
void pin_configuration()
{
  lcd.setCursor(0,0);lcdSay(59);                  // "Configure a pin "
  lcd.setCursor(0,1);lcdSay(60);                  // "L=Proceed R=Exit"
  do {button_key = read_button(); delay(100);}
     while(button_key != btnLEFT && button_key != btnRIGHT);
  if (button_key == btnLEFT)
  {
     get_module();  
     get_pin();     
     get_mode();        
     Mem_Address = 24*ModuleNum+4*PinNum;  // serialSay(61); // Serial.println(Mem_Address);
     if (mode==2)                          // Digital I/O
     {
        servo_speed=0;                     // Speed irrelevent             
        display_and_save();
     }             
     else                                  // Normal servo or signal bounce
     {  
        TargetModule = servoBaseAddress+ModuleNum;  // Calculate which module to send the command to    
        get_LEFT_setting();                         //  Get the two endpoint settings
        get_RIGHT_setting();       
        if((LEFT<90 && RIGHT<90)||(LEFT>90 && RIGHT>90))
        {
           lcd.clear();
           lcd.setCursor(0,0);      
           lcdSay(62);    //  "Values must be  "
           lcd.setCursor(0,1);      
           lcdSay(63);    //  "centred round 90"
        }
        else
        {
           get_speed();       
           display_and_save(); 
        } 
     }     
  }     
  else                              // Didn't press LEFT to proceed, so aborting void
  {
    logo();
  }
}
//-------------------------------------------------------------------------
void logo()
{   
   lcd.clear(); 
   lcd.setCursor(0,0);lcdSay(64);  // "     EzyBus"
   lcd.setCursor(0,1);lcdSay(65);  // "      v1.0"
}
//-------------------------------------------------------------------------
void displayMaps()
{
/*   Serial.print("Switches = ");
   Serial.println(switchMap,BIN);
   Serial.print("Servos   = ");
   Serial.println(servoMap,BIN);
   Serial.println("------------------------");*/
   lcd.clear();
   for (byte x=0; x<8; x++)
   {
      lcd.setCursor(x,0); 
      if (bitRead(switchMap,x)==0)
         {lcd.print(".");}
      else
         {lcd.print(x+1);}
   }  
   for (byte x=0; x<16; x++)    
   {
      lcd.setCursor(x,1); 
      if (bitRead(servoMap,x)==0)
         {lcd.print(".");}
      else
         {lcd.print(x+1,HEX);}     
   }  
   do
   {
      tempo = analogRead(0); delay(100);  // Keep reading buttons
   }     
   while(tempo>1000 );                    // until you press one
}
//-------------------------------------------------------------------------
void mapHardware()
{ Serial.print("Starting mapping");
  for(int i =0; i<8; i++) // map Switch Units
  {
    Serial.println(i);
     byte switchAddress = switchBaseAddress + i;
     Wire.beginTransmission(switchAddress);
     error = Wire.endTransmission();   
     if (error == 0)
     {  
         bitSet(switchMap,i);  
     }  
  }
  for(int i = 0; i<16 ; i++) // map Servo boards
  {
     byte servoAddress = servoBaseAddress + i;
     Wire.beginTransmission(servoAddress);
     error = Wire.endTransmission();
     if (error == 0)
     {  
        bitSet(servoMap,i);  
     }  
  }
  Serial.println("Hardware mapping completed");
}
//-------------------------------------------------------------------------
void setup(){
   delay(1000);            
   Serial.begin(115200);              // Allow writing to monitor window 
   lcd.begin(16, 2);                  // Start the LCD library   
   lcd.clear();                       // Clear the screen
   lcd.setCursor(0,0);lcdSay(20);     // "Updating for" 
   lcd.setCursor(0,1);lcdSay(21);     // "Start of day";
   delay(5000);
   Wire.begin(MasterAddress);         // Activate I2C network    
   mapHardware();
   configure_switches(); 
   pinMode(calibrate_pin, INPUT);  
   if (digitalRead(calibrate_pin) == LOW)
   {
      calibrate();
   }  
   else 
   {   
      fetch_button_settings();
   }   
   if(EEPROM.read(641)!=90)              // Checks to see if program has been run before
   {         
      centre_servos();                   // If not, all values are preset
   }  
   start_of_day();    
   Serial.println("Setup completed");            
   logo();   
}
//-------------------------------------------------------------------------
int isPowerOfTwo(unsigned n)
{  return n && (! (n & (n-1)) ); }
//-------------------------------------------------------------------------
// Returns position of the only set bit in 'n'
int findPosition(unsigned n)
{   if (!isPowerOfTwo(n))
        return -1;
    unsigned i = 1, pos = 1;
    // Iterate through bits of n till we find a set bit
    // i&n will be non-zero only when 'i' and 'n' have a set bit
    // at same position
    while (!(i & n))
       {// Unset current bit and set the next bit in 'i'
        i = i << 1;
        // increment position
        ++pos;
        }
    return pos;
}
//-------------------------------------------------------------------------
void scan_switches()
{ 
  for(int switchNum=0; switchNum<8; switchNum++)           // Read each MCP23017 in turn, from module 0 to module 7
  {
    if (bitRead(switchMap,switchNum)==1)                   // If the switch module exists                     
    {
      new_input = read_switch(32+switchNum);               // Fetch value of port 
      delay(100);                                          // recovery time
      if (new_input != previous_check[switchNum])          // If there is a change in input
      {
         diff = (previous_check[switchNum] ^ new_input);   // Calculate the difference from last time
         PinNum=(findPosition(diff)-1);                    // Convert to a pin number
         TargetModule = servoBaseAddress+2*switchNum;      // Calculate which module to send the command to
         if (PinNum>7)
         {
             TargetModule++;                               //  if pin > 8 then adjust module number
             //  PinNum=PinNum-8;                             
         }                 
         //     delay(200);      // For debounce
         Mem_Address = 64*switchNum+4*PinNum;             
         Serial.print("Fetching from Mem address : "); Serial.println(Mem_Address);
         mode = EEPROM.read(Mem_Address);                  // Fetch the data from EEPROM
         switch_state = bitRead(new_input,PinNum);  
         byte lvalue = EEPROM.read(Mem_Address+1);         // fetch LEFT rotation value from EEPROM
         byte rvalue = EEPROM.read(Mem_Address+2);         // else fetch RIGHT rotation value from EEPROM 
         if (switch_state==1)                              // Decide which rotation value to fetch from EEPROM 
         {
            value = lvalue;
         }  
         else
         {
            value=rvalue;
         }        
         if(mode==2)                                      //  Digital mode
         {                                                
            value=90;                                     //  Ensures any connected servo stays in mid position
         }  
         if(mode==1)
         {
            byte diff=abs(lvalue-rvalue);
            if (switch_state==1)
            {
                SignalUp(lvalue,rvalue,diff);
            }
            else
            {
                SignalDown(lvalue,rvalue,diff);
            }  
         }   
         servo_speed=EEPROM.read(Mem_Address+3);
         previous_check[switchNum]=new_input;
         noshow=false;
         if(mode!=1)                              // Only move servo if not in signal mode 
         {                                     
            send_command();                       // Send the servo message down the I2C bus
         }
      }
   } 
}
}
//-------------------------------------------------------------------------
void loop()
{  
  currentMillis = millis();
  int menu_key = read_button();                   // Read the shield's buttons
  if(menu_key == btnRIGHT)
  {
     show_pin_settings();                         // Option 1 - Display all pin settings on computer screen
  }  
  if(menu_key == btnDOWN)
  {
    centre_servos();                              // Option 2 - Centre all connected servos and their settings
  }     
  if(menu_key == btnUP)    
  {
     mapHardware(); displayMaps();                // Option 3 - Display which modules are on the bus 
     do 
     {
        button_key = read_button();
     }
     while(button_key == btnNONE);
  }    
  if (menu_key == btnSELECT)                      // Option 4 - Configure an output on a servo module
  {
     pin_configuration();
  }   
  scan_switches();                                // Option 5 - Just run the main program
  if ((currentMillis - previousMillis) >= 5000)   // Allow 5 secs for user to read command before returning to logo
  {
     logo();  previousMillis = currentMillis;
  }
}
