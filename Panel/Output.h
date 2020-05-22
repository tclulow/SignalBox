/** Output data.
 */
#ifndef _Output_h
#define _Output_h

// Output modules.
#define OUTPUT_MODULE_SIZE   8      // 8 outputs to each module.
#define OUTPUT_MODULE_MAX    16     // Maximum modules.
#define OUTPUT_MODULE_SHIFT  4      // Shift output number this amount to get a module number.
#define OUTPUT_OUTPUT_MASK   0x0f   // Mask to get output number within a module.

// OutputData saved in EEPROM
#define OUTPUT_BASE  0                                          // EEPROM base of Output data.
#define OUTPUT_SIZE  sizeof(OutputData)                         // Size of OutputData entry.
#define OUTPUT_MAX   (OUTPUT_MODULE_SIZE * OUTPUT_MODULE_MAX)   // Maximum outputs (up to 128).
#define OUTPUT_END   (OUTPUT_BASE + OUTPUT_SIZE * OUTPUT_MAX)   // End of Switch EEPROM.

// Mask for OUTPUT options
#define OUTPUT_STATE        0x80
#define OUTPUT_MODE_MASK    0x0f
#define OUTPUT_MODE_NONE    0x00
#define OUTPUT_MODE_SERVO   0x01
#define OUTPUT_MODE_LED     0x02
#define OUTPUT_MODE_SIGNAL  0x03

/** Data describing an Output's operation.
 */
struct OutputData
{
  uint8_t mode  = 0;
  uint8_t lo    = 0;
  uint8_t hi    = 0;
  uint8_t pace  = 0;
};


/** Variables for working with an Output.
 */
int        outputModules = 0;   // Bit map of Output modules present.
int        outputNumber  = 0;   // Current Output number.
OutputData outputData;          // Data describing current Output.


/** Load an Output's data from EEPROM.
 */
void loadOutput(int aOutput)
{
  if (aOutput < OUTPUT_MAX)
  {
    outputNumber = aOutput;
    EEPROM.get(OUTPUT_BASE + outputNumber * OUTPUT_SIZE, outputData); 
  }
}


/** Load an Output's data from EEPROM.
 */
void loadOutput(int aModule, int aOutput)
{
  loadOutput((aModule << OUTPUT_MODULE_SHIFT) + (aOutput & OUTPUT_OUTPUT_MASK));
}


/** Load an Output's data from EEPROM.
 */
void loadOutputModule(int aModule, int aOutput)
{
  loadOutput((aModule << OUTPUT_MODULE_SHIFT) + (aOutput & OUTPUT_OUTPUT_MASK));
}


/** Save an Output's data to EEPROM.
 *  Data in outputNumber and outputData.
 */
void saveOutput()
{
  if (outputNumber < OUTPUT_MAX)
  {
    EEPROM.put(OUTPUT_BASE + outputNumber * OUTPUT_SIZE, outputData);
  }
}


/** Record the presence of an OutputModule in the map.
 */
void setOutputModulePresent(int aModule)
{
  outputModules |= (1 << aModule); 
}


/** Is an Output module present?
 *  Look for Output's module in outputModules.
 */
boolean isOutputModule(int aModule)
{
  return outputModules & (1 << aModule);
}


/** Is an Output present?
 *  Look for output's module in outputModules.
 */
boolean isOutput(int aOutput)
{
  return isOutputModule(aOutput >> OUTPUT_MODULE_SHIFT);
}

#endif
