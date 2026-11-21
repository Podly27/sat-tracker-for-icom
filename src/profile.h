#ifndef SAT_PROFILE_H
#define SAT_PROFILE_H
#include <stdint.h>
extern uint32_t profile_rx,profile_tx;
extern unsigned profile_tone;
int profile_valid(uint32_t rx,uint32_t tx,unsigned tone);
void profile_set(uint32_t rx,uint32_t tx,unsigned tone);
int profile_target(unsigned up,uint32_t hz);
#endif
