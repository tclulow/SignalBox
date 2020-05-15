/** PROGMEM messages
 */
 
const char message_p[] PROGMEM = "Progmem";


char* getMessage(PGM_P progmemMessage)
{
  static char buffer[16];
  strcpy_P(buffer, progmemMessage);
  return buffer;
}
