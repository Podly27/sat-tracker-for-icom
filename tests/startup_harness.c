#define ISS_HOST_TEST 1
#include "../src/app.c"
#include <assert.h>
Satellite satellites[SAT_MAX];unsigned satellite_count=1;
static unsigned elapsed,sleeps,limit=12,commands,restores,scans,profiles;
static int missing,fix=1,expired,display_closed,controller_finished;
static char screen[8][64];
static void clear_screen(void){memset(screen,0,sizeof(screen));}
static void line(unsigned n,const char*s){assert(n<8&&strlen(s)<32);snprintf(screen[n],64,"%s",s);}
static void sleep_ms(unsigned n){elapsed+=n;sleeps++;}
static int stopped(void){return sleeps>=limit;}
static uint32_t ticks(void){return elapsed;}
static int clock_read(RadioClock*c){*c=(RadioClock){2026,11,21,12,0,0,6};return 0;}
static int state(void*p){*(RadioState*)p=(RadioState){437800000,145990000,0xa0,0,0};return 0;}
static int open_(const char*n,unsigned f){(void)n;(void)f;return -1;}static int read_(int f,void*b,int n){(void)f;(void)b;(void)n;return -1;}static int close_(int f){(void)f;return 0;}
uint32_t utc_seconds(int y,int m,int d,int h,int min,int s){(void)y;(void)m;(void)d;(void)h;(void)min;(void)s;return 1000000;}
int catalog_startup(Api*a,Orbit*o){(void)a;(void)o;if(missing)return -1;satellites[0]=(Satellite){.name="ISS",.norad=25544,.rx=437800000,.tx=145990000,.tone=670,.valid=1,.epoch=1000000-(expired?8*86400:0)};return 0;}
const char*catalog_error(void){return "No usable orbit / update SD";}
int catalog_plan_refresh(uint32_t n){(void)n;return 0;}
void catalog_plan_reset(uint32_t n){(void)n;scans=0;}
int catalog_plan_step(uint32_t n,double lat,double lon,Orbit*o){(void)lat;(void)lon;(void)o;scans++;satellites[0].aos=n+30;satellites[0].los=n+90;return scans>=3;}
int catalog_choose(uint32_t n){(void)n;assert(scans>=3);return 0;}
int catalog_orbit(const Satellite*p,Orbit*o){o->epoch=p->epoch;return 0;}
int orbit_look_profile(Orbit*o,uint32_t n,double la,double lo,double al,uint32_t d,uint32_t u,Look*q){(void)o;(void)n;(void)la;(void)lo;(void)al;*q=(Look){0,-10,1000,0,(int32_t)d,(int32_t)u};return 0;}
int gps_read(GpsPosition*p){*p=(GpsPosition){0,0,fix};return fix?GPS_OK:GPS_NO_FIX;}
void satellite_display_begin(Api*a){(void)a;}void satellite_display_text(const char*s){(void)s;}
void satellite_display_end(void){assert(!commands||controller_finished);display_closed=1;}
int sat_control_profile(Api*a,uint32_t d,uint32_t u,unsigned tone){(void)a;assert(d==437800000&&u==145990000&&tone==670);profiles++;return 0;}
int radio_session_capture(Api*a){(void)a;assert(scans>=3&&profiles);return 1;}
const char*radio_session_status(void){return "TEST SESSION";}
const char*sat_control_tick(Api*a,uint32_t d,uint32_t u,int g,int v,int stop){(void)a;(void)d;(void)u;(void)g;(void)v;(void)stop;commands++;return "SAT RX";}
uint32_t sat_control_rx(void){return 437800000;}uint32_t sat_control_up(void){return 145990000;}
void sat_control_finish(Api*a){(void)a;controller_finished=1;}
int radio_session_restore(Api*a){(void)a;assert(display_closed);restores++;return 1;}
int main(int argc,char**argv){assert(argc==2);if(!strcmp(argv[1],"cancel"))limit=1;missing=!strcmp(argv[1],"missing");fix=strcmp(argv[1],"no-fix")!=0;expired=!strcmp(argv[1],"expired");
 Api a={.abi=3,.radio=0x705,.build=0x07051405,.size=0x54,.line=line,.clear=clear_screen,.open=open_,.read=read_,.close=close_,.sleep=sleep_ms,.ticks=ticks,.radio_state=state,.stop=stopped,.clock=clock_read};app_main(&a);
 if(missing||!fix||expired||limit==1){assert(!commands&&!restores);}else{assert(commands&&restores==1&&profiles==1);}return 0;}
