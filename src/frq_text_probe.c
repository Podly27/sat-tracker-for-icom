/* Experimental APRS-derived native display-cache test. It publishes into
 * the final UI cache, not the RX packet mailbox. No code patching, firmware
 * writes, CI-V, PTT, RX-history events or SD writes. Hardware UI test pending. */
#include "icap.h"
#include "gps.h"
#include "orbit.h"
#include "tuner.h"
#include "aprs_status.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
static unsigned ui_profile;
static Orbit satellite;static char data[512];
#ifdef ISS_HOST_TEST
extern int probe_ui_supported(Api*);
extern unsigned probe_ui_code(void);
extern unsigned probe_live_mode(void);
extern void probe_redraw(void);
#else
static int probe_ui_supported(Api*a){
 if(a->build!=0x07051405)return 0;
 if((uintptr_t)a->radio_state==0x2037bac9&&(uintptr_t)a->clock==0x2037bf27){ui_profile=8;return 1;}
 if((uintptr_t)a->radio_state==0x2037bbc1&&(uintptr_t)a->clock==0x2037c017){ui_profile=14;return 1;}
 return 0;
}
static unsigned probe_ui_code(void){return *(volatile const unsigned char*)0x204b035d;}
/* APRS.APP IC-705 E profile uses this current normalized mode byte;
 * its FM group is 7. The ICAP VFO cache can remain packed 0x70 while
 * the radio's native screen is FM, as confirmed in the device test. */
static unsigned probe_live_mode(void){return (*(volatile const unsigned char*)0x203e212f)>>4;}
static void probe_redraw(void){
 /* Known native UI redraw flags; no radio mode/frequency or RX-message fields. */
 *(volatile uint16_t*)0x20380698|=1;
 *(volatile uint8_t*)0x203806f7|=0x40;
}
#endif
static int elements(Api*a,const char*name){
 int fd=a->open(name,0);if(fd<0)return -1;
 int n=a->read(fd,data,sizeof(data)-1);a->close(fd);if(n<=0||n>=(int)sizeof(data))return -1;data[n]=0;
 char *one=data;while(*one&&!(one[0]=='1'&&one[1]==' ')){one=strchr(one,'\n');if(!one)return -1;one++;}
 char *two=strchr(one,'\n');if(!two)return -1;*two++=0;char*end=strchr(two,'\n');if(end)*end=0;
 n=strlen(one);if(n&&one[n-1]=='\r')one[n-1]=0;n=strlen(two);if(n&&two[n-1]=='\r')two[n-1]=0;
 return orbit_init(&satellite,one,two);
}
#ifndef ISS_HOST_TEST
__attribute__((section(".entry")))
#endif
int app_main(Api*a){
 if(!a||a->abi!=3||a->radio!=0x705||a->size<0x54||!a->line||!a->clear||!a->sleep||!a->stop||!a->clock||!a->radio_state||!a->open||!a->read||!a->close)return -1;
 a->clear();a->line(0,"FRQ TEXT TEST / 5 APRS FM");
 a->line(1,"No tuning / no PTT / no RX log");
 int drawn=0,cache_supported=aprs_status_supported(a);unsigned attempts=0,main_count=0,other_count=0,last_fg=256,last_mode=0,seen[3]={256,256,256};
 if(!probe_ui_supported(a)){a->line(3,"Unsupported native UI bindings");goto done;}
 if(elements(a,"ISS.TLE")&&elements(a,"SATELITE.TLE")){a->line(3,"TLE missing / invalid");goto done;}
 a->line(4,"EXIT hides this Apps window");a->line(5,"Then check text below frequency");
 while(!a->stop()){
  RadioClock c;GpsPosition p;Look q;RadioState radio={0};char text[40];
  if(a->clock(&c)<0||c.year<2025||c.year>2099||c.month<1||c.month>12||c.day<1||c.day>31){a->line(3,"Set radio clock to UTC");a->sleep(500);continue;}
  uint32_t now=utc_seconds(c.year,c.month,c.day,c.hour,c.minute,c.second);
  if(fabs((double)now-satellite.epoch)>7*86400){a->line(3,"TLE expired: update SD card");a->sleep(500);continue;}
  if(gps_read(&p)!=GPS_OK){a->line(3,"GPS: WAITING FOR FIX");a->sleep(500);continue;}
  if(orbit_look(&satellite,now,p.lat,p.lon,0,&q)){a->line(3,"Orbit calculation failed");a->sleep(500);continue;}
  snprintf(text,sizeof(text),"ISS AZ%03u EL%+03d %s",(unsigned)(q.az+0.5)%360,(int)q.el,q.el>=0?"IN RANGE":"BELOW");
  a->line(3,text);
  /* Publish only during FM/RX. The original UI task chooses when to draw
   * its receiver-status cache; this never draws over a menu from the app. */
  unsigned fg=probe_ui_code(),live_mode=probe_live_mode();int result=a->radio_state(&radio);
  for(unsigned i=0;i<3;i++){if(seen[i]==fg)break;if(seen[i]==256){seen[i]=fg;break;}}
  if(fg==0)main_count++;else other_count++;
  if(result==0&&!radio.tx&&live_mode==7&&cache_supported){
   char compact[16];snprintf(compact,sizeof(compact),"ISS A%03u E%+03d",(unsigned)(q.az+0.5)%360,(int)q.el);
   int published=aprs_status_publish(compact);
   if(!published){drawn=1;attempts++;last_fg=fg;last_mode=radio.mode;}
   else{a->line(2,"RX display changed / STOP test");break;}
  }
  char diagnostic[32];
  snprintf(diagnostic,sizeof(diagnostic),"C:%02X LIVE:%X TX:%u R:%d",radio.mode,live_mode,radio.tx,result);a->line(4,diagnostic);
  snprintf(diagnostic,sizeof(diagnostic),"PUB:%lu MAIN:%lu ELSE:%lu",(unsigned long)attempts,(unsigned long)main_count,(unsigned long)other_count);a->line(5,diagnostic);
  snprintf(diagnostic,sizeof(diagnostic),"UI:%02X/%02X/%02X FW:%u",seen[0],seen[1],seen[2],ui_profile);a->line(6,diagnostic);
  if(live_mode!=7)a->line(2,"Set normal radio mode to FM");
  else if(!cache_supported)a->line(2,"Unsupported APRS display cache");
  else a->line(2,"APRS display cache / no RX log");
  snprintf(diagnostic,sizeof(diagnostic),"Last pub FG:%02X M:%02X",last_fg,last_mode);a->line(7,diagnostic);
  a->sleep(500);
 }
 done:
 if(drawn){aprs_status_end();probe_redraw();}
 while(!a->stop())a->sleep(100);
 return 0;
}
