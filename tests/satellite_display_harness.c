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
static void text(const char*s){assert(strlen(s)==13&&!memcmp((const void*)(test_cache+0x2e),s,13));}
int main(void){
 Api a={.ticks=ticks,.radio_state=state,.line=line,.sleep=sleep_,.open=open_,.write=write_,.close=close_};
 for(unsigned i=0;i<202;i++)test_cache[i]=(uint8_t)i;
 satellite_display_begin(&a);satellite_display_text("Waiting for GPS");text("WAIT FOR GPS ");assert(notifications==1);
 tick=1000;satellite_display_text("Waiting for GPS");assert(notifications==1);
 tick=1500;satellite_display_text("Waiting for GPS");assert(notifications==2);text("WAIT FOR GPS ");
 tick=2000;satellite_display_pages("WAIT SO-131","21.11 15:20Z");assert(notifications==3);text("WAIT SO-131  ");
 tick=3499;satellite_display_pages("WAIT SO-131","21.11 15:20Z");assert(notifications==3);
 tick=3500;satellite_display_pages("WAIT SO-131","21.11 15:20Z");assert(notifications==4);text("21.11 15:20Z ");
 tx=1;tick=5000;satellite_display_pages("WAIT SO-131","21.11 15:20Z");assert(notifications==4);
 tx=0;mode=4;tick=6500;satellite_display_pages("WAIT SO-131","21.11 15:20Z");assert(notifications==4);
 mode=7;tick=7000;satellite_display_pages("WAIT SO-131","21.11 15:20Z");assert(notifications==5);text("WAIT SO-131  ");
 satellite_display_pages("SO-131 PASS","LEFT 7 MIN");assert(notifications==6);text("SO-131 PASS  ");
 tick=8499;satellite_display_pages("SO-131 PASS","LEFT 7 MIN");assert(notifications==6);
 tick=8500;satellite_display_pages("SO-131 PASS","LEFT 7 MIN");assert(notifications==7);text("LEFT 7 MIN   ");
 satellite_display_pages("THIS IS TOO LONG","NONE");assert(notifications==7);text("LEFT 7 MIN   ");
 uint8_t before[202];memcpy(before,(const void*)test_cache,202);
 satellite_display_end();assert(!logs&&!pauses&&!fd&&notifications==7);
 for(unsigned i=0;i<202;i++)assert(test_cache[i]==(i>=0x2e&&i<0x3b?' ':before[i]));
 satellite_display_end();assert(!logs&&!pauses&&notifications==7);
 satellite_display_begin(&a);satellite_display_end();assert(!logs&&!pauses&&notifications==7);
 tick=0xffffffffu-1000;satellite_display_begin(&a);satellite_display_pages("WAIT ISS","21.11 15:20Z");text("WAIT ISS     ");assert(notifications==8);
 tick=498;satellite_display_pages("WAIT ISS","21.11 15:20Z");assert(notifications==8);
 tick=499;satellite_display_pages("WAIT ISS","21.11 15:20Z");assert(notifications==9);text("21.11 15:20Z ");
 satellite_display_end();assert(!logs&&!pauses&&notifications==9);
 return 0;
}
