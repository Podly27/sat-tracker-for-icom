#include "tle_network.h"
#include <string.h>
#include <stdint.h>
#define BODY_LIMIT 262144u
static int numeric_mode;
static TleConnectDiag connection[2]={{-999,0,0},{-999,0,0}};
void tle_network_numeric(int enabled){numeric_mode=enabled!=0;}
const TleConnectDiag *tle_network_diagnostics(void){return connection;}
static const char *message="NET: NOT STARTED";
const char *tle_network_status(void){return message;}
void tle_http_init(TleHttp*p){memset(p,0,sizeof(*p));}
static int decimal(const char*s,unsigned*v){
 unsigned n=0;if(*s<'0'||*s>'9')return -1;
 while(*s>='0'&&*s<='9'){if(n>BODY_LIMIT/10)return -1;n=n*10+*s++-'0';if(n>BODY_LIMIT)return -1;}
 if(*s&&*s!='\r')return -1;*v=n;return 0;
}
static int check_line(const char*s,char n){
 if(strlen(s)!=69||s[0]!=n||s[1]!=' '||memcmp(s+2,"25544",5))return 0;
 unsigned sum=0;for(unsigned i=0;i<68;i++){if(s[i]>='0'&&s[i]<='9')sum+=s[i]-'0';else if(s[i]=='-')sum++;}
 return s[68]>='0'&&s[68]<='9'&&sum%10==(unsigned)(s[68]-'0');
}
static int body(TleHttp*p,char c){
 if(++p->body_bytes>BODY_LIMIT)return -1;
 if(c=='\n'){
  if(p->line_len&&p->line[p->line_len-1]=='\r')p->line_len--;
  p->line[p->line_len]=0;
  if(check_line(p->line,'1')){if(p->have_pair)return -1;memcpy(p->first,p->line,70);p->have_first=1;}
  else if(check_line(p->line,'2')&&p->have_first){if(p->have_pair)return -1;memcpy(p->second,p->line,70);p->have_pair=1;p->have_first=0;}
  else p->have_first=0;
  p->line_len=0;return 0;
 }
 if(c!='\r'&&(c<32||c>126))return -1;
 if(p->line_len>=sizeof(p->line)-1)return -1;p->line[p->line_len++]=c;return 0;
}
static int headers(TleHttp*p){
 char *s=p->header;
 if(strncmp(s,"HTTP/1.1 200 ",13)&&strncmp(s,"HTTP/1.0 200 ",13))return -1;
 for(unsigned i=0;i<p->header_len;i++)if(s[i]>='A'&&s[i]<='Z')s[i]+='a'-'A';
 char *line=strstr(s,"\r\n");if(!line)return -1;line+=2;
 unsigned lengths=0,encodings=0;
 while(*line&&strncmp(line,"\r\n",2)){
  char *end=strstr(line,"\r\n");if(!end)return -1;*end=0;
  if(!strncmp(line,"content-length:",15)){
   char*v=line+15;while(*v==' '||*v=='\t')v++;
   if(++lengths>1||decimal(v,&p->remaining))return -1;p->length_known=1;
  }else if(!strncmp(line,"transfer-encoding:",18)){
   char*v=line+18;while(*v==' '||*v=='\t')v++;
   if(++encodings>1||strcmp(v,"chunked"))return -1;p->chunked=1;
  }else if(!strncmp(line,"content-encoding:",17)){
   char*v=line+17;while(*v==' '||*v=='\t')v++;if(strcmp(v,"identity"))return -1;
  }
  line=end+2;
 }
 if(p->chunked&&p->length_known)return -1;
 if(p->length_known&&!p->remaining)p->complete=1;p->headers=1;return 0;
}
static int chunk_byte(TleHttp*p,char c){
 if(p->chunk_state==0){
  if(c!='\n'){if(p->chunk_len>=sizeof(p->chunk)-1)return -1;p->chunk[p->chunk_len++]=c;return 0;}
  if(!p->chunk_len||p->chunk[p->chunk_len-1]!='\r')return -1;
  p->chunk[--p->chunk_len]=0;unsigned n=0,i=0;
  for(;p->chunk[i]&&p->chunk[i]!=';';i++){
   unsigned ch=(unsigned char)p->chunk[i],v;
   if(ch>='0'&&ch<='9')v=ch-'0';else if(ch>='a'&&ch<='f')v=ch-'a'+10;else if(ch>='A'&&ch<='F')v=ch-'A'+10;else return -1;
   if(n>BODY_LIMIT/16)return -1;n=n*16+v;if(n>BODY_LIMIT)return -1;
  }
  if(!i)return -1;p->chunk_len=0;p->chunk_remaining=n;p->chunk_state=n?1:4;return 0;
 }
 if(p->chunk_state==1){if(body(p,c))return -1;if(!--p->chunk_remaining)p->chunk_state=2;return 0;}
 if(p->chunk_state==2){if(c!='\r')return -1;p->chunk_state=3;return 0;}
 if(p->chunk_state==3){if(c!='\n')return -1;p->chunk_state=0;return 0;}
 /* Reject trailers rather than accept framing that has not been tested. */
 if(p->chunk_state==4){if(c!='\r')return -1;p->chunk_state=5;return 0;}
 if(p->chunk_state==5){if(c!='\n')return -1;p->complete=1;return 0;}
 return -1;
}
int tle_http_feed(TleHttp*p,const char*b,unsigned n){
 if(p->failed)return -1;
 for(unsigned i=0;i<n;i++){
  char c=b[i];int r=0;
  if(p->complete){p->failed=1;return -1;}
  if(!p->headers){
   if(!c||p->header_len>=sizeof(p->header)-1){p->failed=1;return -1;}
   p->header[p->header_len++]=c;p->header[p->header_len]=0;
   if(p->header_len>=4&&!memcmp(p->header+p->header_len-4,"\r\n\r\n",4))r=headers(p);
  }else if(p->chunked)r=chunk_byte(p,c);
  else{
   r=body(p,c);
   if(p->length_known&&!--p->remaining)p->complete=1;
  }
  if(r){p->failed=1;return -1;}
 }
 return 0;
}
int tle_http_finish(TleHttp*p){
 if(p->failed||!p->headers||(p->chunked&&!p->complete)||(p->length_known&&!p->complete))return -1;
 if(p->line_len&&body(p,'\n'))return -1;
 return p->have_pair?0:-1;
}
int tle_network_supported(const Api*a){
 if(!a||a->radio!=0x705||a->build!=0x07051405||a->abi!=3||a->size<0x84)return 0;
 uintptr_t connect=(uintptr_t)a->net_connect;
 if(connect==0x2037c455)return (uintptr_t)a->net_ready==0x2037b551&&(uintptr_t)a->net_send==0x2037bb29&&(uintptr_t)a->net_recv==0x2037bc3d&&(uintptr_t)a->net_close==0x2037b611&&(uintptr_t)a->net_error==0x2037b563;
 if(connect==0x2037c601)return (uintptr_t)a->net_ready==0x2037b579&&(uintptr_t)a->net_send==0x2037bc21&&(uintptr_t)a->net_recv==0x2037bd35&&(uintptr_t)a->net_close==0x2037b639&&(uintptr_t)a->net_error==0x2037b58b;
 return 0;
}
/* Private entry also used by host transport tests after explicit mock binding. */
int tle_network_fetch_bound(Api*a,TleHttp*p,const char **source){
 static const char *hosts[]={"celestrak.org","www.amsat.org"};
 /* Public IPv4 addresses resolved 2026-11-21; diagnostic fixtures, not permanent endpoints. */
 static const char *ips[]={"104.168.149.178","104.20.25.253"};
 memset(connection,0,sizeof(connection));connection[0].result=connection[1].result=-999;
 static const char *requests[]={
  "GET /NORAD/elements/gp.php?CATNR=25544&FORMAT=TLE HTTP/1.1\r\nHost: celestrak.org\r\nUser-Agent: ISS-tracker/0.17-probe\r\nAccept-Encoding: identity\r\nConnection: close\r\n\r\n",
  "GET /tle/current/nasabare.txt HTTP/1.1\r\nHost: www.amsat.org\r\nUser-Agent: ISS-tracker/0.17-probe\r\nAccept-Encoding: identity\r\nConnection: close\r\n\r\n"};
 for(unsigned i=0;i<2;i++){
  if(a->stop()){message="NET: CANCELLED";return -2;}
  if(a->net_ready()!=0){message="NET: OFFLINE";return -1;}
  *source=hosts[i];message="NET: HTTPS CONNECT";a->line(3,hosts[i]);a->line(4,message);
  /* Flags bit 0 selects protocol 100, the firmware's secure TCP path.
   * Flags bit 2 would select UDP and is deliberately never used here. */
  unsigned connect_start=a->ticks();
  int fd=a->net_connect(numeric_mode?ips[i]:hosts[i],443,1,10000);
  connection[i].result=fd;connection[i].error=fd<0&&a->net_error?a->net_error():0;connection[i].elapsed=a->ticks()-connect_start;
  if(fd<0){message="NET: CONNECT FAILED";continue;}
  unsigned start=a->ticks(),sent=0;int bad=0;
  while(sent<strlen(requests[i])){
   if(a->stop()||a->ticks()-start>20000){bad=1;break;}
   int n=a->net_send(fd,requests[i]+sent,strlen(requests[i])-sent,500);
   if(n<=0||(unsigned)n>strlen(requests[i])-sent){bad=1;break;}sent+=n;
  }
  tle_http_init(p);char buffer[256];message="NET: READING TLE";a->line(4,message);
  while(!bad&&!p->complete){
   if(a->stop()||a->ticks()-start>20000){bad=1;break;}
   int n=a->net_recv(fd,buffer,sizeof(buffer),500);
   if(n<0||n>(int)sizeof(buffer)){bad=1;break;}
   if(!n)break;
   if(tle_http_feed(p,buffer,(unsigned)n)){bad=1;break;}
   a->sleep(1);
  }
  a->net_close(fd);
  if(a->stop()){message="NET: CANCELLED";return -2;}
  if(!bad&&!tle_http_finish(p)){message="NET: TLE RECEIVED";return 0;}
  message="NET: RESPONSE REJECTED";
 }
 return -1;
}
int tle_network_fetch(Api*a,TleHttp*p,const char **source){
 if(!tle_network_supported(a)){message="NET: UNSUPPORTED API";return -1;}
 return tle_network_fetch_bound(a,p,source);
}
