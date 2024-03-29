/** SignalBox.
 *  @file
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
 *  D12     Interlock warning buzzer (see Config INTERLOCK_BUZZER_PIN).
 *  D13     Flash firmare version and Interlock warning LED (see INTERLOCK_WARNING_PIN).
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


#include "Config.h"                 // Common classes
#include "Messages.h"
#include "Persisted.h"
#include "SystemMgr.h"
#include "I2cComms.h"
#include "OutputDef.h"

#include "Forward.h"                // SignalBox-specific classes.
#include "Display.h"
#include "InputDef.h"
#include "InputMgr.h"
#include "OutputCtl.h"
#include "EzyBus.h"

#include "Buttons.h"
#include "ImportExport.h"
#include "Controller.h"
#include "Configure.h"
#include "Command.h"
#include "Cmri.h"


/** Report a system failure.
 */
void systemFail(PGM_P aMessage, int aValue)
{
    if (isDebug(DEBUG_ERRORS))
    {
        Serial.print(PGMT(M_FAILURE));
        Serial.print(CHAR_TAB);
        Serial.print(PGMT(aMessage));
        Serial.print(CHAR_SPACE);
        Serial.print(aValue, HEX);
        Serial.println();
    }

    disp.clear();
    disp.printProgStrAt(LCD_COL_START, LCD_ROW_EDT, M_FAILURE);
    disp.printProgStrAt(LCD_COL_START, LCD_ROW_BOT, aMessage);
    disp.printHexByteAt(-2, LCD_ROW_BOT, aValue);

    controller.setDisplayTimeout(DELAY_FAIL);
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
            for (uint8_t index = 0; index < INPUT_OUTPUT_MAX; index++)
            {
                inputDef.setOutput(index, 0);       // Zero-length delay.
                inputDef.setDelay(index, true);
            }

            inputMgr.saveInput();
            inputNumber += 1;       // Step through Input numbers.
        }
    }
}


/** Software hasn't been run before.
 */
void firstRun()
{
    // Calibrate the LCD buttons.
    if (disp.hasShield())
    {
        buttons.calibrateButtons();
    }


#if EZYBUS_CONVERT

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
    
#endif

    {
        defaultInputs(INPUT_TYPE_ON_OFF);
    }

    // Save all data to EEPROM.
    systemMgr.saveSystemData();

    buttons.waitForButtonClick();
}


/** Pause for user-input if so configured.
 *  Use buttons to adjust report level.
 */
void reportPause()
{
    if (systemMgr.getReportLevel() >= REPORT_PAUSE)
    {
        switch (buttons.waitForButtonPress())
        {
            case BUTTON_NONE:   break;

            case BUTTON_UP:     systemMgr.setReportLevel(REPORT_LONG);
                                systemMgr.saveSystemData();
                                break;

            case BUTTON_DOWN:   systemMgr.setReportLevel(REPORT_SHORT);
                                systemMgr.saveSystemData();
                                break;

            case BUTTON_LEFT:   systemMgr.setReportLevel(REPORT_NONE);
                                systemMgr.saveSystemData();
                                break;

            case BUTTON_RIGHT:  configure.run();
                                break;

            case BUTTON_SELECT: break;
        }

        // Show (new) report level.
        disp.clearRow(LCD_COL_START, LCD_ROW_BOT);
        disp.printProgStrAt(LCD_COL_START,  LCD_ROW_BOT, M_REPORT);
        disp.printProgStrAt(LCD_COL_REPORT_PARAM, LCD_ROW_BOT, M_REPORT_PROMPTS[systemMgr.getReportLevel()], LCD_LEN_OPTION);

        buttons.waitForButtonRelease();
        disp.clearRow(LCD_COL_START, LCD_ROW_BOT);
    }
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


#if SERIAL_COMMAND
Command command;                    // Serial command handler.
#endif

#if SERIAL_CMRI
Cmri cmri(Serial);                  // Cmri handler using Serial.
#endif


/** Setup the Arduino.
 */
void setup()
{
    bool hasLcdShield = LCD_SHIELD;              // An LCD shield is present.

    // Start Serial IO first - needed if there's any debug output.
    Serial.begin(SERIAL_SPEED);

    // Detect presence of LCD shield using LCD_SHIELD_DETECT_PIN if necessary
    if (   (! LCD_SHIELD)
        && (LCD_SHIELD_DETECT_PIN > 0))
    {
        pinMode(LCD_SHIELD_DETECT_PIN, INPUT_PULLUP);
        hasLcdShield = !digitalRead(LCD_SHIELD_DETECT_PIN);
    }

    if (hasLcdShield)
    {
        disp.createLcdShield();

        // Initial announcement/splash message.
        controller.announce();
        disp.printProgStrAt(LCD_COL_START, LCD_ROW_DET, M_INIT_I2C, LCD_LEN_STATUS);
    }

    // Initialise I2C.
    i2cComms.setId(I2C_CONTROLLER_ID);          // I2C network

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

//    // Force an LCD shield if no I2C LCD present.
//    if (   (!hasLcdShield)
//        && (disp.getLcdId() == 0))
//    {
//        hasLcdShield = true;
//        disp.createLcdShield();
//        controller.announce();
//    }

    // Check for I2C Gateway.
    if (   (I2C_GATEWAY_ID > 0)
        && (i2cComms.exists(I2C_GATEWAY_ID)))
    {
        i2cComms.setGateway(I2C_GATEWAY_ID);
    }

    // Initialise
    disp.printProgStrAt(LCD_COL_START, LCD_ROW_DET, M_STARTUP, LCD_LEN_STATUS);

    systemMgr.flashVersion();                                   // Flash our version number on the built-in LED.

    // Deal with first run (software has never been run before).
    if (!systemMgr.loadSystemData())
    {
        firstRun();
    }
    else if (   (hasLcdShield)
             && (   (buttons.calibrationRequired())             // Calibration required if it's never been done.
                 || (buttons.readButton() != BUTTON_NONE)))     // or an input button is being pressed.
    {
        buttons.calibrateButtons();
        systemMgr.saveSystemData();
        controller.announce();                                  // Re-announce. Calibration will have overwritten the display.
    }

    // buttons.displayButtons();
    
    // // Dump memory in raw format if debug-full.
    // if (isDebug(DEBUG_FULL))
    // {
    //     dumpMemory();
    // }

    // // Report EEPROM object locations and sizes.
    // Serial.print("System base=");
    // Serial.print(systemMgr.getBase(), HEX);
    // Serial.print(", end=");
    // Serial.print(systemMgr.getEnd(), HEX);
    // Serial.println();
    // Serial.print("Input  base=");
    // Serial.print(inputMgr.getBase(), HEX);
    // Serial.print(", end=");
    // Serial.print(inputMgr.getEnd(), HEX);
    // Serial.println();

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

        disp.printProgStrAt(LCD_COL_START, LCD_ROW_DET, M_UPDATE, LCD_LEN_STATUS);

        // Do the update here.
        systemMgr.update();
        buttons.waitForButtonClick();           // Nothing to do, just show it's happening.
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

#if SERIAL_CMRI
#if SERIAL_COMMAND
    if (command.isIdle())   // Don't interfere with command if it's busy.
#endif
    {
        cmri.update();      // Handle CMRI processing
    }
#endif

#if SERIAL_COMMAND
#if SERIAL_CMRI
    if (cmri.isIdle())      // Don't interfere with CMRI if it's busy.
#endif
    {
        command.update();   // Handle serial command processing.
    }
#endif

    // Show heartbeat, sound buzzer, etc.
    controller.update();
}
