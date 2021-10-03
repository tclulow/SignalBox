/** SignalBox.
 *
 *
 *  (c)Copyright Tony Clulow  2021	tony.clulow@pentadtech.com
 *
 *  This work is licensed under the:
 *      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 *      http://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 *  For commercial use, please contact the original copyright holder(s) to agree licensing terms.
 *
 *
 *  Main SignalBox module.
 *  
 *  Libraries used:
 *
 *  Name              | Purpose
 *  ----------------- | -------
 *  EEPROM            | Reading and writing to EEPROM memory.
 *  Wire              | To handle i2c communications.
 *  LiquidCrystal     | For driving an LCD shield attached to the Uno.
 *  LiquidCrystal_I2C | For driving an LCD attached by i2c. Set LCD_I2C false to disable this functionality.
 *
 *
 *  Pin usage:
 *
 *  D0      Serial Rx.
 *  D1      Serial Tx.
 *  D2      Alternate up button.
 *  D3      Alternate right button.
 *  D4      LCD shield data D4.
 *  D5      LCD shield data D5.
 *  D6      LCD shield data D6.
 *  D7      LCD shield data D7.
 *  D8      LCD shield rs.
 *  D9      LCD shield enable.
 *  D10     LCD shield backlight.
 *  D11     LCD shield detect.
 *  D12     Interlock warning.
 *  D13     Flash firmare version.
 *
 *  A0      LCD shield buttons.
 *  A1      Alternate select button.
 *  A2      Alternate left button.
 *  A3      Alternate down button.
 *  A4      I2C SDA.
 *  A5      I2C SCL.
 *  A6      Not available.
 *  A7      Not available.
 */

#define SB_CONTROLLER true          // The controller (Uno) device.


#include "All.h"


#define COMMAND_BUFFER_LEN   8                  // Serial command buffer length
char    commandBuffer[COMMAND_BUFFER_LEN + 1];  // Buffer to read characters with null terminator on the end.
uint8_t commandLen = 0;                         // Length of command.


// Ticking
long    now              = 0L;      // Current time in millisecs.
long    tickHardwareScan = 0L;      // Time for next scan for hardware.
long    tickInputScan    = 0L;      // Time for next scan of input switches.
long    tickGateway      = 0L;      // Time for next gateway request.
long    tickHeartBeat    = 0L;      // Time of last heartbeat.


/** Software hasn't been run before.
 */
void firstRun()
{
    // Initialise SystemData.
    systemData.magic = MAGIC_NUMBER;

    // Calibrate the LCD buttons.
    if (hasLcdShield)
    {
        buttons.calibrateButtons();
    }

    // Decide if EzyBus conversion required.
    if (ezyBusDetected())
    {
        disp.clear();
        disp.printProgStrAt(LCD_COL_START, LCD_ROW_TOP, M_EZY_FOUND);
        disp.printProgStrAt(LCD_COL_START, LCD_ROW_DET, M_EZY_UPDATE);

        ezyBusClear();
        if (buttons.waitForButtonPress() == BUTTON_SELECT)
        {
            ezyBusConvert();
            buttons.waitForButtonClick();
            defaultInputs(INPUT_TYPE_TOGGLE);
        }
        else
        {
            defaultInputs(INPUT_TYPE_ON_OFF);
        }
    }
    else
    {
        defaultInputs(INPUT_TYPE_ON_OFF);
    }

    // Save all data to EEPROM.
    saveSystemData();

    buttons.waitForButtonClick();
}


/** Set the default initial setup
 *  1-1 mapping, inputs to outputs.
 */
void defaultInputs(uint8_t aInputType)
{
    disp.clear();
    disp.printProgStrAt(LCD_COL_START, LCD_ROW_TOP, M_INITIALISING);
    disp.setCursor(LCD_COL_START, LCD_ROW_DET);

    inputNumber = 0;
    inputType   = aInputType;

    for (uint8_t node = 0; node < INPUT_NODE_MAX; node++)
    {
        disp.printHexCh(node);

        for (uint8_t pin = 0; pin < INPUT_PIN_MAX; pin++)
        {
            // Create an input.
            inputDef.setOutput(0, inputNumber);     // Map 1-1 inputs to outputs.
            inputDef.setDelay(0, false);
            for (uint8_t index = 1; index < INPUT_OUTPUT_MAX; index++)
            {
                inputDef.setOutput(index, 0);       // Zero-length delay.
                inputDef.setDelay(index, true);
            }

            saveInput();
            inputNumber += 1;       // Input numbers map nicely to OutputNumbers.
        }
    }
}


/** Convert EzyBus configuration.
 *  One-one mapping with EzyBus modules, and their inputs.
 */
void ezyBusConvert()
{
    int     ezyBus = 0;
    uint8_t value  = 0;

    disp.clearRow(LCD_COL_START, LCD_ROW_DET);
    disp.clearBottomRows();
    disp.printProgStrAt(LCD_COL_START, LCD_ROW_EDT, M_EZY_UPDATING, LCD_COLS);
    disp.setCursor(LCD_COL_START, LCD_ROW_BOT);

    for (outputNode = 0; outputNode < EZY_NODE_MAX; outputNode++)
    {
        disp.printHexCh(outputNode);

        for (outputPin = 0; outputPin < OUTPUT_PIN_MAX; outputPin++)
        {
            // Create an Output that reflects the Ezybus one.
            outputDef.setState(false);
            outputDef.setType((EEPROM.read(ezyBus++) + 1) & OUTPUT_TYPE_MASK);                  // Output types are 1 greater then those of Ezybus.
            outputDef.setLo(EEPROM.read(ezyBus++));
            outputDef.setHi(EEPROM.read(ezyBus++));
            outputDef.setPace((EEPROM.read(ezyBus++) >> EZY_SPEED_SHIFT) & OUTPUT_PACE_MASK);   // Convert Ezybus pace.
            outputDef.setReset(OUTPUT_DEFAULT_RESET);

            writeOutput();
            writeSaveOutput();
        }
    }
}


/** Process a received command.
 *  Using the contents of the commandBuffer:
 *      iNP - Action input for node N, pin P.
 *      lNP - Action output Lo for node N, pin P.
 *      hNP - Action output Hi for node N, pin P.
 *      oNP - Action output Hi/Lo (based on current state) for node N, pin P.
 */
void processCommand()
{
    boolean executed = false;
    uint8_t node     = 0;
    uint8_t pin      = 0;
    boolean state    = true;

    if (isDebug(DEBUG_BRIEF))
    {
      Serial.print(millis());
      Serial.print(CHAR_TAB);
      Serial.print(PGMT(M_INPUT));
      Serial.print(PGMT(M_DEBUG_COMMAND));
      Serial.println(commandBuffer);
    }

    // Expect three characters, command, nodeId, pinId
    if (strlen(commandBuffer) == 3)
    {
        node = charToHex(commandBuffer[1]);
        pin  = charToHex(commandBuffer[2]);

        switch (commandBuffer[0] | 0x20)            // Command character converted to lower-case.
        {
            case 'i': if (   (node < INPUT_NODE_MAX)
                          && (pin  < INPUT_PIN_MAX))
                      {
                          loadInput(node, pin);
                          controller.processInput(false);
                          executed = true;
                      }
                      break;

            case 'o': state = getOutputState(node, pin);
            case 'l': state = !state;
            case 'h': if (   (node < OUTPUT_NODE_MAX)
                          && (pin  < OUTPUT_PIN_MAX))
                      {
                          writeOutputState(node, pin, state, 0);
                          readOutputStates(node);                   // Recover states in case LED_4 has moved one.
                          executed = true;
                      }

            default:  break;
        }
    }

    // Report error if not executed.
    if (!executed)
    {
        if (isDebug(DEBUG_ERRORS))
        {
          Serial.print(millis());
          Serial.print(CHAR_TAB);
          Serial.print(PGMT(M_UNKNOWN));
          Serial.print(PGMT(M_DEBUG_COMMAND));
          Serial.println(commandBuffer);
        }

        if (isReportEnabled(REPORT_SHORT))
        {
            disp.clearRow(LCD_COL_START, LCD_ROW_BOT);
            disp.setCursor(LCD_COL_START, LCD_ROW_BOT);
            disp.printProgStr(M_UNKNOWN);
            disp.printCh(CHAR_SPACE);
            disp.printStr(commandBuffer);
            controller.setDisplayTimeout(getReportDelay());
        }
    }
}


/** Handle input on the Serial interface,
 */
void handleSerialInput()
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
}


/** See if there's a Gateway request.
 *  Process the request.
 *  Return true if work done.
 */
boolean gatewayRequest()
{
    boolean workDone = false;

    i2cComms.sendGateway(COMMS_CMD_SYSTEM | COMMS_SYS_GATEWAY, -1, -1);
    if (i2cComms.requestGateway())
    {
        uint8_t command = i2cComms.readByte();
        uint8_t node    = i2cComms.readByte();
        uint8_t option  = command & COMMS_OPTION_MASK;
        uint8_t pin     = option & OUTPUT_PIN_MASK;

        command &= COMMS_COMMAND_MASK;

        switch (command)
        {
            case COMMS_CMD_SYSTEM: if (option == COMMS_SYS_OUT_STATES)
                                   {
                                       i2cComms.sendGateway(COMMS_CMD_SYSTEM | COMMS_SYS_OUT_STATES, getOutputStates(node), -1);
                                       workDone = true;
                                   }
                                   else if (option == COMMS_SYS_INP_STATES)
                                   {
                                       i2cComms.sendGateway(COMMS_CMD_SYSTEM | COMMS_SYS_INP_STATES, (currentSwitchState[node] >> 8) & 0Xff, currentSwitchState[node] & 0xFF);
                                       workDone = true;
                                   }
                                   else
                                   {
                                       systemFail(M_GATEWAY, command | option);
                                   }
                                   break;

            case COMMS_CMD_NONE:   break;

            default:               systemFail(M_GATEWAY, command | option);
        }
    }
//    else
//    {
//        // Turn off Gateway if there's a comms error.
//        i2cComms.setGateway(0);
//    }

    i2cComms.readAll();

    return workDone;
}


//uint8_t * heapPtr, * stackPtr;
//
//void checkMem(const char* aMessage)
//{
//  stackPtr = (uint8_t *)malloc(4);  // use stackPtr temporarily
//  heapPtr = stackPtr;               // save value of heap pointer
//  free(stackPtr);                   // free up the memory again (sets stackPtr to 0)
//  stackPtr =  (uint8_t *)(SP);      // save value of stack pointer
//  Serial.print(aMessage);
//  Serial.print(" ");
//  Serial.print((int)heapPtr);
//  Serial.print(" ");
//  Serial.println((int)stackPtr);
//}


/** Setup the Arduino.
 */
void setup()
{
    // Start Serial IO first - needed if there's any debug output.
    Serial.begin(SERIAL_SPEED);

    // Detect presence of LCD shield using LCD_SHIELD_DETECT_PIN if necessary
#if ! LCD_SHIELD && LCD_SHIELD_DETECT_PIN
    pinMode(LCD_SHIELD_DETECT_PIN, INPUT_PULLUP);
    hasLcdShield = !digitalRead(LCD_SHIELD_DETECT_PIN);
#endif

    if (hasLcdShield)
    {
        disp.createLcdShield();

        // Initial announcement/splash message.
        controller.announce();
        disp.printProgStrAt(LCD_COL_START, LCD_ROW_DET, M_INIT_I2C, LCD_LEN_STATUS);
    }

    // Initialise I2C.
    Wire.begin(I2C_CONTROLLER_ID);          // I2C network
    // Wire.setTimeout(25000L);             // Doesn't seem to have any effect.

#if LCD_I2C
    // Scan for I2C LCD.
    for (uint8_t id = I2C_LCD_HI; id >= I2C_LCD_LO; id--)
    {
        if (i2cComms.exists(id))
        {
            disp.setLcd(id);
            controller.announce();          // Again for I2C LCD.
            break;
        }
    }
#endif

    // Force an LCD shield if no I2C LCD present.
    if (   (!hasLcdShield)
        && (disp.getLcdId() == 0))
    {
        hasLcdShield = true;
        disp.createLcdShield();
        controller.announce();
    }

    // Check for I2C Gateway.
    if (i2cComms.exists(I2C_GATEWAY_ID))
    {
        i2cComms.setGateway(I2C_GATEWAY_ID);
    }

    // Initialise
    disp.printProgStrAt(LCD_COL_START, LCD_ROW_DET, M_STARTUP, LCD_LEN_STATUS);

    flashVersion();                                     // Flash our version number on the built-in LED.

    // Deal with first run (software has never been run before).
    if (!loadSystemData())
    {
        firstRun();
    }
    else if (   (hasLcdShield)
             && (   (buttons.calibrationRequired())             // Calibration required if it's never been done.
                 || (buttons.readButton() != BUTTON_NONE)))     // or an input button is being pressed.
    {
        buttons.calibrateButtons();
        saveSystemData();
    }

    // Dump memory in raw format if debug-full.
    if (isDebug(DEBUG_FULL))
    {
        dumpMemory();
    }

    // Check if version update required.
    if (systemData.version != VERSION)
    {
        if (isDebug(DEBUG_NONE))
        {
            Serial.print(PGMT(M_UPDATE));
            Serial.print(CHAR_SPACE);
            Serial.print(systemData.version, HEX);
            Serial.print(CHAR_SPACE);
            Serial.print(VERSION, HEX);
            Serial.println();
        }

        disp.printProgStrAt(LCD_COL_START, LCD_ROW_DET, M_UPDATE, LCD_LEN_STATUS);

        // Do the update here.
        buttons.waitForButtonClick();           // Nothing to do, just show it's happening.

        systemData.version = VERSION;
        saveSystemData();
    }

    // Scan for Input and Output nodes.
    controller.scanHardware();
}


/** Main loop.
 */
void loop()
{
    // Press any button to configure.
    if (buttons.readButton())
    {
        configure.run();
        controller.announce();
    }

    handleSerialInput();

    now = millis();

    // Rescan for new hardware
    if (now > tickHardwareScan)
    {
        tickHardwareScan = now + STEP_HARDWARE_SCAN;
        controller.scanInputHardware();
        controller.scanOutputHardware();
    }

    // Process any inputs
    if (now > tickInputScan)
    {
        tickInputScan = now + STEP_INPUT_SCAN;
        // scanOutputs();
        controller.scanInputs(NULL);
    }

    // See if there are any Gateway requests
    if (now > tickGateway)
    {
        if (!gatewayRequest())
        {
            tickGateway = now + STEP_GATEWAY;
        }
    }

    // Show heartbeat.
    if (now > tickHeartBeat)
    {
        tickHeartBeat = now + STEP_HEARTBEAT;
        controller.showHeartBeat();
    }
}
