#include "tle_store.h"
#include "tle_network.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
static char files[3][512];static int exists[3],fd_slot=-1,fd_write,position,stop_flag,sleeps,fetches,writes,offline,fetch_error,download_bad,write_error,read_corrupt,stop_download,stop_save,old_download;
static const char *names[]={"SAT.TLE","SAT.BAK","unused"};
static const char *old="ISS\n1 OLD\n2 ok\n",*fresh="ISS\n1 FRESH\n2 ok\n";
static void line(unsigned n,const char*s){assert(n<8&&strlen(s)<=31);}
static void clear(void){}
static int stop(void){return stop_flag;}
static void sleep_ms(unsigned n){(void)n;sleeps++;assert(sleeps<10);}
static int clock_read(RadioClock*c){*c=(RadioClock){2026,11,21,8,0,0,6};return 0;}
static int ready(void){return offline?-9:0;}
static int open_file(const char*name,unsigned flags){
 assert(fd_slot==-1);int i;for(i=0;i<3;i++)if(!strcmp(name,names[i]))break;assert(i<3&&flags<=1);
 if(!flags&&!exists[i])return -1;fd_slot=i;fd_write=flags;position=0;
 if(flags){exists[i]=1;files[i][0]=0;assert(i!=0||!exists[1]||strcmp(files[1],fresh)==0||stop_save);}
 return i+1;
}
static int read_file(int fd,void*b,int n){assert(fd==fd_slot+1&&!fd_write);int size=(int)strlen(files[fd_slot])-position;if(size>n)size=n;if(size>11)size=11;memcpy(b,files[fd_slot]+position,(unsigned)size);if(read_corrupt&&fd_slot==1&&writes&&size)((char*)b)[0]='x';position+=size;return size;}
static int write_file(int fd,const void*b,int n){assert(fd==fd_slot+1&&fd_write);writes++;if(write_error)return -1;if(n>7)n=7;assert(position+n<512);memcpy(files[fd_slot]+position,b,(unsigned)n);position+=n;files[fd_slot][position]=0;if(stop_save)stop_flag=1;return n;}
static int close_file(int fd){assert(fd==fd_slot+1);fd_slot=-1;return 0;}
uint32_t utc_seconds(int y,int m,int d,int h,int min,int s){(void)y;(void)m;(void)d;(void)h;(void)min;(void)s;return 1000000;}
int orbit_init(Orbit*o,const char*a,const char*b){assert(b[0]=='2');if(strstr(a,"BAD"))return -1;memset(o,0,sizeof(*o));o->epoch=1000000-(strstr(a,"FRESH")?3600:strstr(a,"EXPIRED")?8*86400:strstr(a,"NEW")?(old_download?2*86400:9*3600):2*86400);return 0;}
int tle_network_supported(const Api*a){(void)a;return 1;}
const char *tle_network_status(void){return "NET: TEST FAILURE";}
int tle_network_fetch(Api*a,TleHttp*p,const char**source){(void)a;fetches++;*source="mock";if(stop_download){stop_flag=1;return -2;}if(fetch_error)return -1;strcpy(p->first,download_bad?"1 BAD":"1 NEW");strcpy(p->second,"2 ok");return 0;}
int main(int argc,char**argv){assert(argc==2);const char*s=argv[1];Orbit orbit;exists[0]=1;strcpy(files[0],old);
 if(!strcmp(s,"fresh"))strcpy(files[0],fresh);
 if(!strcmp(s,"missing"))exists[0]=0;
 if(!strcmp(s,"backup")){strcpy(files[0],"torn write");exists[1]=1;strcpy(files[1],fresh);}
 if(!strcmp(s,"closest")){exists[1]=1;strcpy(files[1],fresh);}
 offline=!strcmp(s,"offline")||!strcmp(s,"expired-offline");
 if(!strcmp(s,"expired-offline"))strcpy(files[0],"ISS\n1 EXPIRED\n2 ok\n");
 fetch_error=!strcmp(s,"fetch-fail");download_bad=!strcmp(s,"bad-download");old_download=!strcmp(s,"old-download");write_error=!strcmp(s,"write-fail");read_corrupt=!strcmp(s,"readback-fail");stop_download=!strcmp(s,"stop-download");stop_save=!strcmp(s,"stop-save");
 char before[512];strcpy(before,files[0]);
 Api api={.abi=3,.radio=0x705,.size=0x90,.line=line,.clear=clear,.sleep=sleep_ms,.stop=stop,.clock=clock_read,.open=open_file,.read=read_file,.write=write_file,.close=close_file,.net_ready=ready};int r=tle_startup(&api,&orbit);
 if(stop_download||stop_save)assert(r==-2);
 else if(!strcmp(s,"expired-offline"))assert(r==-1);
 else assert(r==0);
 assert(fd_slot==-1);
 if(strcmp(s,"missing"))assert(!strcmp(files[0],before));
 if(!strcmp(s,"fresh")||!strcmp(s,"backup")||!strcmp(s,"closest")){assert(!fetches&&!writes);assert(orbit.epoch==996400);}
 else if(offline||fetch_error||download_bad||old_download){assert(!writes);}
 else if(!stop_download){assert(writes);}
 if(r==0&&!offline&&!fetch_error&&!download_bad&&!old_download&&fetches)assert(orbit.epoch==967600);
 return 0;
}
