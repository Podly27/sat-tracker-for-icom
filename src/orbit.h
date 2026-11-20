#ifndef ISS_ORBIT_H
#define ISS_ORBIT_H
#include <stdint.h>
#include "SGP4.h"
typedef struct { ElsetRec rec; double epoch; } Orbit;
typedef struct { double az, el, range, rate; int32_t down_hz, up_hz; } Look;
uint32_t utc_seconds(int y,int m,int d,int h,int min,int s);
int orbit_init(Orbit *o,const char *l1,const char *l2);
int orbit_look(Orbit *o,uint32_t utc,double lat,double lon,double alt,Look *out);
int orbit_init_id(Orbit*,const char*,const char*,const char *catalog_id);
int orbit_look_profile(Orbit*,uint32_t,double,double,double,uint32_t down,uint32_t up,Look*);
int grid_position(const char *s,double *lat,double *lon);
#endif
