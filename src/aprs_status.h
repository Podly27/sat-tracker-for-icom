#ifndef ISS_APRS_STATUS_H
#define ISS_APRS_STATUS_H
#include "icap.h"
/* Display cache only: no receive-header/mailbox publication, logging, RF or PTT. */
int aprs_status_supported(Api*);
int aprs_status_publish(const char*);
int aprs_status_needs_restore(void);
/* Read-only main-screen snapshots; call only after supported() succeeds. */
const char *aprs_status_debug(unsigned);
int aprs_status_end(void);
/* Clear owned characters only: no native notification or old UI flag replay. */
int aprs_status_release(void);
/* Optional diagnostic callback runs only with interrupts restored. */
int aprs_status_end_trace(void (*trace)(unsigned));
#endif
