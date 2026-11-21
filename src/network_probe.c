#include "tle_network.h"
#include "orbit.h"
#include <stdio.h>
#include <math.h>
static TleHttp response;static Orbit candidate;
__attribute__((section(".entry"))) int app_main(Api*a){
 if(!a||a->abi!=3||a->radio!=0x705||a->size<0x54||!a->clear||!a->line||!a->stop||!a->sleep||!a->ticks||!a->clock)return -1;
 a->clear();a->line(0,
#ifdef TLE_FORCE_IP
 "TLE HTTPS IP TEST / 2"
#else
 "TLE HTTPS DNS TEST / 2"
#endif
 );
 a->line(1,"No tuning / no PTT / no SD write");
 char tcp_line[32]="TCP: UNSUPPORTED API";
 if(tle_network_supported(a)){
#ifdef TLE_FORCE_IP
  const char *host="104.168.149.178";tle_network_numeric(1);
#else
  const char *host="celestrak.org";tle_network_numeric(0);
#endif
  a->line(3,host);a->line(4,"Testing TCP443 without TLS");
  if(a->net_ready()==0&&!a->stop()){
   int fd=a->net_connect(host,443,0,5000);int error=fd<0?a->net_error():0;
   snprintf(tcp_line,sizeof(tcp_line),"TCP443 C:%d E:%d",fd,error);
   if(fd>=0)a->net_close(fd);
  }else snprintf(tcp_line,sizeof(tcp_line),"TCP: OFFLINE OR STOPPED");
 }
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
 const TleConnectDiag *d=tle_network_diagnostics();
 a->line(1,tcp_line);
 snprintf(text,sizeof(text),"CL TLS C:%d E:%d %lums",d[0].result,d[0].error,(unsigned long)d[0].elapsed);a->line(2,d[0].result==-999?"CL TLS: NOT ATTEMPTED":text);
 snprintf(text,sizeof(text),"AM TLS C:%d E:%d %lums",d[1].result,d[1].error,(unsigned long)d[1].elapsed);a->line(3,d[1].result==-999?"AM TLS: NOT ATTEMPTED":text);
 a->line(7,"Use STOP to exit this check");
 while(!a->stop())a->sleep(100);
 return r;
}
