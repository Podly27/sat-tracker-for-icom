/* Read-only bindings, pinned to 705E140 / SQ6EMM 20261119v8.
 * These are the original Icom GPS getters used by SQ6EMM's @MYLOC.
 * Passing 0 explicitly chooses receiver data, without the manual-position fallback.
 * No settings writes, GPS configuration changes, interrupt masking or PTT calls.
 */
#include "gps.h"
#include <string.h>
#include <stddef.h>
typedef int (*FixFn)(void);
typedef void (*CoordFn)(uint8_t out[4],unsigned use_manual);
static const uintptr_t FIX_FN=0x20212cac,LAT_FN=0x20074880,LON_FN=0x200748a0;
int gps_coordinate(const uint8_t raw[4],int latitude,double *out){
 unsigned deg=raw[0],minute=(unsigned)raw[1]*256+raw[2];
 unsigned limit=latitude?90:180;char positive=latitude?'N':'E',negative=latitude?'S':'W';
 if(deg>limit||minute>=60000||(deg==limit&&minute)||(raw[3]!=positive&&raw[3]!=negative))return GPS_BAD_COORDINATES;
 double v=deg+(double)minute/60000.0;*out=raw[3]==negative?-v:v;return GPS_OK;
}
static int signatures(void){
 /* ARM instructions, verified against the supplied and decoded firmware. */
 static const uint32_t f[]={0xe59f0c70,0xe5d00000,0xe7e00150,0xe12fff1e};
 static const uint32_t a[]={0xe3510000,0x0a000003,0xe59f12dc,0xe5911093};
 static const uint32_t b[]={0xe3510000,0x0a000003,0xe59f12bc,0xe5911097};
 return !memcmp((const void*)FIX_FN,f,sizeof(f))&&!memcmp((const void*)LAT_FN,a,sizeof(a))&&!memcmp((const void*)LON_FN,b,sizeof(b));
}
int gps_read(GpsPosition *out){
 out->fix=0;
 if(!signatures())return GPS_UNSUPPORTED;
 FixFn fix=(FixFn)FIX_FN;CoordFn lat=(CoordFn)LAT_FN,lon=(CoordFn)LON_FN;
 for(unsigned retry=0;retry<4;retry++){
  uint8_t a[8],b[8];if(!fix())return GPS_NO_FIX;
  lat(a,0);lon(a+4,0);lat(b,0);lon(b+4,0);
  if(!fix())return GPS_NO_FIX;
  if(memcmp(a,b,sizeof(a)))continue;
  double x,y;int result=gps_coordinate(a,1,&x);if(result)return result;
  result=gps_coordinate(a+4,0,&y);if(result)return result;
  out->lat=x;out->lon=y;out->fix=1;return GPS_OK;
 }
 return GPS_UNSTABLE;
}
