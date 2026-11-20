#ifndef ISS_SAT_CONTROL_H
#define ISS_SAT_CONTROL_H
#include "icap.h"
const char *sat_control_tick(Api*,uint32_t down,uint32_t up,int gps,int orbit,int stopping);
void sat_control_finish(Api*);
uint32_t sat_control_rx(void);
uint32_t sat_control_up(void);
#endif
