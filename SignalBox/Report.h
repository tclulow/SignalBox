/** Reporting.
 */
#ifndef _Report_h
#define _Report_h


/** Is reporting enabled (at a particular level)?
 */
boolean reportEnabled(int aLevel);


/** Length of time to wait for depending on the reporting level.
 */
int reportDelay();


/** Pause for user-input if so configured.
 */
void reportPause();


#endif
