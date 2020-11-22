/** Input data.
 */
#ifndef _Report_h
#define _Report_h

#define REPORT_OFF      0       // No REPORTING.
#define REPORT_SHORT    1       // Short reports.
#define REPORT_LONG     2       // Long reports.
#define REPORT_PAUSE    3       // Reports pause for acknowledgement.
#define REPORT_MAX      4       // Maximum report option.


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
