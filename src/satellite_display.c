/* APRS display cache only. STOP must fit the v8 loader's 3-second grace. */
#include "satellite_display.h"
#include "aprs_status.h"
#include "tuner.h"
#include <string.h>
#include <stdio.h>
static Api *api;static unsigned enabled,used,scheduled,bound;static uint32_t refresh;
static char current[14];
void satellite_display_begin(Api*a){api=a;bound=enabled=aprs_status_supported(a);used=scheduled=0;memset(current,0,sizeof(current));}
/* Keep the call interface for diagnostics; only the primary message is shown. */
void satellite_display_pages(const char*first,const char*second){
 (void)second;
 if(!enabled||!first||!*first||strlen(first)>13)return;
 char incoming[14]={0};unsigned length=strlen(first);
 for(unsigned i=0;i<13;i++)incoming[i]=i<length?first[i]:' ';
 if(memcmp(current,incoming,sizeof(current))){memcpy(current,incoming,sizeof(current));scheduled=0;}
 uint32_t now=api->ticks();if(scheduled&&(int32_t)(now-refresh)<0&&!aprs_status_needs_restore())return;
 RadioState r;
#ifdef ISS_HOST_TEST
 extern unsigned display_live_mode(void);
 unsigned mode=display_live_mode();
#else
 unsigned mode=(*(volatile const unsigned char*)0x203e212f)>>4;
#endif
 if(api->radio_state(&r)||r.tx||mode!=7)return;
 if(aprs_status_publish(current)){enabled=0;return;}
 used=scheduled=1;refresh=now+500;
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

const char *satellite_display_debug(unsigned row){return bound?aprs_status_debug(row):"UI diagnostics unavailable";}
