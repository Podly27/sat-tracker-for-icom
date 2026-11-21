/* Same cache publication and paced cleanup as FRQTEXT /6. No CI-V/PTT. */
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
static void stage(unsigned n){
 char label[32];snprintf(label,sizeof(label),"Display STOP stage %u",n);api->line(2,label);
 /* Keep the file close and 750 ms pacing which worked in FRQTEXT /6.
  * Last closed stage is diagnostic evidence, not a completed-unload proof. */
 if(api->write&&api->open&&api->close){char text[48];int len=snprintf(text,sizeof(text),"Stage %u\n",n);int fd=api->open("satellites.LOG",1);
  if(fd>=0){int at=0;while(at<len){int k=api->write(fd,text+at,len-at);if(k<=0||k>len-at)break;at+=k;}api->close(fd);}}
 api->sleep(750);
}
void satellite_display_end(void){
 if(!used)return;stage(1);aprs_status_end_trace(stage);stage(5);
#ifndef ISS_HOST_TEST
 *(volatile uint16_t*)0x20380698|=1;*(volatile uint8_t*)0x203806f7|=0x40;
#endif
 stage(6);used=0;
}
