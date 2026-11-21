/* Isolated native information-dialog check. No CI-V, PTT or SD writes. */
#include "icap.h"
#include "gps.h"
#include "orbit.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
static Orbit satellite;static char data[512],popup_text[256];
static int elements(Api*a,const char*name){
 int fd=a->open(name,0);if(fd<0)return -1;
 int n=a->read(fd,data,sizeof(data)-1);a->close(fd);if(n<=0||n>=(int)sizeof(data))return -1;data[n]=0;
 char *one=data;while(*one&&!(one[0]=='1'&&one[1]==' ')){one=strchr(one,'\n');if(!one)return -1;one++;}
 char *two=strchr(one,'\n');if(!two)return -1;*two++=0;
 char *end=strchr(two,'\n');if(end)*end=0;
 n=strlen(one);if(n&&one[n-1]=='\r')one[n-1]=0;n=strlen(two);if(n&&two[n-1]=='\r')two[n-1]=0;
 return orbit_init(&satellite,one,two);
}
#ifndef ISS_HOST_TEST
__attribute__((section(".entry")))
#endif
int app_main(Api*a){
 if(!a||a->abi!=3||a->radio!=0x705||a->build!=0x07051405||a->size<0x88||!a->line||!a->clear||!a->sleep||!a->stop||!a->clock||!a->open||!a->read||!a->close)return -1;
 a->clear();a->line(0,"ISS DISPLAY TEST / EXPERIMENTAL");
 a->line(1,"Native text popup / no tuning");
#ifndef ISS_HOST_TEST
 if(((uintptr_t)a->popup!=0x2037c011||(uintptr_t)a->radio_state!=0x2037bac9)&&
    ((uintptr_t)a->popup!=0x2037c101||(uintptr_t)a->radio_state!=0x2037bbc1)){
  a->line(3,"Unsupported native popup API");goto done;
 }
#endif
 if(elements(a,"ISS.TLE")&&elements(a,"SATELITE.TLE")){a->line(3,"TLE missing / invalid");goto done;}
 while(!a->stop()){
  RadioClock c;GpsPosition p;Look q;
  if(a->clock(&c)<0||c.year<2025||c.year>2099||c.month<1||c.month>12||c.day<1||c.day>31){a->line(3,"Set radio clock to UTC");a->sleep(250);continue;}
  uint32_t now=utc_seconds(c.year,c.month,c.day,c.hour,c.minute,c.second);
  if(fabs((double)now-satellite.epoch)>7*86400){a->line(3,"TLE expired: update SD card");goto done;}
  if(gps_read(&p)!=GPS_OK){a->line(3,"GPS: WAITING FOR FIX");a->sleep(250);continue;}
  if(orbit_look(&satellite,now,p.lat,p.lon,0,&q)){a->line(3,"Orbit calculation failed");goto done;}
  unsigned sector=((unsigned)(q.az+22.5)/45)%8;
  static const char *directions[]={"N","NE","E","SE","S","SW","W","NW"};
  static const char *markers[]={"^","/",">","\\","v","/","<","\\"};
  snprintf(popup_text,sizeof(popup_text),
   "ISS AZ %03u deg %s\nEL %+03d deg / %ld km\nN ^     E >     S v     W <\nISS direction: %s %s\n%s\nSnapshot %02u:%02u:%02u UTC\nBACK closes / Apps STOP ends",
   (unsigned)(q.az+0.5)%360,directions[sector],(int)q.el,(long)(q.range+0.5),markers[sector],directions[sector],q.el>=0?"ISS ABOVE HORIZON":"ISS BELOW HORIZON",c.hour,c.minute,c.second);
  a->line(3,"Opening native ISS info popup");
  int r=a->popup(popup_text);
  a->line(4,r==0?"BACK closes the popup":"Popup busy: close other dialog");
  a->line(5,"Snapshot only / no live updates");
  break;
 }
 done:
 a->line(7,"Use Apps STOP to finish test");
 while(!a->stop())a->sleep(100);
 return 0;
}
