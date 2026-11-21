#include "icap.h"
#include "catalog.h"
#include "gps.h"
#include "tuner.h"
#include "civ_queue.h"
#include "sat_control.h"
#include "radio_session.h"
#include "pass_status.h"
#include "satellite_display.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
static Orbit orbit,scan_orbit;static Api*api;
static char lines[8][32];
static void show(unsigned n,const char*s){api->line(n,s);}
static void message(const char*s){api->clear();show(0,"SAT / IC-705");show(2,s);satellite_display_text(s);}
static void freq(char*b,uint32_t hz){snprintf(b,16,"%03lu.%03lu.%03lu",(unsigned long)(hz/1000000),(unsigned long)((hz/1000)%1000),(unsigned long)(hz%1000));}
#ifndef ISS_HOST_TEST
__attribute__((section(".entry")))
#endif
int app_main(Api*a){
 if(!a||a->abi!=3||a->radio!=0x705||a->build!=0x07051405||a->size<0x54||!a->line||!a->clear||!a->sleep||!a->stop||!a->clock||!a->ticks||!a->radio_state||!a->open||!a->read||!a->close)return -1;
 api=a;
#ifndef ISS_HOST_TEST
 if((uintptr_t)a->radio_state!=0x2037bac9||(uintptr_t)a->clock!=0x2037bf27){a->clear();show(0,"Tracker requires pinned v8");while(!a->stop())a->sleep(250);return -1;}
#endif
 int startup=catalog_startup(a,&scan_orbit);
 if(startup==-2)return 0;
 if(startup){message(catalog_error());while(!a->stop())a->sleep(250);return -1;}
 satellite_display_begin(a);
 uint32_t prev=0,planned_at=0;double oldlat=1000,oldlon=1000;int planning=1,active=-1,session=0;const char*bridge="";
 while(!a->stop()){
  RadioClock c;GpsPosition pos;
  if(a->clock(&c)||c.year<2025||c.year>2099||c.month<1||c.month>12||c.day<1||c.day>31||c.hour>23||c.minute>59||c.second>59){message("Set radio clock to UTC");a->sleep(250);continue;}
  uint32_t now=utc_seconds(c.year,c.month,c.day,c.hour,c.minute,c.second);
  if(gps_read(&pos)!=GPS_OK){if(session)sat_control_tick(a,0,0,0,0,0);message("Waiting for GPS");a->sleep(500);continue;}
  int reset=!prev||now<prev||now-prev>5||fabs(pos.lat-oldlat)>.02||fabs(pos.lon-oldlon)>.02||now-planned_at>=3600;
  if(reset){catalog_plan_reset(now);planned_at=now;planning=1;}
  else if(catalog_plan_refresh(now))planning=1;prev=now;oldlat=pos.lat;oldlon=pos.lon;
  if(planning)planning=!catalog_plan_step(now,pos.lat,pos.lon,&scan_orbit);
  int desired=planning&&active<0?-1:catalog_choose(now);RadioState radio={0};int have=a->radio_state(&radio)==0;
  int switch_wait=0;
  if(desired>=0&&desired!=active&&!strstr(bridge,"STOP:")){
   if(!have||radio.tx)switch_wait=1;
   else{
    Satellite*p=&satellites[desired];
    if(sat_control_profile(a,p->rx,p->tx,p->tone)){message("Switch: wait for radio RX");a->sleep(100);continue;}
    if(catalog_orbit(p,&orbit)){p->valid=0;active=-1;continue;}
    active=desired;
   }
  }
  if(active<0){message(planning?"Finding satellite passes":switch_wait?"Release PTT to select satellite":"No pass in next 24 hours");a->sleep(planning?10:250);continue;}
  Satellite*p=&satellites[active];Look q;
  if(fabs((double)now-p->epoch)>7*86400||orbit_look_profile(&orbit,now,pos.lat,pos.lon,0,p->rx,p->tx,&q)){
   if(session)sat_control_tick(a,0,0,1,0,0);message("Orbit expired / restart SAT");a->sleep(500);continue;
  }
  session=1;int captured=radio_session_capture(a);
  bridge=captured==1?sat_control_tick(a,q.down_hz,q.up_hz,1,1,0):radio_session_status();
  snprintf(lines[0],32,"%s / FM REPEATER %s",p->name,q.el>=0?"IN RANGE":"BELOW");
  snprintf(lines[1],32,"%02u:%02u:%02u UTC  GPS FIX",c.hour,c.minute,c.second);
  snprintf(lines[2],32,"AZ %03d EL %+03d %5ldkm",(int)(q.az+.5),(int)q.el,(long)(q.range+.5));
  char f[16];freq(f,sat_control_rx()?sat_control_rx():(uint32_t)q.down_hz);snprintf(lines[3],32,"RX A %s",f);
  freq(f,sat_control_up()?sat_control_up():(uint32_t)q.up_hz);snprintf(lines[4],32,"TX B %s SPLIT",f);
  if(switch_wait)snprintf(lines[5],32,"%s waits for RX",satellites[desired].name);
  else if(planning&&!p->done)snprintf(lines[5],32,"Updating satellite passes");
  else pass_status_format(lines[5],32,p->name,now,p->aos,p->los,q.el>=0,1);
  if(p->tone)snprintf(lines[6],32,"CTCSS %u.%u Hz / priority %u",p->tone/10,p->tone%10,(unsigned)active+1);
  else snprintf(lines[6],32,"No CTCSS / priority %u",(unsigned)active+1);
  lines[7][0]=0;if(strstr(bridge,"STOP:")||strstr(bridge,"ERROR")||strstr(bridge,"SAVE:"))snprintf(lines[7],32,"%s",bridge);
  if(captured<0&&strstr(bridge,"CIV BUSY"))snprintf(lines[6],32,"%s",civ_busy_status());
  for(unsigned i=0;i<8;i++)show(i,lines[i]);
  if(captured<0)satellite_display_pages("SAVE FAILED","CHECK SAT APP");
  else if(!captured)satellite_display_text(strstr(bridge,"WAIT CI-V")?"CI-V WAIT":"SAVE RADIO");
  else if(lines[7][0])satellite_display_pages("TUNING STOP","CHECK SAT APP");
  else if(switch_wait){char label[14];snprintf(label,14,"%s WAIT",satellites[desired].name);satellite_display_pages(label,"RELEASE PTT");}
  else if(planning&&!p->done)satellite_display_text("FIND PASSES");
  else{char pages[2][14];unsigned n=pass_status_pages(pages,p->name,now,p->aos,p->los,q.el>=0,1);satellite_display_pages(pages[0],n>1?pages[1]:0);}
  a->sleep(100);
 }
 a->clear();show(0,"SAT / STOP");show(2,"1 Drain radio commands");
 if(session)sat_control_finish(a);
 show(2,"2 Release status text");satellite_display_end();
 if(!session)return 0;
 show(2,"3 Restore radio settings");uint32_t start=a->ticks();int restored;
 while((restored=radio_session_restore(a))==0&&a->ticks()-start<30000){show(3,radio_session_status());a->sleep(50);}
 show(2,restored==1?"STOP complete":"STOP: restore incomplete");show(3,restored==1?"Original settings restored":radio_session_status());
 return 0;
}
