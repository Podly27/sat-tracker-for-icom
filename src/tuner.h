#ifndef ISS_TUNER_H
#define ISS_TUNER_H
#include <stdint.h>
#include "icap.h"
typedef struct {uint32_t main,sub;uint8_t mode,tx;uint16_t reserved;} RadioState;
/* Checked against the 12-byte original ICAP callback; TX is read-only. */
typedef struct {unsigned step,waiting,phase;uint32_t sent_at,last_at,last_rx,last_up,last_tx_at;int failed,started,restore_a;} Tuner;
const char *tuner_tick(Tuner*,Api*,uint32_t down,uint32_t up,int gps_valid,int orbit_valid);
#endif
