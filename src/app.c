#include "icap.h"
#include "orbit.h"
#include "gps.h"
#include "civ_queue.h"
#include "tuner.h"
#ifdef ISS_RX_CONTROL
#include "rx_control.h"
#endif
#ifdef ISS_SAT_CONTROL
#include "sat_control.h"
#include "radio_session.h"
#endif
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#define APP_DISPLAY "ISS tracker"
#define APP_TLE "ISS.TLE"
static Api *api;static Orbit orbit;static Tuner tuner;static char linebuf[8][32],filebuf[512];
static const char *bridge="CIV PROBE NOT RUN";
static const char *control(Api *a,uint32_t down,uint32_t up,int gps,int valid){
#ifdef ISS_SAT_CONTROL
 int capture=radio_session_capture(a);
 if(capture!=1)return radio_session_status();
 return sat_control_tick(a,down,up,gps,valid,0);
#elif defined(ISS_RX_CONTROL)
 (void)up;return rx_control_tick(a,down,gps,valid,0);
#elif defined(ISS_MONITOR)
 (void)a;(void)down;(void)up;(void)gps;(void)valid;
#ifdef ISS_OBSERVE
 extern const char *civ_observe(void);
 return civ_observe();
#else
 return "MONITOR 2 / NO RADIO CONTROL";
#endif
#else
 return tuner_tick(&tuner,a,down,up,gps,valid);
#endif
}
static double lat,lon,alt;static uint32_t nextscan,aos,los,scanned,scanend;static int inpass;
static void show(unsigned n,const char*s){api->line(n,s);}
static int readfile(const char *name,char *buf,int cap) {
 int h=api->open(name,0);if(h<0)return h;int n=api->read(h,buf,cap-1);api->close(h);
 if(n<0)return n;buf[n]=0;return n;
}
static void status(const char *text){api->clear();show(0,APP_DISPLAY " / IC-705");show(2,text);show(7,bridge);}
static int elements(void){
 if(readfile(APP_TLE,filebuf,sizeof(filebuf))<0)return -1;
 char *a=filebuf;while(*a&&!(a[0]=='1'&&a[1]==' ')){a=strchr(a,'\n');if(!a)return -1;a++;}
 char *b=strchr(a,'\n');if(!b)return -1;*b++=0;int n=strlen(a);if(n&&a[n-1]=='\r')a[n-1]=0;
 char *end=strchr(b,'\n');if(end)*end=0;n=strlen(b);if(n&&b[n-1]=='\r')b[n-1]=0;
 return orbit_init(&orbit,a,b);
}
static void wait_error(const char *s){show(0,APP_DISPLAY " / IC-705");show(2,s);show(4,"No radio settings changed");while(!api->stop())api->sleep(250);}
static void pass_reset(uint32_t now){aos=los=0;nextscan=now;scanned=0;scanend=now+86400;inpass=0;}
static void pass_work(void){
 Look q;for(int i=0;i<3&&nextscan<=scanend&&!los;i++,nextscan+=30){
  if(orbit_look(&orbit,nextscan,lat,lon,alt,&q))return;
  if(q.el>=0&&!inpass){aos=nextscan;inpass=1;}
  if(q.el<0&&inpass){los=nextscan;return;}scanned++;
 }
}
static void freq(char*b,int32_t hz){snprintf(b,16,"%03ld.%03ld.%03ld",(long)(hz/1000000),(long)((hz/1000)%1000),(long)(hz%1000));}
__attribute__((section(".entry"))) int app_main(Api *a){
 api=a;if(!a||a->radio!=0x705||a->build!=0x07051405||a->abi<3||a->size<0x54||!a->clock)return -1;
#if defined(ISS_RX_CONTROL)||defined(ISS_SAT_CONTROL)
 if(!a->radio_state||!a->ticks||!a->stop||!a->sleep||!a->line||!a->clear)return -1;
#endif
 a->clear();
#ifdef ISS_SAT_CONTROL
 bridge="SAT HOLD: STARTING";
#elif defined(ISS_RX_CONTROL)
 bridge="RX HOLD: STARTING";
#elif defined(ISS_MONITOR)
 bridge="MONITOR 2 / NO RADIO CONTROL";
#else
 int probe=civ_read_probe_begin();
 if(probe==0){for(unsigned i=0;i<10&&civ_read_probe_pending();i++)a->sleep(100);bridge=civ_read_probe_pending()?"CIV QUEUE: TIMEOUT":"CIV QUEUE: CONSUMED";}
 else bridge=probe==-2?"CIV QUEUE: UNSUPPORTED":"CIV QUEUE: BUSY";
#endif
 if(elements()){wait_error(APP_TLE " invalid / missing");return -4;}
 uint32_t prev=0;double oldlat=1000,oldlon=1000;
 while(!a->stop()){
  RadioClock c;if(a->clock(&c)<0||c.year<2025||c.year>2099||c.month<1||c.month>12||c.day<1||c.day>31){bridge=control(a,0,0,0,0);status("Set radio clock to UTC");a->sleep(250);continue;}
  uint32_t now=utc_seconds(c.year,c.month,c.day,c.hour,c.minute,c.second);
  GpsPosition pos;int gps=gps_read(&pos);
  if(gps!=GPS_OK){bridge=control(a,0,0,0,0);status(gps==GPS_NO_FIX?"GPS: WAITING FOR FIX":gps==GPS_UNSUPPORTED?"GPS: FIRMWARE MISMATCH":gps==GPS_UNSTABLE?"GPS: UPDATING POSITION":"GPS: INVALID COORDINATES");show(4,"Use internal GPS / clear sky");a->sleep(1000);continue;}
  lat=pos.lat;lon=pos.lon;alt=0;
  if(!prev||now<prev||now-prev>5||fabs(oldlat-lat)>0.02||fabs(oldlon-lon)>0.02||(los&&now>los))pass_reset(now);
  oldlat=lat;oldlon=lon;prev=now;
  if(fabs((double)now-orbit.epoch)>7*86400){bridge=control(a,0,0,1,0);status(APP_TLE " expired (>7 days)");a->sleep(1000);continue;}
  Look q;if(orbit_look(&orbit,now,lat,lon,alt,&q)){bridge=control(a,0,0,1,0);status("Orbit calculation failed");a->sleep(1000);continue;}
  bridge=control(a,q.down_hz,q.up_hz,1,1);
  pass_work();
  snprintf(linebuf[0],32,"ISS / FM REPEATER  %s",q.el>=0?"IN RANGE":"BELOW");
  snprintf(linebuf[1],32,"%02u:%02u:%02u UTC  %s",c.hour,c.minute,c.second,"GPS FIX");
  snprintf(linebuf[2],32,"AZ %03d  EL %+03d  %5ldkm",(int)(q.az+0.5),(int)q.el,(long)(q.range+0.5));
  char f[16];freq(f,q.down_hz);snprintf(linebuf[3],32,"RX %s FM",f);
#ifdef ISS_SAT_CONTROL
  uint32_t actual_rx=sat_control_rx(),actual_up=sat_control_up();
  freq(f,actual_rx?actual_rx:q.down_hz);snprintf(linebuf[3],32,"RX A %s",f);
  freq(f,actual_up?actual_up:q.up_hz);snprintf(linebuf[4],32,"TX B %s SPLIT",f);
#elif defined(ISS_RX_CONTROL)
  snprintf(linebuf[3],32,"TARGET %s",f);
  RadioState actual={0};
  if(a->radio_state(&actual)==0){freq(f,actual.main);snprintf(linebuf[4],32,"ACTUAL %s",f);}
  else snprintf(linebuf[4],32,"Actual RX unavailable");
#else
  freq(f,q.up_hz);snprintf(linebuf[4],32,"UP %s T67.0",f);
#endif
  if(q.el>=0&&!los)snprintf(linebuf[5],32,"Finding end of pass...");
  else if(aos)snprintf(linebuf[5],32,"%s %02lu:%02lu:%02lu UTC",q.el>=0?"LOS":"AOS",(unsigned long)(((q.el>=0?los:aos)/3600)%24),(unsigned long)(((q.el>=0?los:aos)/60)%60),(unsigned long)((q.el>=0?los:aos)%60));
  else if(nextscan>scanend)snprintf(linebuf[5],32,"No pass in next 24 hours");
  else snprintf(linebuf[5],32,"Finding next pass... %lu%%",(unsigned long)(scanned*100/2881));
  linebuf[6][0]=linebuf[7][0]=0;
  /* Keep operational errors visible, without coordinates or ACK diagnostics. */
  if(strstr(bridge,"STOP:")||strstr(bridge,"ERROR")||strstr(bridge,"SAVE:"))snprintf(linebuf[7],32,"%s",bridge);
  for(unsigned i=0;i<8;i++)show(i,linebuf[i]);
  a->sleep(100);
 }
#ifdef ISS_SAT_CONTROL
 sat_control_finish(a);
 a->clear();show(0,APP_DISPLAY " / STOP");
 uint32_t restore_start=a->ticks();int restored;
 while((restored=radio_session_restore(a))==0&&a->ticks()-restore_start<30000){
  show(3,radio_session_status());a->sleep(50);
 }
 show(3,restored==1?"Original settings restored":radio_session_status());
 if(restored!=1){show(5,"Restore incomplete - check TRX");a->sleep(3000);}
#elif defined(ISS_RX_CONTROL)
 rx_control_finish(a);
#elif !defined(ISS_MONITOR)
 /* Restore A if STOP occurs in the brief tone setup window; never key PTT. */
 if(tuner.step>=6&&tuner.step<=9){
  for(unsigned i=0;i<100&&civ_read_probe_pending();i++)a->sleep(10);
  RadioState stopped={0};
  if(!civ_read_probe_pending()&&a->radio_state&&a->radio_state(&stopped)==0&&!stopped.tx){
   if(!civ_iss_profile(9))for(unsigned i=0;i<100&&civ_read_probe_pending();i++)a->sleep(10);
  }
 }
#endif
 return 0;
}
