#include "tle_store.h"
#include "tle_network.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
static TleHttp download;
static const char *files[]={"ISS.TLE","ISS.BAK","SATELITE.TLE"};
static int read_text(Api*a,const char*name,char*buf,unsigned size){
 int fd=a->open(name,0);if(fd<0)return -1;unsigned used=0;int n=0;
 while(used<size-1){
  n=a->read(fd,buf+used,(int)(size-1-used));if(n<=0)break;
  if((unsigned)n>size-1-used){n=-1;break;}used+=(unsigned)n;
 }
 int closed=a->close(fd);if(n<0||closed<0||used==size-1)return -1;
 buf[used]=0;return (int)used;
}
static int parse(Orbit*out,char*text){
 char*one=text;while(*one&&!(one[0]=='1'&&one[1]==' ')){one=strchr(one,'\n');if(!one)return -1;one++;}
 char*two=strchr(one,'\n');if(!two)return -1;*two++=0;char*end=strchr(two,'\n');if(end)*end=0;
 unsigned n=strlen(one);if(n&&one[n-1]=='\r')one[n-1]=0;n=strlen(two);if(n&&two[n-1]=='\r')two[n-1]=0;
 return orbit_init(out,one,two);
}
int tle_store_load(Api*a,uint32_t now,Orbit*out,unsigned*slot){
 char text[512];Orbit candidate;double best=8*86400;int valid=0;*slot=3;
 for(unsigned i=0;i<3&&!a->stop();i++){
  if(read_text(a,files[i],text,sizeof(text))<0||parse(&candidate,text))continue;
  double age=fabs((double)now-candidate.epoch);
  if(age<=7*86400&&age<best){*out=candidate;*slot=i;best=age;valid=1;}
 }
 return valid?0:-1;
}
int tle_store_save(Api*a,unsigned protected_slot,const char*one,const char*two){
 /* The current valid slot (or legacy SATELITE.TLE) is never opened for writing.
  * A torn/failed write affects only the alternate slot; next boot validates both. */
 unsigned dest=protected_slot==0?1:0;char text[160],check[160];
 int length=snprintf(text,sizeof(text),"ISS (ZARYA)\n%s\n%s\n",one,two);
 if(length<=0||length>=(int)sizeof(text)||!a->write||a->stop())return -1;
 int fd=a->open(files[dest],1);if(fd<0)return -1;int used=0,failed=0;
 while(used<length){
  if(a->stop()){failed=1;break;}
  int n=a->write(fd,text+used,length-used);
  if(n<=0||n>length-used){failed=1;break;}used+=n;
 }
 if(a->close(fd)<0)failed=1;
 if(failed||a->stop())return -1;
 int n=read_text(a,files[dest],check,sizeof(check));
 return n==length&&!memcmp(check,text,(unsigned)length)?0:-1;
}
static int radio_now(Api*a,uint32_t*now){
 RadioClock c;if(a->clock(&c)<0||c.year<2025||c.year>2099||c.month<1||c.month>12||c.day<1||c.day>31||c.hour>23||c.minute>59||c.second>59)return -1;
 *now=utc_seconds(c.year,c.month,c.day,c.hour,c.minute,c.second);return 0;
}
int tle_startup(Api*a,Orbit*out){
 a->clear();a->line(0,"ISS tracker / STARTUP 0.18");a->line(1,"Checking TLE epoch / UTC");
 uint32_t now=0;while(!a->stop()&&radio_now(a,&now)){a->line(3,"Set radio clock to UTC");a->sleep(250);}
 if(a->stop())return -2;
 unsigned slot;int valid=tle_store_load(a,now,out,&slot)==0;
 int old=!valid||fabs((double)now-out->epoch)>86400;
 char line[32];
 if(old){
  a->line(2,valid?"TLE older than 24h":"No usable cached TLE");
  if(tle_network_supported(a)&&a->net_ready()==0){
   const char*source="";int r=tle_network_fetch(a,&download,&source);
   if(r==-2||a->stop())return -2;
   if(!r){
    Orbit candidate;
    if(!radio_now(a,&now)&&!orbit_init(&candidate,download.first,download.second)&&fabs((double)now-candidate.epoch)<=86400){
     int saved=tle_store_save(a,slot,download.first,download.second);
     if(a->stop())return -2;
     *out=candidate;valid=1;a->line(4,saved?"TLE downloaded / SD save failed":"TLE downloaded / SD verified");
    }else a->line(4,"Downloaded orbit/epoch rejected");
   }else a->line(4,tle_network_status());
  }else a->line(4,"Offline / using cached TLE");
 }else a->line(4,"Cached TLE is current (<24h)");
 if(a->stop())return -2;
 if(!valid){a->line(5,"No valid TLE / no radio changes");return -1;}
 if(radio_now(a,&now)||fabs((double)now-out->epoch)>7*86400){a->line(5,"TLE expired / check UTC");return -1;}
 snprintf(line,sizeof(line),"Epoch offset: %+.1fh",((double)now-out->epoch)/3600);a->line(5,line);
 a->line(6,"Next: GPS and AOS/LOS search");a->sleep(1500);
 return a->stop()?-2:0;
}
