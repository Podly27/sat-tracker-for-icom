/* Real config/cache/OMM/SGP4 with simulated card and refresh transport. */
#include "../src/catalog.c"
#include <assert.h>
static char disk[3][8192],fresh[8192],original[8192];static int exists[3],fd=-1,write_mode,at,offline,old_clock,fetch_fail,write_fail,corrupt,stopflag,cancel_download,cancel_save,fetches,writes;
static const char*names[]={"SAT.CFG","SAT.TLE","SAT.BAK"};
static int stop_(void){return stopflag;}
static void clear_(void){}static void line_(unsigned n,const char*s){assert(n<8&&strlen(s)<32);}
static void sleep_(unsigned n){(void)n;}static int clock_(RadioClock*c){*c=(RadioClock){2026,11,old_clock?22:21,8,0,0,6};return 0;}
static int open_(const char*n,unsigned flags){assert(fd<0&&flags<=1);int i;for(i=0;i<3;i++)if(!strcmp(n,names[i]))break;assert(i<3);if(!flags&&!exists[i])return -1;fd=i;at=0;write_mode=flags;if(flags){assert(i!=1);disk[i][0]=0;exists[i]=1;}return i+1;}
static int close_(int f){assert(f==fd+1);fd=-1;return 0;}
static int read_(int f,void*b,int n){assert(f==fd+1&&!write_mode);int left=(int)strlen(disk[fd])-at;if(n>left)n=left;if(n>13)n=13;memcpy(b,disk[fd]+at,n);if(corrupt&&fd==2&&writes&&at>6&&n)((char*)b)[0]='x';at+=n;return n;}
static int write_(int f,const void*b,int n){assert(f==fd+1&&write_mode);writes++;if(write_fail)return -1;if(n>7)n=7;assert(at+n<8192);memcpy(disk[fd]+at,b,n);at+=n;disk[fd][at]=0;if(cancel_save)stopflag=1;return n;}
static int ready_(void){return offline?-1:0;}
int tle_network_supported(const Api*a){(void)a;return 1;}
int tle_network_fetch_sat(Api*a,TleHttp*p,const char**source,uint32_t id,const char*alpha,int csv,int(*record)(const char*,void*),void*ctx){
 (void)a;*source="mock";fetches++;if(cancel_download){stopflag=1;return -2;}if(fetch_fail&&id==27607)return -1;
 if(csv){assert(id==101101&&record);char*start=strstr(fresh,"CSV,");assert(start);char row[320];strcpy(row,start+4);char*end=strchr(row,'\n');if(end)*end=0;return record(row,ctx)==1?0:-1;}
 const char*s=fresh;char key[9];snprintf(key,sizeof(key),"1 %s",alpha);s=strstr(s,key);assert(s);memcpy(p->first,s,69);p->first[69]=0;s=strchr(s,'\n')+1;memcpy(p->second,s,69);p->second[69]=0;return 0;
}
static Api api={.abi=3,.radio=0x705,.build=0x07051405,.size=0x90,.clear=clear_,.line=line_,.sleep=sleep_,.clock=clock_,.stop=stop_,.open=open_,.read=read_,.write=write_,.close=close_,.net_ready=ready_};
static void file(const char*path,char*out){FILE*f=fopen(path,"rb");assert(f);size_t n=fread(out,1,8191,f);assert(n<8191);out[n]=0;fclose(f);}
static void reepoch(void){char*p=fresh;while((p=strstr(p,"1 "))){if(p==fresh||p[-1]=='\n'){memcpy(p+18,"26326",5);unsigned sum=0;for(unsigned i=0;i<68;i++)if(p[i]>='0'&&p[i]<='9')sum+=p[i]-'0';else if(p[i]=='-')sum++;p[68]='0'+sum%10;}p+=2;}p=strstr(fresh,"2026-11-20T");assert(p);memcpy(p,"2026-11-22T",11);}
int main(int argc,char**argv){assert(argc==4);const char*mode=argv[1];file(argv[2],disk[0]);file(argv[3],disk[1]);strcpy(original,disk[1]);strcpy(fresh,disk[1]);exists[0]=exists[1]=1;
 if(!strcmp(mode,"config")){
  assert(!catalog_config(&api)&&satellite_count==5);assert(satellites[0].norad==25544&&satellites[4].norad==101101&&!strcmp(satellites[4].id,"A1101"));
  const char*bad[]={"ISS,1,437.8,145.99,67","XX,25544,437.8,145.99,67","TOO-LONG,12345,437.8,145.9,0","XX,12345,145.9,145.8,0","XX,12345,437.8,145.9,66.6","XX,12345,437.8,145.9,nan","XX,12345,437.8,145.9,67,extra"};
  for(unsigned i=0;i<sizeof(bad)/sizeof(*bad);i++){assert(catalog_config_line(bad[i])<0);assert(satellite_count==5);}
  assert(!catalog_config_line("# disabled"));assert(!catalog_config_line("TEST1,12345,437.1,145.9,0"));assert(!catalog_config_line("TEST2,12346,437.1,145.9,0"));assert(!catalog_config_line("TEST3,12347,437.1,145.9,0"));assert(catalog_config_line("TEST4,12348,437.1,145.9,0")<0);return 0;
 }
 if(!strcmp(mode,"stale")||!strcmp(mode,"partial")||!strcmp(mode,"write-fail")||!strcmp(mode,"readback-fail")||!strcmp(mode,"cancel-download")||!strcmp(mode,"cancel-save")){old_clock=1;reepoch();}
 offline=!strcmp(mode,"offline");fetch_fail=!strcmp(mode,"partial");write_fail=!strcmp(mode,"write-fail");corrupt=!strcmp(mode,"readback-fail");cancel_download=!strcmp(mode,"cancel-download");cancel_save=!strcmp(mode,"cancel-save");
 if(offline)old_clock=1;
 if(!strcmp(mode,"missing"))exists[1]=0;
 Orbit scratch;int result=catalog_startup(&api,&scratch);assert(fd==-1);
 assert(!strcmp(original,disk[1]));
 if(cancel_download||cancel_save){assert(result==-2);return 0;}
 assert(!result&&satellite_count==5);for(unsigned i=0;i<5;i++)assert(satellites[i].valid);
 if(!old_clock&&!strcmp(mode,"fresh")){assert(!fetches&&!writes);}
 if(offline)assert(!fetches&&!writes);
 if(old_clock&&!offline){assert(fetches==5&&writes);if(fetch_fail)assert(satellites[1].epoch<satellites[0].epoch-86400);}
 uint32_t utc=utc_seconds(2026,11,old_clock?22:21,8,0,0);
 catalog_plan_reset(utc);unsigned count=0;while(!catalog_plan_step(utc,50,14,&scratch)){assert(++count<2500);}for(unsigned i=0;i<5;i++)assert(satellites[i].done&&satellites[i].los>satellites[i].aos&&satellites[i].aos>=utc&&satellites[i].los<=utc+86430);
 assert(catalog_choose(utc)>=0);
 for(unsigned i=0;i<5;i++){satellites[i].aos=utc+100+i*100;satellites[i].los=utc+200+i*100;}
 satellites[4].aos=utc+1;assert(catalog_choose(utc)==4);
 satellites[2].aos=utc-10;satellites[2].los=utc+20;assert(catalog_choose(utc)==2);
 satellites[0].aos=utc-10;satellites[0].los=utc+20;assert(catalog_choose(utc)==0);
 satellites[0].los=utc;assert(catalog_choose(utc)==2);
 assert(catalog_plan_refresh(utc));assert(!satellites[0].done&&satellites[2].done&&catalog_choose(utc)==2);satellites[2].valid=0;assert(catalog_choose(utc)==4);
 return 0;
}
