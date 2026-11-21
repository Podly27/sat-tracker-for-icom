/* Version-specific CI-V request mailbox. No direct call to command handlers.
 * Original Icom task consumes complete ordinary CI-V requests asynchronously.
 * Published bytes match the original UART parser. No PTT, service, reset,
 * calibration, arbitrary memory writes or arbitrary CI-V command interface.
 */
#include "civ_queue.h"
#include "profile.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
static volatile uint8_t *const q=(volatile uint8_t*)0x2038856c;
static volatile uint8_t *const uart=(volatile uint8_t*)0x203800e8;
static volatile uint8_t *const counter=(volatile uint8_t*)0x203800dd;
static int supported(void){
 static const uint32_t signature[]={0xe92d41f0,0xe1a05000,0xe59f7da4,0xe3510000};
 return !memcmp((const void*)0x20028a98,signature,sizeof(signature))&&
  *(const uint32_t*)0x20029860==0x2038856c&&*(const uint32_t*)0x20029848==0x203800e8;
}
/* Read-only transport snapshots kept in app RAM. No foreign queue reset. */
typedef struct {uint32_t tick;uint8_t valid,kind,length,payload[7],raw[42];} CivTrace;
static CivTrace trace[6];static int trace_ready;
static void trace_read(unsigned slot,unsigned kind,const uint8_t*payload,unsigned length){
 if(!trace_ready)return;CivTrace*s=&trace[slot];s->tick=*(volatile const uint32_t*)0x20380e08;s->kind=kind;s->length=length;
 for(unsigned i=0;i<7;i++)s->payload[i]=i<length?payload[i]:0;
 unsigned at=0;for(unsigned i=0;i<9;i++)s->raw[at++]=q[i];s->raw[at++]=q[0x82];
 for(unsigned i=0;i<16;i++)s->raw[at++]=q[0x83+i];s->raw[at++]=q[0x103];
 for(unsigned i=0;i<9;i++)s->raw[at++]=uart[i];for(unsigned i=0;i<6;i++)s->raw[at++]=counter[i];s->valid=1;
}
void civ_trace_start(void){memset(trace,0,sizeof(trace));trace_ready=supported();if(trace_ready){unsigned cpsr;__asm__ volatile("mrs %0, cpsr\n cpsid i":"=r"(cpsr)::"memory");trace_read(0,0,0,0);__asm__ volatile("msr cpsr_c, %0"::"r"(cpsr):"memory");}}
void civ_trace_save(Api*a){
 if(!a->write)return;unsigned cpsr;__asm__ volatile("mrs %0, cpsr\n cpsid i":"=r"(cpsr)::"memory");trace_read(5,0,0,0);__asm__ volatile("msr cpsr_c, %0"::"r"(cpsr):"memory");
 int fd=a->open("SATDBG.LOG",1);if(fd<0)return;
 static const char*names[]={"BOOT","FIRST","REPLY","FREE","BUSY","FAIL"};static const char hex[]="0123456789ABCDEF";
 for(unsigned i=0;i<6&&!a->stop();i++){CivTrace*s=&trace[i];if(!s->valid)continue;char line[160];int n=snprintf(line,sizeof(line),"%s T=%lu K=%u N=%u P=",names[i],(unsigned long)s->tick,s->kind,s->length);
  for(unsigned j=0;j<7;j++){line[n++]=hex[s->payload[j]>>4];line[n++]=hex[s->payload[j]&15];}line[n++]=' ';line[n++]='D';line[n++]='=';
  for(unsigned j=0;j<42;j++){line[n++]=hex[s->raw[j]>>4];line[n++]=hex[s->raw[j]&15];}line[n++]='\n';
  int at=0;while(at<n&&!a->stop()){int r=a->write(fd,line+at,n-at);if(r<=0||r>n-at)break;at+=r;}
 }
 a->close(fd);
}
static int publish(const uint8_t *payload,unsigned length){
 if(length<1||length>7)return -2;
 if(!supported())return -2;
 unsigned cpsr;__asm__ volatile("mrs %0, cpsr\n cpsid i":"=r"(cpsr)::"memory");
 int result=-1;
 if(!q[1]&&!uart[1]&&!uart[3]&&!*counter){
  q[2]=0xe0;for(unsigned i=0;i<length;i++)q[3+i]=payload[i];
  /* Match the original frame-completion path at 20028c24..20028c54,
   * including UART ownership/mode and retained parser length. Previously
   * only q matched; omitted uart[0] affected the native response path.
   * Keep the conservative all-idle guard. Do not erase stale/busy state.
   */
  q[0x82]=length+3;
  *counter=length+3;counter[1]=0;
  uart[0]=1;uart[1]=1;uart[4]=0;uart[5]=0;
  __asm__ volatile("dmb sy":::"memory");q[1]=1;result=0;
 }
 __asm__ volatile("msr cpsr_c, %0"::"r"(cpsr):"memory");return result;
}
/* Read-only diagnostics: q pending, UART states 0/1/3, parser count.
 * Keep every existing busy guard until the real idle state is known. */
const char *civ_busy_status(void){
 static char text[32];
 snprintf(text,sizeof(text),"Q%XR%XC%02X U%02X%02X%02X%02X%02X%02X%02X%02X%02X",(unsigned)q[1],(unsigned)q[0x103],(unsigned)*counter,
  (unsigned)uart[0],(unsigned)uart[1],(unsigned)uart[2],(unsigned)uart[3],(unsigned)uart[4],(unsigned)uart[5],(unsigned)uart[6],(unsigned)uart[7],(unsigned)uart[8]);
 return text;
}
int civ_read_probe_begin(void){static const uint8_t read[]={3};return publish(read,1);}
int civ_read_probe_pending(void){return q[1]!=0;}
static int owned_begin(CivOwned *t,const uint8_t *payload,unsigned length,unsigned kind){
 if(!t||t->active)return -1;
 if(length<1||length>7||!supported())return -2;
 unsigned cpsr;__asm__ volatile("mrs %0, cpsr\n cpsid i":"=r"(cpsr)::"memory");
 int result=-1;
 /* Only claim a wholly idle path with no uncollected reply. */
 if(trace_ready&&!trace[1].valid)trace_read(1,kind,payload,length);
 int idle=!q[1]&&!q[0x103]&&!*counter&&uart[0]<=1;
 for(unsigned i=1;i<9;i++)if(uart[i])idle=0;
 if(idle){
  for(unsigned i=0;i<9;i++)t->uart_before[i]=uart[i];
  for(unsigned i=0;i<6;i++)t->counter_before[i]=counter[i];
  memcpy(t->payload,payload,length);t->length=length;t->kind=kind;t->read_value=0;
  result=publish(payload,length);
  if(!result)t->active=1;
 }else trace_read(4,kind,payload,length);
 __asm__ volatile("msr cpsr_c, %0"::"r"(cpsr):"memory");return result;
}
int civ_owned_poll(CivOwned *t){
 if(!t||!t->active||!supported())return -3;
 unsigned cpsr;__asm__ volatile("mrs %0, cpsr\n cpsid i":"=r"(cpsr)::"memory");
 int result=0;
 /* Abort on a foreign request or native transport progress; never force idle. */
 int own=q[2]==0xe0&&q[0x82]==t->length+3;
 for(unsigned i=0;i<t->length;i++)if(q[3+i]!=t->payload[i])own=0;
 for(unsigned i=0;i<9;i++){
  unsigned expected=i<2?1:(i==4||i==5)?0:t->uart_before[i];
  if(uart[i]!=expected)own=0;
 }
 for(unsigned i=0;i<6;i++){
  unsigned expected=i==0?t->length+3:i==1?0:t->counter_before[i];
  if(counter[i]!=expected)own=0;
 }
 if(!own)result=-3;
 else if(!q[1]&&q[0x103]){
  volatile const uint8_t *r=q+0x83;
  if(q[0x103]!=1||r[0]!=0xe0||r[1]!=0xa4)result=-3;
  else if(r[2]==0xfa&&r[3]==0xfd)result=2;
  else if(t->kind==0&&r[2]==0xfb&&r[3]==0xfd)result=1;
  else if((((t->kind==1||t->kind==3)&&r[2]==0x25&&r[3]==(t->kind==1?1:0))||
           (t->kind==5&&r[2]==0x1c&&r[3]==3))&&r[9]==0xfd){
   uint64_t hz=0,mult=1;int valid=1;
   for(unsigned i=4;i<9;i++){
    unsigned x=r[i];if((x&15)>9||(x>>4)>9)valid=0;
    hz+=((x>>4)*10+(x&15))*mult;mult*=100;
   }
   if(valid&&hz<=0xffffffffu){t->read_value=(uint32_t)hz;result=1;}else result=-3;
  }else if(t->kind==2&&r[2]==0x0f&&r[3]<=1&&r[4]==0xfd){t->read_value=r[3];result=1;}
  else if(t->kind==4&&r[2]==0x26&&r[3]==0&&r[5]<=1&&r[6]>=1&&r[6]<=3&&r[7]==0xfd){t->read_value=r[4];result=1;}
  else if(t->kind>=10&&t->kind<=15){
   unsigned field=t->kind-10;const uint8_t*p=t->payload;
   unsigned prefix=field==CIV_FREQ||field==CIV_MODE||field==CIV_TONE||field==CIV_TONE_ON||field==CIV_TSQL?2:1;
   unsigned n=field==CIV_FREQ?5:field==CIV_MODE||field==CIV_TONE?3:1;
   int valid=1;for(unsigned i=0;i<prefix;i++)if(r[2+i]!=p[i])valid=0;
   if(r[2+prefix+n]!=0xfd)valid=0;
   uint32_t value=0;
   if(field==CIV_FREQ){uint64_t hz=0,m=1;for(unsigned i=0;i<n;i++){unsigned x=r[2+prefix+i];if((x&15)>9||(x>>4)>9)valid=0;hz+=((x>>4)*10+(x&15))*m;m*=100;}if(hz>470000000||hz<30000)valid=0;value=(uint32_t)hz;}
   else{for(unsigned i=0;i<n;i++)value|=(uint32_t)r[2+prefix+i]<<(i*8);}
   if(field==CIV_MODE&&((value&255)>0x17||((value>>8)&255)>1||((value>>16)&255)<1||((value>>16)&255)>3))valid=0;
   if((field==CIV_TONE_ON||field==CIV_TSQL)&&value>1)valid=0;
   if(field==CIV_SPLIT&&value!=0&&value!=1&&value!=0x10&&value!=0x11&&value!=0x12)valid=0;
   if(field==CIV_TONE){for(unsigned i=0;i<3;i++){unsigned x=(value>>(i*8))&255;if((x&15)>9||(x>>4)>9)valid=0;}if((value&255)!=0)valid=0;}
   if(valid){t->read_value=value;result=1;}else result=-3;
  }else result=-3;
  if(result==1||result==2){
   trace_read(2,t->kind,t->payload,t->length);
   /* Consume ONLY our correlated internal reply. It must not also be sent
    * to the physical REMOTE port. Restore only bytes changed by publish;
    * unchanged native fields and request/reply data are left untouched.
    */
   q[0x103]=0;counter[0]=t->counter_before[0];counter[1]=t->counter_before[1];
   uart[0]=t->uart_before[0];uart[1]=t->uart_before[1];
   uart[4]=t->uart_before[4];uart[5]=t->uart_before[5];
   __asm__ volatile("dmb sy":::"memory");t->active=0;trace_read(3,t->kind,t->payload,t->length);
  }
 }
 __asm__ volatile("msr cpsr_c, %0"::"r"(cpsr):"memory");return result;
}
int civ_iss_frequency(unsigned unselected,uint32_t hz){
 if(unselected>1)return -2;
 if((!unselected&&(hz<437770000u||hz>437830000u))||(unselected&&(hz<145980000u||hz>145999999u)))return -2;
 uint8_t data[7]={0x25,(uint8_t)unselected};
 for(unsigned i=2;i<7;i++){unsigned pair=hz%100;data[i]=(pair/10)*16+pair%10;hz/=100;}
 return publish(data,7);
}
static int owned_frequency(CivOwned *t,unsigned up,uint32_t hz){
 if(!profile_target(up,hz))return -2;
 uint8_t data[7]={0x25,(uint8_t)up};
 for(unsigned i=2;i<7;i++){unsigned p=hz%100;data[i]=(p/10)*16+p%10;hz/=100;}
 return owned_begin(t,data,7,0);
}
int civ_owned_rx_begin(CivOwned*t,uint32_t hz){return owned_frequency(t,0,hz);}
int civ_owned_up_begin(CivOwned*t,uint32_t hz){return owned_frequency(t,1,hz);}
int civ_owned_up_read(CivOwned*t){static const uint8_t p[]={0x25,1};return owned_begin(t,p,2,1);}
int civ_owned_rx_read(CivOwned*t){static const uint8_t p[]={0x25,0};return owned_begin(t,p,2,3);}
int civ_owned_mode_read(CivOwned*t){static const uint8_t p[]={0x26,0};return owned_begin(t,p,2,4);}
int civ_owned_tx_read(CivOwned*t){static const uint8_t p[]={0x1c,3};return owned_begin(t,p,2,5);}
int civ_owned_split_read(CivOwned*t){static const uint8_t p[]={0x0f};return owned_begin(t,p,1,2);}
int civ_setting_read(CivOwned*t,unsigned field){
 static const uint8_t commands[][2]={{0x25,0},{0x26,0},{0x1b,0},{0x16,0x42},{0x16,0x43},{0x0f,0}};
 if(field>CIV_SPLIT)return -2;
 return owned_begin(t,commands[field],field==CIV_SPLIT?1:2,10+field);
}
int civ_setting_write(CivOwned*t,unsigned field,uint32_t v){
 uint8_t p[7];unsigned n=0;
 if(field==CIV_SIMPLEX){if(v)return -2;p[n++]=0x0f;p[n++]=0x10;}
 else if(field==CIV_SELECT){if(v>1)return -2;p[n++]=7;p[n++]=v;}
 else if(field==CIV_FREQ){if(v<30000||v>470000000)return -2;p[n++]=0x25;p[n++]=0;for(unsigned i=0;i<5;i++){unsigned x=v%100;p[n++]=(x/10)*16+x%10;v/=100;}}
 else if(field==CIV_MODE){if((v&255)>0x17||((v>>8)&255)>1||((v>>16)&255)<1||((v>>16)&255)>3||v>>24)return -2;p[n++]=0x26;p[n++]=0;for(unsigned i=0;i<3;i++)p[n++]=v>>(i*8);}
 else if(field==CIV_TONE){if((v&255)||v>>24)return -2;for(unsigned i=0;i<3;i++){unsigned x=(v>>(i*8))&255;if((x&15)>9||(x>>4)>9)return -2;}p[n++]=0x1b;p[n++]=0;for(unsigned i=0;i<3;i++)p[n++]=v>>(i*8);}
 else if(field==CIV_TONE_ON||field==CIV_TSQL){if(v>1)return -2;p[n++]=0x16;p[n++]=field==CIV_TONE_ON?0x42:0x43;p[n++]=v;}
 else if(field==CIV_SPLIT){if(v>1)return -2;p[n++]=0x0f;p[n++]=v;}
 else return -2;
 return owned_begin(t,p,n,0);
}
static int profile_publish(CivOwned*t,const uint8_t*p,unsigned n){return t?owned_begin(t,p,n,0):publish(p,n);}
static int profile_send(CivOwned*t,unsigned step){

 static const uint8_t a[]={7,0},b[]={7,1},fm0[]={0x26,0,5,0,1},fm1[]={0x26,1,5,0,1};
 static const uint8_t tsqoff[]={0x16,0x43,0};
 unsigned tone=profile_tone?profile_tone:670;
 uint8_t tone_data[]={0x1b,0,0,(uint8_t)(((tone/1000)<<4)|((tone/100)%10)),(uint8_t)((((tone/10)%10)<<4)|(tone%10))};
 uint8_t ton[]={0x16,0x42,(uint8_t)(profile_tone!=0)};
 static const uint8_t simplex[]={0x0f,0x10},split[]={0x0f,1};
 switch(step){
 case 0:return profile_publish(t,a,sizeof(a));
 case 3:return profile_publish(t,fm0,sizeof(fm0));
 case 4:return profile_publish(t,fm1,sizeof(fm1));
 case 5:return profile_publish(t,b,sizeof(b));
 case 6:return profile_publish(t,tone_data,sizeof(tone_data));
 case 7:return profile_publish(t,ton,sizeof(ton));
 case 8:case 10:return profile_publish(t,tsqoff,sizeof(tsqoff));
 case 9:return profile_publish(t,a,sizeof(a));
 case 11:case 13:return profile_publish(t,simplex,sizeof(simplex));
 case 12:return profile_publish(t,split,sizeof(split));
 default:return -2;
 }
}

int civ_iss_profile(unsigned step){return profile_send(0,step);}
int civ_owned_profile_begin(CivOwned*t,unsigned step){return t?profile_send(t,step):-2;}
