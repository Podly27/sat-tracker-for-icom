#ifndef ISS_TX_CONTROL_H
#define ISS_TX_CONTROL_H
#include "icap.h"
const char *tx_control_tick(Api*,uint32_t up,int gps,int orbit,int stopping,uint32_t rx);
int tx_control_busy(void);
uint32_t tx_control_up(void);
uint32_t tx_control_verified(void);
const char *tx_control_error(void);
int tx_control_reset(void);
void tx_control_finish(Api*,uint32_t rx);
#endif
