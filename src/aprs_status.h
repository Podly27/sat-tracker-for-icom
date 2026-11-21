#ifndef ISS_APRS_STATUS_H
#define ISS_APRS_STATUS_H
#include "icap.h"
/* Display cache only: no receive-header/mailbox publication, logging, RF or PTT. */
int aprs_status_supported(Api*);
int aprs_status_publish(const char*);
int aprs_status_end(void);
#endif
