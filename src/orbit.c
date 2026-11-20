#include "orbit.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
uint32_t utc_seconds(int y,int m,int d,int h,int min,int s) {
    y-=m<=2;int era=y/400;unsigned yo=y-era*400;
    unsigned doy=(153*(m+(m>2?-3:9))+2)/5+d-1;
    int days=era*146097+(int)(yo*365+yo/4-yo/100+doy)-719468;
    return (uint32_t)days*86400u+h*3600u+min*60u+s;
}
static double field(const char *s,int at,int n) {char b[24];if(n>23)return 0;memcpy(b,s+at,n);b[n]=0;return atof(b);}
static int checksum(const char *s) {int sum=0;for(int i=0;i<68;i++){if(s[i]>='0'&&s[i]<='9')sum+=s[i]-'0';else if(s[i]=='-')sum++;}return s[68]>='0'&&s[68]<='9'&&sum%10==s[68]-'0';}
int orbit_init_id(Orbit *o,const char *a,const char *b,const char *id) {
    if(strlen(a)!=69||strlen(b)!=69||a[0]!='1'||b[0]!='2'||!checksum(a)||!checksum(b))return -1;
    if(!id||strlen(id)!=5||memcmp(a+2,id,5)||memcmp(b+2,id,5))return -2;
    memset(o,0,sizeof(*o));int yy=(int)field(a,18,2);yy+=yy<57?2000:1900;
    double day=field(a,20,12);if(day<1||day>=367)return -3;
    o->epoch=(double)utc_seconds(yy,1,1,0,0,0)+(day-1)*86400;
    ElsetRec *r=&o->rec;r->whichconst=wgs72;
    r->jdsatepoch=2440587.5+floor(o->epoch/86400);r->jdsatepochF=fmod(o->epoch,86400)/86400;
    r->inclo=field(b,8,8)*deg2rad;r->nodeo=field(b,17,8)*deg2rad;r->ecco=field(b,26,7)*1e-7;
    r->argpo=field(b,34,8)*deg2rad;r->mo=field(b,43,8)*deg2rad;r->no_kozai=field(b,52,11)*twopi/1440;
    double mant=field(a,54,5)*1e-5;if(a[53]=='-')mant=-mant;
    r->bstar=mant*pow(10,field(a,59,2));
    if(r->inclo<0||r->inclo>pi||r->ecco<0||r->ecco>=1||r->no_kozai<twopi/110||r->no_kozai>twopi/80)return -3;
    if(!sgp4init('i',r)||r->error)return -4;
    return 0;
}
int orbit_init(Orbit *o,const char *a,const char *b){return orbit_init_id(o,a,b,"25544");}
int grid_position(const char *s,double *lat,double *lon) {
    int n=0;while(s[n]&&s[n]!='\n'&&s[n]!='\r'&&s[n]!=' ')n++;
    if(n!=4&&n!=6)return -1;
    int a=s[0]&~32,b=s[1]&~32;
    if(a<'A'||a>'R'||b<'A'||b>'R'||s[2]<'0'||s[2]>'9'||s[3]<'0'||s[3]>'9')return -1;
    *lon=-180+(a-'A')*20+(s[2]-'0')*2;*lat=-90+(b-'A')*10+(s[3]-'0');
    if(n==4){*lon+=1;*lat+=0.5;}else{
        int c=s[4]&~32,d=s[5]&~32;if(c<'A'||c>'X'||d<'A'||d>'X')return -1;
        *lon+=(c-'A'+0.5)/12;*lat+=(d-'A'+0.5)/24;
    }return 0;
}
int orbit_look_profile(Orbit *o,uint32_t utc,double lat,double lon,double alt,uint32_t down,uint32_t uplink,Look *out) {
    if(down<100000000||down>500000000||uplink<100000000||uplink>500000000)return -1;
    if(lat<-90||lat>90||lon<-180||lon>180||alt<-500||alt>10000)return -1;
    double r[3],v[3];if(!sgp4(&o->rec,((double)utc-o->epoch)/60,r,v)||o->rec.error)return -2;
    double gm=gstime(2440587.5+(double)utc/86400),c=cos(gm),s=sin(gm);
    double x=c*r[0]+s*r[1],y=-s*r[0]+c*r[1],z=r[2];
    double vx=c*v[0]+s*v[1]+7.29211514670698e-5*y,vy=-s*v[0]+c*v[1]-7.29211514670698e-5*x,vz=v[2];
    lat*=deg2rad;lon*=deg2rad;double sl=sin(lat),cl=cos(lat),so=sin(lon),co=cos(lon);
    double n=6378.137/sqrt(1-0.00669437999014*sl*sl),h=alt/1000;
    double dx=x-(n+h)*cl*co,dy=y-(n+h)*cl*so,dz=z-(n*(1-0.00669437999014)+h)*sl;
    double east=-so*dx+co*dy,north=-sl*co*dx-sl*so*dy+cl*dz,up=cl*co*dx+cl*so*dy+sl*dz;
    out->range=sqrt(dx*dx+dy*dy+dz*dz);out->rate=(dx*vx+dy*vy+dz*vz)/out->range;
    out->az=atan2(east,north)/deg2rad;if(out->az<0)out->az+=360;
    out->el=atan2(up,sqrt(east*east+north*north))/deg2rad;
    out->down_hz=(int32_t)(down*(1-out->rate/299792.458)+0.5);
    out->up_hz=(int32_t)(uplink*(1+out->rate/299792.458)+0.5);
    return 0;
}

int orbit_look(Orbit *o,uint32_t utc,double lat,double lon,double alt,Look *out){return orbit_look_profile(o,utc,lat,lon,alt,437800000,145990000,out);}
