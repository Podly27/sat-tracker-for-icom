#include "satellite_display.h"
#include "aprs_status.h"
#include "tuner.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
volatile uint8_t test_cache[202];
static uint32_t tick;static int tx,mode=7,support=1,notifications,logs,fd,at,pauses;static char logbuf[48];
void test_dirty(void){notifications++;}
unsigned display_live_mode(void){return mode;}
static uint32_t ticks(void){return tick;}
static int state(void*p){*(RadioState*)p=(RadioState){0,0,0x70,tx,0};return 0;}
static void line(unsigned n,const char*s){assert(n<8&&strlen(s)<32);}
static void sleep_(unsigned ms){assert(ms==750);pauses++;tick+=ms;}
static int open_(const char*s,unsigned flags){assert(!strcmp(s,"SAT.LOG")&&flags==1&&!fd);at=0;fd=1;return 1;}
static int write_(int f,const void*b,int n){assert(f==1&&fd&&n>0);if(n>3)n=3;assert(at+n<48);memcpy(logbuf+at,b,n);at+=n;return n;}
static int close_(int f){assert(fd&&f==1);fd=0;logs++;char expected[48];snprintf(expected,sizeof(expected),"Stage %d\n",logs);assert(at==(int)strlen(expected)&&!memcmp(logbuf,expected,at));return 0;}
int main(void){
 Api a={.ticks=ticks,.radio_state=state,.line=line,.sleep=sleep_,.open=open_,.write=write_,.close=close_};
 for(unsigned i=0;i<202;i++)test_cache[i]=(uint8_t)i;
 uint8_t saved[202];memcpy(saved,(const void*)test_cache,202);
 satellite_display_begin(&a);satellite_display_text("Waiting for GPS");
 assert(notifications==1&&!memcmp((const void*)(test_cache+0x2e),"Waiting for G",13));
 satellite_display_text("Waiting for GPS");assert(notifications==1);
 tick=1000;satellite_display_text("Waiting for GPS");assert(notifications==2&&!memcmp((const void*)(test_cache+0x2e),"aiting for GP",13));
 tx=1;tick=2000;satellite_display_text("Waiting for GPS");assert(notifications==2);
 tx=0;mode=4;tick=3000;satellite_display_text("Waiting for GPS");assert(notifications==2);
 mode=7;tick=4000;satellite_display_text("ISS pass: 7 min left");assert(notifications==3&&!memcmp((const void*)(test_cache+0x2e),"ISS pass: 7 m",13));
 satellite_display_end();assert(!logs&&!pauses&&!fd);assert(notifications==4);assert(!memcmp(saved,(const void*)test_cache,202));
 satellite_display_end();assert(!logs&&!pauses);
 satellite_display_begin(&a);satellite_display_end();assert(!logs&&!pauses);
 return 0;
}
