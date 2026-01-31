/** OutputModule.
 *  @file
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
 *  Libraries used:
 *
 *  Name              | Purpose
 *  ----------------- | -------
 *  EEPROM            | Reading and writing to EEPROM memory.
 *  Wire              | To handle i2c communications.
 *
 *
 *  Pin usage:
 *
 *  D0      Serial Rx.      Could be jumper J2.
 *  D1      Serial Tx.      Could be jumper J1.
 *  D2      Unused.
 *  D3      Unused.
 *  D4      Alternate jumper J1.
 *  D5      Alternate jumper J2.
 *  D6      Alternate jumper J3.
 *  D7      Alternate jumper J4.
 *  D8      Unused.
 *  D9      Unused.
 *  D10     Unused.
 *  D11     Remote IR.
 *  D12     Unused.
 *  D13     Unused.         Also flash firmware version.
 *
 *  A0      Unused.
 *  A1      Unused.
 *  A2      Unused.
 *  A3      Unused.
 *  A4      I2C SDA.
 *  A5      I2C SCL.
 *  A6      Jumper J4
 *  A7      Jumper J3
 *
 */


#define SB_INPUT_MODULE true       // The is an input module.


#include "Config.h"                 // Common classes.
#include "Messages.h"
#include "Persisted.h"
#include "I2cComms.h"
#include "SystemMgr.h"

#include "Remote.h"


// Ticking
unsigned long  now       = 0;   // To keep the current time (since boot).

#define COMMAND_BUFFER_LEN   8                  // Serial command buffer length
char    commandBuffer[COMMAND_BUFFER_LEN + 1];  // Buffer to read characters with null terminator on the end.
uint8_t commandLen = 0;                         // Length of command.


// I2C request command parameters
volatile uint8_t requestCommand = COMMS_CMD_NONE;
volatile uint8_t requestOption  = 0;
volatile uint8_t requestNode    = 0;


/** Setup the Arduino.
 */
void setup()
{
    Serial.begin(SERIAL_SPEED);     // Serial IO.

    systemMgr.init();               // Initialise SystemMgr.
    
    // Load SystemData from EEPROM and check it's valid.
    if (!systemMgr.loadSystemData())
    {
        firstRun();
    }
    else
    {
        // Recover state from EEPROM.
    }

    // Start I2C communications.
    i2cComms.setId(systemMgr.getModuleId(true));
    i2cComms.onReceive(processReceipt);
    i2cComms.onRequest(processRequest);

    // Flash out version number on the built-in LED,
    systemMgr.flashVersion();

    // Check if version update required.
    if (systemMgr.isUpdateRequired())
    {
        if (isDebug(DEBUG_NONE))
        {
            Serial.print(PGMT(M_UPDATE));
            Serial.print(CHAR_SPACE);
            Serial.print(systemMgr.getVersion(), HEX);
            Serial.print(CHAR_TILDE);
            Serial.print(VERSION, HEX);
            Serial.println();
        }

        // Do the update here.
        systemMgr.update();
    }

    remote.init();
}


/** Initialise data when first run.
 */
void firstRun()
{
    systemMgr.saveSystemData();
}


/** Report unrecognised command.
 */
void unrecognisedCommand(PGM_P aMessage, uint8_t aCommand, uint8_t aOption)
{
    if  (isDebug(DEBUG_ERRORS))
    {
        Serial.print(PGMT(aMessage));
        Serial.print(PGMT(M_DEBUG_COMMAND));
        Serial.print(aCommand, HEX);
        Serial.print(PGMT(M_DEBUG_OPTION));
        Serial.print(aOption);
        Serial.println();
    }
}


/** Data received.
 *  Process the command.
 */
void processReceipt(int aLen)
{
    if (aLen > 0)
    {
        // Read the command byte.
        uint8_t command = i2cComms.readByte();

        if (isDebug(DEBUG_BRIEF))
        {
            Serial.println();
            Serial.print(PGMT(M_DEBUG_RECEIPT));
            Serial.print(PGMT(M_DEBUG_COMMAND));
            Serial.print(command, HEX);
            Serial.print(PGMT(M_DEBUG_LEN));
            Serial.print(aLen, HEX);
            Serial.println();
        }

        switch (command)
        {
            default:               unrecognisedCommand(M_DEBUG_RECEIPT, command, 0);
                                   break;
        }
    }
    else
    {
        // Null receipt - Just the master seeing if we exist.
        if (isDebug(DEBUG_BRIEF))
        {
            Serial.println();
            Serial.print(PGMT(M_DEBUG_RECEIPT));
            Serial.print(CHAR_SPACE);
            Serial.print(PGMT(M_DEBUG_LEN));
            Serial.print(aLen, HEX);
            Serial.println();
        }
    }

    // Consume unexpected data.
    if (i2cComms.available())
    {
        if (isDebug(DEBUG_ERRORS))
        {
            Serial.println();
            Serial.print(PGMT(M_DEBUG_UNEXPECTED));
            Serial.print(PGMT(M_DEBUG_LEN));
            Serial.print(i2cComms.available(), HEX);
            Serial.print(CHAR_COLON);
        }

        while (i2cComms.available())
        {
            uint8_t ch = i2cComms.readByte();
            if (isDebug(DEBUG_ERRORS))
            {
                Serial.print(CHAR_SPACE);
                Serial.print(ch, HEX);
            }
        }
        if (isDebug(DEBUG_ERRORS))
        {
            Serial.println();
        }
    }
}


/** Process a Request (for data).
 *  Send data to master.
 */
void processRequest()
{
    if (isDebug(DEBUG_BRIEF))
    {
        Serial.println();
        Serial.print(PGMT(M_DEBUG_REQUEST));
        Serial.print(PGMT(M_DEBUG_COMMAND));
        Serial.print(PGMT(M_DEBUG_COMMANDS[requestCommand >> COMMS_COMMAND_SHIFT]));
        Serial.print(PGMT(M_DEBUG_OPTION));
        Serial.print(requestOption, HEX);
        Serial.println();
    }

    switch (requestCommand)
    {
        default:               unrecognisedCommand(M_DEBUG_REQUEST, requestCommand, 0);
                               break;
    }

    // Clear pending command.
    requestCommand = COMMS_CMD_NONE;
}


/** Process a received command.
 *  Using the contents of the commandBuffer:
 */
void processCommand()
{
}


//// Metrics.
//long start = 0;
//long count = 0;

//// DEBUG test marker
//int testRun = 0;

/** Main loop.
 */
void loop()
{
    // Look for command characters
    while (Serial.available() > 0)
    {
        char ch = Serial.read();
        if (ch == CHAR_RETURN)
        {
            // Ignore carriage-return
        }
        else if (ch == CHAR_NEWLINE)
        {
            // Process the received command
            if (commandLen > 0)
            {
                commandBuffer[commandLen] = CHAR_NULL;
                processCommand();
                commandLen = 0;
            }
        }
        else if (commandLen <= COMMAND_BUFFER_LEN)
        {
            commandBuffer[commandLen++] = ch;
        }
    }

    // Record the time now
    now = millis();

    // Trigger an update of the Remote.
    remote.update();


//    // Metrics
//    count += 1;
//    if ((now - start) > 1000L)
//    {
//        Serial.println();
//        Serial.print(now);
//        Serial.print(": ");
//        Serial.print(count);
//        Serial.println();
//        
//        count = 0;
//        start = now;
//    }

}
