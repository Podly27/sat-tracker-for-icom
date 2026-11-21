#ifndef ISS_TLE_STORE_H
#define ISS_TLE_STORE_H
#include "icap.h"
#include "orbit.h"
/* Startup only: returns 0 with usable orbit, -1 without data, -2 on STOP.
 * Alternates ISS.TLE / ISS.BAK; never overwrites the selected valid source. */
int tle_startup(Api*,Orbit*);
int tle_store_load(Api*,uint32_t,Orbit*,unsigned *slot);
int tle_store_save(Api*,unsigned protected_slot,const char*,const char*);
#endif
