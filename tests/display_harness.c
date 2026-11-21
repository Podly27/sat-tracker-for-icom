#define ISS_HOST_TEST 1
#include "../src/display_probe.c"
#include <assert.h>
static int sleeps,limit=5,missing,fix=1,expired,calls,busy;static char text[256];
static void clear_(void){}
static void line_(unsigned n,const char*s){assert(n<8&&strlen(s)<=31);}
static void sleep_(unsigned n){(void)n;sleeps++;}
static int stop_(void){return sleeps>=limit;}
static int clock_(RadioClock*c){*c=(RadioClock){2026,11,21,8,0,0,6};return 0;}
static int open_(const char*n,unsigned flags){assert(!flags);assert(!strcmp(n,"ISS.TLE")||!strcmp(n,"SATELITE.TLE"));return missing?-1:1;}
static int read_(int fd,void*b,int count){(void)fd;(void)count;strcpy(b,"1 mock\n2 mock\n");return 14;}
static int close_(int fd){(void)fd;return 0;}
static int popup_(const char*s){calls++;assert(strlen(s)<sizeof(text));strcpy(text,s);return busy?-8:0;}
uint32_t utc_seconds(int y,int m,int d,int h,int min,int s){(void)y;(void)m;(void)d;(void)h;(void)min;(void)s;return 1000000;}
int orbit_init(Orbit*o,const char*a,const char*b){(void)a;(void)b;o->epoch=1000000-(expired?8*86400:0);return 0;}
int orbit_look(Orbit*o,uint32_t t,double lat,double lon,double alt,Look*q){(void)o;(void)t;(void)lat;(void)lon;(void)alt;*q=(Look){105,13,1306,0,437790246,145993253};return 0;}
int gps_read(GpsPosition*p){*p=(GpsPosition){0,0,fix};return fix?GPS_OK:GPS_NO_FIX;}
int main(int argc,char**argv){assert(argc==2);missing=!strcmp(argv[1],"missing");fix=strcmp(argv[1],"no-gps")!=0;expired=!strcmp(argv[1],"expired");busy=!strcmp(argv[1],"busy");
 Api a={.abi=3,.radio=0x705,.build=0x07051405,.size=0x90,.clear=clear_,.line=line_,.sleep=sleep_,.stop=stop_,.clock=clock_,.open=open_,.read=read_,.close=close_,.popup=popup_};
 assert(app_main(&a)==0);
 if(missing||!fix||expired)assert(!calls);
 else{assert(calls==1);assert(strstr(text,"ISS AZ 105 deg E"));assert(strstr(text,"EL +13 deg / 1306 km"));unsigned lines=1,width=0;for(const char*p=text;*p;p++){if(*p=='\n'){assert(width<=31);lines++;width=0;}else width++;}assert(lines==7&&width<=31);}
 return 0;
}
