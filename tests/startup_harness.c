#define ISS_HOST_TEST 1
/* Host-only ICAP state-machine test; orbit and hardware are deterministic mocks. */
#define ISS_SAT_CONTROL 1
#include "../src/app.c"
#include <assert.h>
static unsigned elapsed,sleeps,limit,commands,restores,searches;
static int missing,fix=1;static double epoch_offset;static char screen[8][64];
static void clear_screen(void){memset(screen,0,sizeof(screen));}
static void line(unsigned n,const char*s){assert(n<8);assert(strlen(s)<32);snprintf(screen[n],64,"%s",s);}
static void sleep_ms(unsigned n){elapsed+=n;sleeps++;}
static int stopped(void){return sleeps>=limit;}
static uint32_t ticks(void){return elapsed;}
static int clock_read(RadioClock*c){*c=(RadioClock){2026,11,20,12,0,0,5};return 0;}
static int open_file(const char*n,unsigned flags){assert(!strcmp(n,"ISS.TLE")&&!flags);return missing?-1:1;}
static int read_file(int h,void*b,int cap){(void)h;(void)cap;strcpy(b,"1 mock\n2 mock\n");return 14;}
static int close_file(int h){(void)h;return 0;}
static int state(void*p){(void)p;return 0;}
uint32_t utc_seconds(int y,int m,int d,int h,int min,int s){(void)y;(void)m;(void)d;(void)h;(void)min;(void)s;return 1000000;}
int orbit_init(Orbit*o,const char*a,const char*b){(void)a;(void)b;o->epoch=1000000-epoch_offset;return 0;}
int orbit_look(Orbit*o,uint32_t t,double la,double lo,double al,Look*q){(void)o;(void)la;(void)lo;(void)al;searches++;*q=(Look){0,t>=1000030&&t<1000090?10:-10,1000,0,437800000,145990000};return 0;}
int gps_read(GpsPosition*p){*p=(GpsPosition){0,0,fix};return fix?GPS_OK:GPS_NO_FIX;}
int radio_session_capture(Api*a){(void)a;assert(los==1000090);return 1;}
const char *radio_session_status(void){return "TEST SESSION";}
const char *sat_control_tick(Api*a,uint32_t d,uint32_t u,int g,int v,int stop){(void)a;(void)d;(void)u;(void)g;(void)v;(void)stop;commands++;return "SAT RX";}
uint32_t sat_control_rx(void){return 437800000;}uint32_t sat_control_up(void){return 145990000;}
void sat_control_finish(Api*a){(void)a;}
int radio_session_restore(Api*a){(void)a;restores++;return 1;}
int main(int argc,char**argv){assert(argc==2);limit=10;
 if(!strcmp(argv[1],"cancel"))limit=1;
 else if(!strcmp(argv[1],"missing"))missing=1;
 else if(!strcmp(argv[1],"no-fix"))fix=0;
 else if(!strcmp(argv[1],"expired"))epoch_offset=8*86400;
 else if(!strcmp(argv[1],"old"))epoch_offset=2*86400;
 Api a={.abi=3,.radio=0x705,.build=0x07051405,.size=0x54,.line=line,.clear=clear_screen,.open=open_file,.read=read_file,.close=close_file,.sleep=sleep_ms,.ticks=ticks,.radio_state=state,.stop=stopped,.clock=clock_read};
 app_main(&a);
 if(missing||!fix||epoch_offset>7*86400||limit==1){assert(!commands);assert(!restores);}
 else{assert(commands);assert(restores==1);assert(searches);}
 return 0;
}
