#define ISS_HOST_TEST 1
#include "../src/frq_text_probe.c"
#include <assert.h>
static int sleeps,missing,fix=1,expired,painted,redrawn,visible=1,tx,mode=0xa0,supported=1;
static void clear_(void){}
static void line_(unsigned n,const char*s){assert(n<8&&strlen(s)<=31);}
static void sleep_(unsigned n){(void)n;sleeps++;}
static int stop_(void){return sleeps>=5;}
static int clock_(RadioClock*c){*c=(RadioClock){2026,11,21,8,0,0,6};return 0;}
static int state_(void*p){*(RadioState*)p=(RadioState){437800000,145990000,(unsigned char)mode,(unsigned char)tx,0};return 0;}
static int open_(const char*n,unsigned flags){assert(!flags);assert(!strcmp(n,"ISS.TLE")||!strcmp(n,"SATELITE.TLE"));return missing?-1:1;}
static int read_(int fd,void*b,int count){(void)fd;(void)count;strcpy(b,"1 mock\n2 mock\n");return 14;}
static int close_(int fd){(void)fd;return 0;}
int probe_ui_supported(Api*a){(void)a;return supported;}
unsigned probe_ui_code(void){return visible?0:0x33;}
void probe_paint(unsigned y,const char*s){assert(visible&&!tx&&mode==0xa0&&fix&&!expired);assert(y==96);assert(!strcmp(s,"ISS AZ105 EL+13 IN RANGE"));painted++;}
void probe_redraw(void){redrawn++;}
int aprs_status_supported(Api*a){(void)a;return supported;}
int aprs_status_publish(const char*s){assert(!tx&&mode==0xa0&&fix&&!expired);assert(!strcmp(s,"ISS A105 E+13"));painted++;return 0;}
int aprs_status_end(void){return 0;}
uint32_t utc_seconds(int y,int m,int d,int h,int min,int s){(void)y;(void)m;(void)d;(void)h;(void)min;(void)s;return 1000000;}
int orbit_init(Orbit*o,const char*a,const char*b){(void)a;(void)b;o->epoch=1000000-(expired?8*86400:0);return 0;}
int orbit_look(Orbit*o,uint32_t t,double lat,double lon,double alt,Look*q){(void)o;(void)t;(void)lat;(void)lon;(void)alt;*q=(Look){105,13,1306,0,437790246,145993253};return 0;}
int gps_read(GpsPosition*p){*p=(GpsPosition){0,0,fix};return fix?GPS_OK:GPS_NO_FIX;}
int main(int argc,char**argv){assert(argc==2);missing=!strcmp(argv[1],"missing");fix=strcmp(argv[1],"no-gps")!=0;expired=!strcmp(argv[1],"expired");visible=strcmp(argv[1],"menu")!=0;tx=!strcmp(argv[1],"tx");if(!strcmp(argv[1],"ssb"))mode=0x10;if(!strcmp(argv[1],"unsupported"))supported=0;
 Api a={.abi=3,.radio=0x705,.build=0x07051405,.size=0x90,.clear=clear_,.line=line_,.sleep=sleep_,.stop=stop_,.clock=clock_,.radio_state=state_,.open=open_,.read=read_,.close=close_};assert(app_main(&a)==0);
 if(missing||!fix||expired||tx||mode!=0xa0||!supported){assert(!painted);assert(!redrawn);}else{assert(painted>0);assert(redrawn==1);}return 0;
}
