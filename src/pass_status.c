#include "pass_status.h"
#include <stdio.h>
static unsigned leap(unsigned y){return y%4==0&&(y%100!=0||y%400==0);}
static void date(uint32_t utc,unsigned *day,unsigned *month){
 unsigned days=utc/86400,y=1970;
 for(;;){unsigned n=365+leap(y);if(days<n)break;days-=n;y++;}
 static const unsigned char lengths[]={31,28,31,30,31,30,31,31,30,31,30,31};
 unsigned m=0;while(m<11){unsigned n=lengths[m]+(m==1&&leap(y));if(days<n)break;days-=n;m++;}
 *day=days+1;*month=m+1;
}
void pass_status_format(char*out,size_t size,const char*name,uint32_t now,uint32_t aos,uint32_t los,int in_range,int complete){
 if(in_range){
  if(!los)snprintf(out,size,"%s pass / finding end",name);
  else if(los<=now)snprintf(out,size,"%s pass ending",name);
  else snprintf(out,size,"%s pass: %lu min left",name,(unsigned long)((los-now)/60+((los-now)%60!=0)));
 }else if(aos>now){
  unsigned day,month;date(aos,&day,&month);
  snprintf(out,size,"Waiting for %s %02u.%02u. %02u:%02u",name,day,month,(unsigned)((aos/3600)%24),(unsigned)((aos/60)%60));
 }else if(complete)snprintf(out,size,"No %s pass in next 24 hours",name);
 else snprintf(out,size,"Finding next %s pass",name);
}

unsigned pass_status_pages(char pages[2][14],const char*name,uint32_t now,uint32_t aos,uint32_t los,int in_range,int complete){
 pages[0][0]=pages[1][0]=0;
 if(in_range){
  snprintf(pages[0],14,"%s PASS",name);
  if(!los)snprintf(pages[1],14,"FINDING LOS");
  else if(los<=now)snprintf(pages[1],14,"PASS ENDING");
  else snprintf(pages[1],14,"LEFT %lu MIN",(unsigned long)((los-now)/60+((los-now)%60!=0)));
  return 2;
 }
 if(aos>now){unsigned day,month;date(aos,&day,&month);snprintf(pages[0],14,"WAIT %s",name);
  snprintf(pages[1],14,"%02u.%02u %02u:%02uZ",day,month,(unsigned)((aos/3600)%24),(unsigned)((aos/60)%60));return 2;}
 snprintf(pages[0],14,complete?"NO PASS 24H":"FIND PASSES");return 1;
}
