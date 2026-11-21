#include "tle_network.h"
#include "orbit.h"
#include <stdio.h>
#include <math.h>
static TleHttp response;static Orbit candidate;
__attribute__((section(".entry"))) int app_main(Api*a){
 if(!a||a->abi!=3||a->radio!=0x705||a->size<0x54||!a->clear||!a->line||!a->stop||!a->sleep||!a->ticks||!a->clock)return -1;
 a->clear();a->line(0,"TLE HTTPS CHECK / EXPERIMENTAL");
 a->line(1,"No tuning / no PTT / no SD write");
 const char *source="None";int r=tle_network_fetch(a,&response,&source);char text[32];
 if(!r){
  RadioClock c;int clock=a->clock(&c);
  if(orbit_init(&candidate,response.first,response.second))a->line(5,"FAIL: INVALID ISS ORBIT");
  else if(clock<0||c.year<2025||c.year>2099||c.month<1||c.month>12||c.day<1||c.day>31)a->line(5,"FAIL: SET CLOCK TO UTC");
  else{
   double age=((double)utc_seconds(c.year,c.month,c.day,c.hour,c.minute,c.second)-candidate.epoch)/3600;
   snprintf(text,sizeof(text),"TLE epoch offset %+.1f hours",age);a->line(5,text);
   a->line(6,fabs(age)<=24?"PASS: ISS TLE <=24h":"FAIL: TLE OUTSIDE 24 HOURS");
  }
 }else a->line(5,tle_network_status());
 a->line(7,"Use STOP to exit this check");
 while(!a->stop())a->sleep(100);
 return r;
}
