#include "catalog.h"
#include "profile.h"
#include "tle_network.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
Satellite satellites[SAT_MAX];unsigned satellite_count;
static const char *error="";
const char *catalog_error(void){return error;}
/* Strict finite decimal, including the scientific notation used by OMM BSTAR. */
static int number(const char*s,double*out){
 int sign=1,es=1,e=0,digits=0;double v=0,scale=1;
 if(*s=='-'||*s=='+'){if(*s=='-')sign=-1;s++;}
 while(*s>='0'&&*s<='9'){v=v*10+*s++-'0';digits++;}
 if(*s=='.'){s++;while(*s>='0'&&*s<='9'){scale*=.1;v+=(*s++-'0')*scale;digits++;}}
 if(!digits)return -1;
 if(*s=='e'||*s=='E'){s++;if(*s=='-'||*s=='+'){if(*s=='-')es=-1;s++;}if(*s<'0'||*s>'9')return -1;while(*s>='0'&&*s<='9'){e=e*10+*s++-'0';if(e>30)return -1;}while(e--)v*=es<0?.1:10;}
 if(*s||!isfinite(v))return -1;*out=v*sign;return 0;
}
/* CSV names may be quoted; fields themselves are decoded without truncation. */
static int split(char *s,char**fields,unsigned cap){
 unsigned n=0;char *r=s,*w=s;
 for(;;){if(n==cap)return -1;fields[n++]=w;int quoted=*r=='"';if(quoted)r++;
  for(;;){if(!*r){if(quoted)return -1;*w=0;return n;}if(quoted&&*r=='"'){r++;if(*r=='"'){*w++=*r++;continue;}quoted=0;if(*r&&*r!=',')return -1;}
   if(!quoted&&*r==','){r++;*w++=0;break;}if(!*r){*w=0;return n;}*w++=*r++;}
 }
}
static int catalog_id(uint32_t n,char*out){
 if(!n||n>339999)return -1;
 static const char letters[]="0123456789ABCDEFGHJKLMNPQRSTUVWXYZ";
 if(n<100000)snprintf(out,6,"%05lu",(unsigned long)n);
 else{out[0]=letters[n/10000];snprintf(out+1,5,"%04lu",(unsigned long)(n%10000));}return 0;
}
int catalog_config_line(const char*raw){
 char line[160];size_t len=strlen(raw);if(len>=sizeof(line))return -1;memcpy(line,raw,len+1);
 char *s=line;while(*s==' '||*s=='\t')s++;if(!*s||*s=='#'||*s==';')return 0;
 char*comment=strchr(s,'#');if(comment)*comment=0;
 char *f[5];if(split(s,f,5)!=5||satellite_count==SAT_MAX)return -1;
 for(unsigned i=0;i<5;i++){while(*f[i]==' '||*f[i]=='\t')f[i]++;char*end=f[i]+strlen(f[i]);while(end>f[i]&&(end[-1]==' '||end[-1]=='\t'))*--end=0;}
 len=strlen(f[0]);if(!len||len>6)return -1;for(unsigned i=0;i<len;i++)if(!((f[0][i]>='A'&&f[0][i]<='Z')||(f[0][i]>='0'&&f[0][i]<='9')||f[0][i]=='-'))return -1;
 double id,rx,tx,tone;if(number(f[1],&id)||number(f[2],&rx)||number(f[3],&tx)||number(f[4],&tone)||id<1||id>339999||floor(id)!=id||rx<144||rx>440||tx<144||tx>440||tone<0||tone>254.1)return -1;
 Satellite p={0};p.norad=(uint32_t)id;p.rx=(uint32_t)(rx*1e6+.5);p.tx=(uint32_t)(tx*1e6+.5);p.tone=(unsigned)(tone*10+.5);
 if(fabs(tone*10-p.tone)>.001||!profile_valid(p.rx,p.tx,p.tone)||catalog_id(p.norad,p.id))return -1;
 for(unsigned i=0;i<satellite_count;i++)if(satellites[i].norad==p.norad||!strcmp(satellites[i].name,f[0]))return -1;
 memcpy(p.name,f[0],len+1);satellites[satellite_count++]=p;return 0;
}
/* All input is line bounded; overlong or partial lines are rejected. */
static int rows(Api*a,const char*name,int (*row)(const char*,void*),void*ctx){
 int fd=a->open(name,0);if(fd<0)return -1;char line[512],block[128];unsigned used=0;int result=0,n=0;
 while(!a->stop()&&(n=a->read(fd,block,sizeof(block)))>0){if(n>(int)sizeof(block)){result=-1;break;}
  for(int i=0;i<n;i++){char c=block[i];if(c=='\n'){if(used&&line[used-1]=='\r')used--;line[used]=0;if(row(line,ctx)){result=-1;break;}used=0;}else if(c==0||(unsigned char)c>126||((unsigned char)c<32&&c!='\r'&&c!='\t')||used==sizeof(line)-1){result=-1;break;}else line[used++]=c;}if(result)break;
 }
 if(a->stop()||n<0)result=-1;if(!result&&used){if(line[used-1]=='\r')used--;line[used]=0;result=row(line,ctx);}if(a->close(fd)<0)result=-1;return result;
}
static int config_row(const char*s,void*ctx){(void)ctx;return catalog_config_line(s);}
int catalog_config(Api*a){
 satellite_count=0;memset(satellites,0,sizeof(satellites));
 /* A missing configuration uses the original ISS profile; an existing invalid
  * or empty configuration fails before any settings are captured. */
 int fd=a->open("SAT.CFG",0);if(fd<0)return catalog_config_line("ISS,25544,437.800,145.990,67.0");a->close(fd);
 if(rows(a,"SAT.CFG",config_row,0)||!satellite_count){error="Invalid SAT.CFG / no tuning";return -1;}return 0;
}
static int csv_orbit(Orbit*o,const char*raw,uint32_t expected){
 char line[SAT_RECORD];if(strlen(raw)>=sizeof(line))return -1;strcpy(line,raw);char*f[17];if(split(line,f,17)!=17)return -1;
 double id,v[7];if(number(f[11],&id)||id!=expected)return -1;
 const unsigned indices[]={3,4,5,6,7,8,14};for(unsigned i=0;i<7;i++)if(number(f[indices[i]],&v[i]))return -1;
 if(v[0]<1440.0/110||v[0]>1440.0/80||v[1]<0||v[1]>=1||v[2]<0||v[2]>180||v[3]<0||v[3]>=360||v[4]<0||v[4]>=360||v[5]<0||v[5]>=360||fabs(v[6])>1)return -1;
 char*t=f[2];if(strlen(t)<19||strlen(t)>27||t[4]!='-'||t[7]!='-'||t[10]!='T'||t[13]!=':'||t[16]!=':')return -1;
 const unsigned positions[]={0,5,8,11,14};const unsigned widths[]={4,2,2,2,2};unsigned date[5];
 for(unsigned i=0;i<5;i++){date[i]=0;for(unsigned j=0;j<widths[i];j++){char c=t[positions[i]+j];if(c<'0'||c>'9')return -1;date[i]=date[i]*10+c-'0';}}
 double sec;if(number(t+17,&sec)||sec<0||sec>=60||date[0]<2025||date[0]>2099||date[1]<1||date[1]>12||date[2]<1||date[2]>31||date[3]>23||date[4]>59)return -1;
 static const unsigned char days[]={31,28,31,30,31,30,31,31,30,31,30,31};
 unsigned max=days[date[1]-1]+(date[1]==2&&date[0]%4==0);if(date[2]>max)return -1;
 memset(o,0,sizeof(*o));o->epoch=utc_seconds(date[0],date[1],date[2],date[3],date[4],0)+sec;
 ElsetRec*r=&o->rec;r->whichconst=wgs72;r->jdsatepoch=2440587.5+floor(o->epoch/86400);r->jdsatepochF=fmod(o->epoch,86400)/86400;
 r->no_kozai=v[0]*twopi/1440;r->ecco=v[1];r->inclo=v[2]*deg2rad;r->nodeo=v[3]*deg2rad;r->argpo=v[4]*deg2rad;r->mo=v[5]*deg2rad;r->bstar=v[6];return !sgp4init('i',r)||r->error?-1:0;
}
int catalog_orbit(const Satellite*p,Orbit*o){return !strncmp(p->first,"CSV,",4)?csv_orbit(o,p->first+4,p->norad):orbit_init_id(o,p->first,p->second,p->id);}
typedef struct {Api*a;Orbit*orbit;uint32_t now;unsigned load,count,verify;double score;char one[70];unsigned seen[SAT_MAX];} Cache;
static int cache_row(const char*line,void*context){
 Cache*c=context;
 if(!*line||*line=='#')return 0;
 if(line[0]=='1'&&line[1]==' '){if(strlen(line)!=69)return -1;memcpy(c->one,line,70);return 0;}
 for(unsigned i=0;i<satellite_count;i++){
  Satellite*p=&satellites[i];int csv=!strncmp(line,"CSV,",4);
  if(!csv&&!(line[0]=='2'&&line[1]==' '))return 0;
  int ok=csv?csv_orbit(c->orbit,line+4,p->norad):
    strlen(line)==69&&c->one[0]&&!memcmp(c->one+2,p->id,5)?orbit_init_id(c->orbit,c->one,line,p->id):-1;
  if(ok)continue;
  double age=fabs((double)c->now-c->orbit->epoch);if(age>7*86400)continue;
  if(c->verify&&(csv?strcmp(p->first,line):strcmp(p->first,c->one)||strcmp(p->second,line)))return -1;
  if(c->seen[i]++)return -1;c->count++;c->score+=age;
  if(c->load){p->epoch=c->orbit->epoch;p->valid=1;if(csv){if(strlen(line)>=sizeof(p->first))return -1;strcpy(p->first,line);p->second[0]=0;}else{memcpy(p->first,c->one,70);memcpy(p->second,line,70);}}
  break;
 }
 if(line[0]=='2')c->one[0]=0;return 0;
}
static int now(Api*a,uint32_t*out){RadioClock c;if(a->clock(&c)||c.year<2025||c.year>2099||c.month<1||c.month>12||c.day<1||c.day>31||c.hour>23||c.minute>59||c.second>59)return -1;*out=utc_seconds(c.year,c.month,c.day,c.hour,c.minute,c.second);return 0;}
static int csv_download(const char*line,void*context){
 Satellite*p=context;if(!strncmp(line,"OBJECT_NAME,OBJECT_ID,EPOCH,",28))return 0;
 if(strlen(line)+4>=sizeof(p->first))return -1;
 /* Keep the original source record; validation happens before it is committed. */
 snprintf(p->first,sizeof(p->first),"CSV,%s",line);p->second[0]=0;return 1;
}
static int put(Api*a,int fd,const char*s){unsigned left=strlen(s);while(left&&!a->stop()){int n=a->write(fd,s,left);if(n<=0||(unsigned)n>left)return -1;s+=n;left-=n;}return left?-1:0;}
static int save(Api*a,unsigned slot,uint32_t utc,Orbit*scratch){
 const char*dest=slot==0?"SAT.BAK":"SAT.TLE";int fd=a->open(dest,1);if(fd<0)return -1;int fail=0;unsigned expected=0;
 for(unsigned i=0;i<satellite_count&&!fail;i++){Satellite*p=&satellites[i];if(!p->valid)continue;expected++;
  fail=put(a,fd,p->name)||put(a,fd,"\n")||put(a,fd,p->first)||put(a,fd,"\n")||(*p->second&&(put(a,fd,p->second)||put(a,fd,"\n")));
 }
 if(a->close(fd)<0)fail=1;if(fail||a->stop())return -1;
 Cache check={.a=a,.orbit=scratch,.now=utc,.verify=1};if(rows(a,dest,cache_row,&check)||check.count!=expected)return -1;return 0;
}
int catalog_startup(Api*a,Orbit*scratch){
 a->clear();a->line(0,"SAT / STARTUP 0.32");
 if(catalog_config(a))return -1;
 uint32_t utc;while(!a->stop()&&now(a,&utc)){a->line(2,"Set radio clock to UTC");a->sleep(250);}if(a->stop())return -2;
 const char*files[]={"SAT.TLE","SAT.BAK"};unsigned slot=2,best=0;double score=1e12;
 for(unsigned i=0;i<2;i++){Cache c={.a=a,.orbit=scratch,.now=utc};if(!rows(a,files[i],cache_row,&c)&&(c.count>best||(c.count&&c.count==best&&c.score<score))){slot=i;best=c.count;score=c.score;}}
 if(slot<2){Cache c={.a=a,.orbit=scratch,.now=utc,.load=1};if(rows(a,files[slot],cache_row,&c)){error="TLE cache read failed";return -1;}}
 unsigned changed=0,valid=0;
 for(unsigned i=0;i<satellite_count&&!a->stop();i++){
  Satellite*p=&satellites[i];a->line(1,p->name);
  if(!p->valid||fabs((double)utc-p->epoch)>86400){
   a->line(2,"Refreshing orbital data");
   if(tle_network_supported(a)&&a->net_ready()==0){
    TleHttp download;const char*source;Satellite candidate=*p;
    int csv=p->norad>=100000;
    int result=tle_network_fetch_sat(a,&download,&source,p->norad,p->id,csv,csv?csv_download:0,&candidate);
    if(result==-2||a->stop())return -2;
    if(!result){
     if(!csv){memcpy(candidate.first,download.first,70);memcpy(candidate.second,download.second,70);}
     if(!now(a,&utc)&&!catalog_orbit(&candidate,scratch)&&fabs((double)utc-scratch->epoch)<=86400){candidate.valid=1;candidate.epoch=scratch->epoch;*p=candidate;changed=1;}
    }
   }
  }
  if(p->valid&&fabs((double)utc-p->epoch)>7*86400)p->valid=0;
  valid+=p->valid;
 }
 if(a->stop())return -2;
 if(changed&&a->write){a->line(2,"Saving verified orbit cache");a->line(4,save(a,slot,utc,scratch)?"SD save failed / using RAM":"Orbit cache saved");}
 if(a->stop())return -2;
 if(!valid){error="No usable orbit / update SD";return -1;}
 char text[32];snprintf(text,sizeof(text),"Usable satellites: %u/%u",valid,satellite_count);a->line(2,text);a->sleep(500);return a->stop()?-2:0;
}
static unsigned scan_sat,inpass;static uint32_t scan_at,scan_end;static int scan_loaded;
void catalog_plan_reset(uint32_t utc){scan_sat=inpass=0;scan_at=utc;scan_end=utc+86400;scan_loaded=0;for(unsigned i=0;i<satellite_count;i++)satellites[i].aos=satellites[i].los=satellites[i].done=0;}
int catalog_plan_refresh(uint32_t utc){
 int changed=0;for(unsigned i=0;i<satellite_count;i++){Satellite*p=&satellites[i];if(p->los&&p->los<=utc){p->aos=p->los=p->done=0;changed=1;}}
 if(changed){scan_sat=0;scan_loaded=0;}return changed;
}
int catalog_plan_step(uint32_t utc,double lat,double lon,Orbit*scratch){
 /* Cooperative batches keep STOP responsive during a 24-hour scan. Each
  * satellite needs only its first current/future pass, not a full-day table. */
 for(unsigned work=0;work<6&&scan_sat<satellite_count;work++){
  Satellite*p=&satellites[scan_sat];
  if(p->done){scan_sat++;scan_loaded=0;continue;}
  if(!p->valid||fabs((double)utc-p->epoch)>7*86400){p->done=1;scan_sat++;scan_loaded=0;continue;}
  if(!scan_loaded){if(catalog_orbit(p,scratch)){p->valid=0;continue;}scan_loaded=1;scan_at=utc;scan_end=utc+86400;inpass=0;}
  Look q;if(orbit_look_profile(scratch,scan_at,lat,lon,0,p->rx,p->tx,&q)){p->valid=0;continue;}
  if(q.el>=0&&!inpass){p->aos=scan_at;inpass=1;}
  if(q.el<0&&inpass){p->los=scan_at;p->done=1;scan_sat++;scan_loaded=0;}
  else{scan_at+=30;if(scan_at>scan_end){p->done=1;scan_sat++;scan_loaded=0;}}
 }
 return scan_sat>=satellite_count;
}
int catalog_choose(uint32_t utc){
 int best=-1;
 /* Visible passes always win; order breaks overlaps. Otherwise the earliest
  * AOS wins, with order as the tie-break. A later high-priority pass does not
  * hide an earlier pass of another satellite. */
 for(unsigned i=0;i<satellite_count;i++){Satellite*p=&satellites[i];if(!p->valid||!p->done||!p->los||p->los<=utc||fabs((double)utc-p->epoch)>7*86400)continue;
  if(p->aos<=utc)return (int)i;
  if(best<0||p->aos<satellites[best].aos)best=(int)i;
 }return best;
}
