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
 tick=499;satellite_display_text("Waiting for GPS");assert(notifications==1);
 tick=500;satellite_display_text("Waiting for GPS");assert(notifications==2);text("WAIT FOR GPS ");
 tick=1000;satellite_display_pages("WAIT SO-131","21.11 15:20Z");assert(notifications==3);text("WAIT SO-131  ");
 for(unsigned i=1;i<=8;i++){tick=1000+i*500;satellite_display_pages("WAIT SO-131","21.11 15:20Z");assert(notifications==3+i);text("WAIT SO-131  ");}
 tx=1;tick=5500;satellite_display_text("WAIT SO-131");assert(notifications==11);
 tx=0;mode=4;tick=6000;satellite_display_text("WAIT SO-131");assert(notifications==11);
 mode=7;tick=6500;satellite_display_text("WAIT SO-131");assert(notifications==12);text("WAIT SO-131  ");
 satellite_display_pages("SO-131 PASS","LEFT 7 MIN");assert(notifications==13);text("SO-131 PASS  ");
 tick=7000;satellite_display_pages("SO-131 PASS","LEFT 7 MIN");assert(notifications==14);text("SO-131 PASS  ");
 satellite_display_text("THIS IS TOO LONG");assert(notifications==15);text("CHECK SAT APP");
 satellite_display_pages("THIS IS TOO LONG",0);assert(notifications==15);
 uint8_t before[202];memcpy(before,(const void*)test_cache,202);
 satellite_display_end();assert(!logs&&!pauses&&!fd&&notifications==15);
 for(unsigned i=0;i<202;i++)assert(test_cache[i]==(i>=0x2e&&i<0x3b?' ':before[i]));
 satellite_display_end();assert(!logs&&!pauses&&notifications==15);
 tick=0xffffffffu-250;satellite_display_begin(&a);satellite_display_text("WAIT ISS");assert(notifications==16);
 tick=248;satellite_display_text("WAIT ISS");assert(notifications==16);
 tick=249;satellite_display_text("WAIT ISS");assert(notifications==17);text("WAIT ISS     ");
 satellite_display_end();assert(!logs&&!pauses&&notifications==17);
 return 0;
}
