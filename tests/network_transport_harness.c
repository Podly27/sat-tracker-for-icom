#include "tle_network.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
extern int tle_network_fetch_bound(Api*,TleHttp*,const char**);
static const char*scenario;static unsigned connects,closes,clock_ms,position,sends,receives;static char wire[1024];
static int stop(void){return !strcmp(scenario,"cancel")&&receives>0;}
static int ready(void){return !strcmp(scenario,"offline")?-9:0;}
static unsigned ticks(void){return clock_ms;}
static void sleep_ms(unsigned n){clock_ms+=n;}
static void line(unsigned i,const char*s){assert(i<8&&strlen(s)<=31);}
static int connect_(const char*host,unsigned port,unsigned flags,unsigned timeout){
 assert((!strcmp(host,"celestrak.org")||!strcmp(host,"www.amsat.org"))&&port==443&&flags==1&&timeout==5000);connects++;position=0;
 return !strcmp(scenario,"connect-fail")?-10:0;
}
static int send_(int fd,const void*b,unsigned n,unsigned timeout){
 assert(fd==0&&b&&timeout==500);sends++;
 if(!strcmp(scenario,"send-fail"))return -10;
 return n>13?13:(int)n;
}
static int recv_(int fd,void*b,unsigned n,unsigned timeout){
 assert(fd==0&&timeout==500);receives++;
 if(!strcmp(scenario,"read-fail")||(!strcmp(scenario,"fallback")&&connects==1))return -10;
 if(!strcmp(scenario,"timeout")){clock_ms+=21000;*(char*)b='x';return 1;}
 unsigned count=(unsigned)strlen(wire)-position;if(count>n)count=n;if(count>17)count=17;
 memcpy(b,wire+position,count);position+=count;return count;
}
static int close_(int fd){assert(fd==0);closes++;return 0;}
int main(int argc,char**argv){
 assert(argc==3);scenario=argv[1];FILE*f=fopen(argv[2],"rb");assert(f);char tle[512];unsigned size=(unsigned)fread(tle,1,sizeof(tle)-1,f);fclose(f);tle[size]=0;
 snprintf(wire,sizeof(wire),"HTTP/1.1 200 OK\r\nContent-Length: %u\r\n\r\n%s",size,tle);
 Api api={.abi=3,.radio=0x705,.build=0x07051405,.size=0x90,.line=line,.ticks=ticks,.sleep=sleep_ms,.stop=stop,.net_ready=ready,.net_connect=connect_,.net_send=send_,.net_recv=recv_,.net_close=close_};
 if(!strcmp(scenario,"profiles")){
  api.net_ready=(void*)0x2037b551;api.net_connect=(void*)0x2037c455;api.net_send=(void*)0x2037bb29;api.net_recv=(void*)0x2037bc3d;api.net_close=(void*)0x2037b611;
  assert(tle_network_supported(&api));
  api.net_ready=(void*)0x2037b579;api.net_connect=(void*)0x2037c601;api.net_send=(void*)0x2037bc21;api.net_recv=(void*)0x2037bd35;api.net_close=(void*)0x2037b639;
  assert(tle_network_supported(&api));api.size=0x70;assert(!tle_network_supported(&api));api.size=0x90;api.net_recv=(void*)0x2037bc3d;assert(!tle_network_supported(&api));return 0;
 }
 TleHttp response;const char*source="";int r;
 if(!strcmp(scenario,"unsupported")){r=tle_network_fetch(&api,&response,&source);assert(r<0&&!connects);return 0;}
 r=tle_network_fetch_bound(&api,&response,&source);
 if(!strcmp(scenario,"success")||!strcmp(scenario,"fallback")){assert(r==0);assert(response.have_pair);assert(sends);assert(connects==(!strcmp(scenario,"fallback")?2:1));}
 else assert(r<0);
 assert(closes==(!strcmp(scenario,"connect-fail")?0:connects));return 0;
}
