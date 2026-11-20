#ifndef RADIO_SESSION_H
#define RADIO_SESSION_H
#include "icap.h"
/* Capture before satellite setup; restore only captured, validated settings.
 * 0 in progress, 1 captured/restored, -1 failed. No PTT commands.
 */
int radio_session_capture(Api*);
int radio_session_restore(Api*);
int radio_session_saved(void);
const char *radio_session_status(void);
#endif
