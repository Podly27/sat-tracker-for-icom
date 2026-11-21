#ifndef SAT_CATALOG_H
#define SAT_CATALOG_H
#include "icap.h"
#include "orbit.h"
#define SAT_MAX 8
#define SAT_RECORD 320
/* first is either a real TLE line 1 or CelesTrak CSV prefixed CSV,. */
typedef struct {char name[7],id[6],first[SAT_RECORD],second[70];uint32_t norad,rx,tx,aos,los;unsigned tone,valid,done;double epoch;} Satellite;
extern Satellite satellites[SAT_MAX];extern unsigned satellite_count;
int catalog_config(Api*);
int catalog_config_line(const char*);
int catalog_orbit(const Satellite*,Orbit*);
int catalog_startup(Api*,Orbit*scratch);
void catalog_plan_reset(uint32_t now);
int catalog_plan_refresh(uint32_t now);
int catalog_plan_step(uint32_t now,double lat,double lon,Orbit*scratch);
int catalog_choose(uint32_t now);
const char *catalog_error(void);
#endif
