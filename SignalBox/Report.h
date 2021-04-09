/** Reporting.
 *
 *
 *  (c)Copyright Tony Clulow  2021    tony.clulow@pentadtech.com
 *
 *  This work is licensed under the:
 *      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 *      http://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 *  For commercial use, please contact the original copyright holder(s) to agree licensing terms
 */
#ifndef Report_h
#define Report_h


/** Is reporting enabled (at a particular level)?
 */
boolean reportEnabled(uint8_t aLevel);


/** Length of time to wait for depending on the reporting level.
 */
int reportDelay();


/** Pause for user-input if so configured.
 */
void reportPause();


#endif
