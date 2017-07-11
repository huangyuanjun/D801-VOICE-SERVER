
#include "vproc_common.h"

/* Vproc_msDelay(): use this function to
 *     force a delay of specified time in resolution of milli-second
 *
 * Input Argument: time in unsigned 16-bit
 * Return: none
 */

void Vproc_msDelay(unsigned short time) {
     usleep(time * 1000); /*PLATFORM SPECIFIC - system wait in ms*/
}

 /* VprocWait(): use this function to
 *     force a delay of specified time in resolution of 125 micro-Seconds
 *
 * Input Argument: time in unsigned 32-bit
 * Return: none
 */
void VprocWait(unsigned long int time) {
     usleep(125 * time); /*system wait in frame*/
}
