/** PROGMEM messages
 */
 
const char message_p[] PROGMEM = "Progmem";


/** Gets a PROGMEM message and copies it to a buffer
 *  for regular usage.
 *  
 *  Note the buffer is a singleton and will be overwritten by the next call.
 */
char* getMessage(PGM_P progmemMessage)
{
  static char buffer[16];
  strcpy_P(buffer, progmemMessage);
  return buffer;
}
