/* APRS display cache only. STOP must fit the v8 loader's 3-second grace. */
#include "satellite_display.h"
#include "aprs_status.h"
#include "tuner.h"
#include <string.h>
#include <stdio.h>
static Api *api;static unsigned enabled,used,offset;static uint32_t next;
static char message[32];
void satellite_display_begin(Api*a){api=a;enabled=aprs_status_supported(a);used=offset=next=0;message[0]=0;}
void satellite_display_text(const char*s){
 if(!enabled)return;
 uint32_t now=api->ticks();
 if(strcmp(message,s)){snprintf(message,sizeof(message),"%s",s);offset=0;next=0;}
 if(next&&(int32_t)(now-next)<0)return;next=now+1000;
 RadioState r;
#ifdef ISS_HOST_TEST
 extern unsigned display_live_mode(void);
 unsigned mode=display_live_mode();
#else
 unsigned mode=(*(volatile const unsigned char*)0x203e212f)>>4;
#endif
 if(api->radio_state(&r)||r.tx||mode!=7)return;
 unsigned n=strlen(message);if(!n)return;char text[14];
 for(unsigned i=0;i<13;i++){unsigned at=(offset+i)%(n+3);text[i]=at<n?message[at]:' ';}text[13]=0;
 if(!aprs_status_publish(text))used=1;else{enabled=0;return;}
 if(n>13)offset=(offset+1)%(n+3);
}
void satellite_display_end(void){
 if(!used)return;aprs_status_end();
#ifndef ISS_HOST_TEST
 *(volatile uint16_t*)0x20380698|=1;*(volatile uint8_t*)0x203806f7|=0x40;
#endif
 used=0;
}
