/* APRS display cache only. STOP must fit the v8 loader's 3-second grace. */
#include "satellite_display.h"
#include "aprs_status.h"
#include "tuner.h"
#include <string.h>
#include <stdio.h>
static Api *api;static unsigned enabled,used,page,count,scheduled;static uint32_t next;
static char pages[2][14];
void satellite_display_begin(Api*a){api=a;enabled=aprs_status_supported(a);used=page=count=scheduled=0;memset(pages,0,sizeof(pages));}
void satellite_display_pages(const char*first,const char*second){
 if(!enabled)return;
 const char*input[2]={first,second};char incoming[2][14]={{0}};unsigned total=second&&*second?2:1;
 for(unsigned p=0;p<total;p++){
  if(!input[p]||!*input[p]||strlen(input[p])>13)return;
  for(unsigned i=0;i<13;i++)incoming[p][i]=i<strlen(input[p])?input[p][i]:' ';
 }
 if(count!=total||memcmp(pages,incoming,sizeof(pages))){memcpy(pages,incoming,sizeof(pages));count=total;page=scheduled=0;}
 uint32_t now=api->ticks();if(scheduled&&(int32_t)(now-next)<0)return;
 RadioState r;
#ifdef ISS_HOST_TEST
 extern unsigned display_live_mode(void);
 unsigned mode=display_live_mode();
#else
 unsigned mode=(*(volatile const unsigned char*)0x203e212f)>>4;
#endif
 if(api->radio_state(&r)||r.tx||mode!=7)return;
 if(aprs_status_publish(pages[page])){enabled=0;return;}
 used=1;page=(page+1)%count;scheduled=1;next=now+1500;
}
void satellite_display_text(const char*s){
 static const char*long_text[]={"Waiting for GPS","Set radio clock to UTC","Switch: wait for radio RX","Finding satellite passes","Release PTT to select satellite","No pass in next 24 hours","Orbit expired / restart SAT","Updating satellite passes"};
 static const char*short_text[]={"WAIT FOR GPS","SET UTC CLOCK","WAIT FOR RX","FIND PASSES","RELEASE PTT","NO PASS 24H","ORBIT EXPIRED","FIND PASSES"};
 for(unsigned i=0;i<sizeof(long_text)/sizeof(*long_text);i++)if(!strcmp(s,long_text[i])){s=short_text[i];break;}
 if(strlen(s)>13)s="CHECK SAT APP";
 satellite_display_pages(s,0);
}
void satellite_display_end(void){
 if(!used)return;
 /* No native UI calls or forced redraw during Apps STOP. The normal radio
  * screen repaints when the loader returns to it. */
 aprs_status_release();used=0;
}
