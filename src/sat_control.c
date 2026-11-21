/* ISS A=RX, B=uplink, FM/67 Hz/split. All configuration/tuning occurs in RX.
 * No PTT. ACKs and UP/split readback use the owned transaction mechanism.
 */
#include "sat_control.h"
#include "civ_queue.h"
#include "profile.h"
#include "tuner.h"
#include "radio_mode.h"
#ifdef ISS_TX_CONTROL
#include "tx_control.h"
#endif
#include <stdio.h>
enum {INIT=1,RX,UP,READ_UP,READ_SPLIT,RESTORE_A,READ_RX,READ_MODE};
static CivOwned t;
static unsigned step,action,ready,selected_b,restore_failed,phase;
static uint32_t sent_at,last_at,init_rx,init_up,cycle_rx,cycle_up,rx_hz,up_hz,acks;
static uint32_t resume_until;
static const char *failure,*saved_failure;
static char text[32];
static uint32_t diff(uint32_t a,uint32_t b){return a>b?a-b:b-a;}
uint32_t sat_control_rx(void){return rx_hz;}
uint32_t sat_control_up(void){return up_hz;}
static void submitted(unsigned what,uint32_t now){action=what;sent_at=now;}
const char *sat_control_tick(Api*a,uint32_t down,uint32_t up,int gps,int orbit,int stopping){
 uint32_t now=a->ticks();RadioState r={0};int have=a->radio_state(&r)==0;
 if(t.active&&(!failure||action==RESTORE_A)){
  int result=civ_owned_poll(&t);
  if(result==1){
   acks++;
   if(action==RESTORE_A){selected_b=0;ready=0;step=phase=0;failure=saved_failure;saved_failure=0;}
   else if(action==INIT){
    if(step==3)rx_hz=init_rx;
    if(step==5)selected_b=1;
    if(step==10)selected_b=0;
    if(step==14){up_hz=t.read_value;if(diff(up_hz,init_up)>10)failure="SAT STOP: UP READBACK";}
    if(step==15&&t.read_value!=1)failure="SAT STOP: SPLIT READBACK";
    step++;
    if(step==17){rx_hz=t.read_value;if(diff(rx_hz,init_rx)>10){snprintf(text,sizeof(text),"RX ERR %lu/%lu",(unsigned long)init_rx,(unsigned long)rx_hz);failure=text;}}
    if(step==18&&t.read_value!=5){snprintf(text,sizeof(text),"MODE ERR %lu (want 5)",(unsigned long)t.read_value);failure=text;}
    if(step==18&&!failure){ready=1;last_at=now;phase=0;}
   }else if(action==RX){rx_hz=cycle_rx;phase=1;}
   else if(action==UP)phase=2;
   else if(action==READ_UP){up_hz=t.read_value;if(diff(up_hz,cycle_up)>10)failure="SAT STOP: UP READBACK";else phase=3;}
   else if(action==READ_SPLIT){if(t.read_value!=1)failure="SAT STOP: SPLIT CHANGED";else phase=4;}
   else if(action==READ_RX){rx_hz=t.read_value;if(diff(rx_hz,cycle_rx)>10){snprintf(text,sizeof(text),"RX ERR %lu/%lu",(unsigned long)cycle_rx,(unsigned long)rx_hz);failure=text;}else phase=5;}
   else if(action==READ_MODE){if(t.read_value!=5){snprintf(text,sizeof(text),"MODE ERR %lu (want 5)",(unsigned long)t.read_value);failure=text;}else{phase=0;last_at=now;}}
  }else if(result==2)failure="SAT STOP: RADIO REJECTED";
  else if(result<0)failure="SAT STOP: STATE CHANGED";
  else if(now-sent_at>3000)failure="SAT STOP: ACK TIMEOUT";
  if(failure&&action==RESTORE_A&&selected_b)restore_failed=1;
 }
 if(t.active)return failure?failure:"SAT: WAIT ACK";
 /* Restore A after an interrupted B-tone setup, including STOP or fix loss.
  * Never attempt this while TX or after an unowned/unknown active request.
  */
 if(selected_b&&(stopping||!gps||!orbit||failure)&&!restore_failed){
  if(!have||r.tx){restore_failed=1;failure="SAT STOP: SELECT A IN RX";return failure;}
  if(civ_owned_profile_begin(&t,9)){restore_failed=1;failure="SAT STOP: RESTORE A BUSY";return failure;}
  saved_failure=failure;failure=0;
  submitted(RESTORE_A,now);return "SAT: RESTORING VFO A";
 }
 if(failure)return failure;
 if(stopping)return "SAT: STOP COMPLETE";
 if(!have){failure="SAT STOP: NO RADIO STATUS";return failure;}
#ifdef ISS_TX_CONTROL
 if(ready&&(r.tx||tx_control_busy())){
  /* Discard a partially completed RX correction cycle across TX. */
  phase=0;
  const char *state=tx_control_tick(a,up,gps,orbit,0,rx_hz);
  if(tx_control_up())up_hz=tx_control_up();
  if(state)return state;
  /* On release, catch RX up promptly; allow the display cache to settle. */
  last_at=now-2000;resume_until=now+1000;
 }
#endif
 if(r.tx)return "SAT HOLD: TRANSMITTING";
 if(ready&&!phase&&now-last_at>=500&&(!resume_until||(int32_t)(now-resume_until)>=0)&&diff(r.main,rx_hz)>10){failure="SAT STOP: MANUAL CHANGE";return failure;}
 if(!gps)return "SAT HOLD: WAIT GPS";
 if(!orbit)return "SAT HOLD: TLE/TIME";
 if(!profile_target(0,down)||!profile_target(1,up)){failure="SAT STOP: TARGET RANGE";return failure;}
 if(!ready){
  if(step==0){init_rx=down;init_up=up;}
  int result=step==3?civ_owned_rx_begin(&t,init_rx):step==4?civ_owned_up_begin(&t,init_up):
   step==14?civ_owned_up_read(&t):step==15?civ_owned_split_read(&t):
   step==16?civ_owned_rx_read(&t):step==17?civ_owned_mode_read(&t):
   civ_owned_profile_begin(&t,step==1?3:step==2?4:step==9?13:step>=10?step-1:step);
  if(result){failure="SAT STOP: INIT BUSY/UNSUPPORTED";return failure;}
  submitted(INIT,now);snprintf(text,sizeof(text),"SAT SETUP %u/18 - NO PTT",step+1);return text;
 }
 if(!phase&&(now-last_at<2000||(diff(rx_hz,down)<50&&diff(up_hz,up)<20))){
#ifdef ISS_TX_CONTROL
  if(tx_control_error()){snprintf(text,sizeof(text),"RX / %s",tx_control_error());return text;}
  snprintf(text,sizeof(text),"SAT RX ACK %lu TX OK %lu",(unsigned long)acks,(unsigned long)tx_control_verified());return text;
#else
  snprintf(text,sizeof(text),"SAT RX SPLIT / ACK %lu",(unsigned long)acks);return text;
#endif
 }
 int result;
 if(!phase){cycle_rx=down;cycle_up=up;result=civ_owned_rx_begin(&t,cycle_rx);action=RX;}
 else if(phase==1){result=civ_owned_up_begin(&t,cycle_up);action=UP;}
 else if(phase==2){result=civ_owned_up_read(&t);action=READ_UP;}
 else if(phase==3){result=civ_owned_split_read(&t);action=READ_SPLIT;}
 else if(phase==4){result=civ_owned_rx_read(&t);action=READ_RX;}
 else{result=civ_owned_mode_read(&t);action=READ_MODE;}
 if(result){failure="SAT STOP: CIV BUSY";return failure;}
 sent_at=now;return "SAT: WAIT ACK";
}
void sat_control_finish(Api*a){
#ifdef ISS_TX_CONTROL
 tx_control_finish(a,rx_hz);
#endif
 while((t.active&&!failure)||(selected_b&&!t.active&&!restore_failed)){
  sat_control_tick(a,0,0,0,0,1);
  if((t.active&&!failure)||(selected_b&&!t.active&&!restore_failed))a->sleep(50);
 }
}

int sat_control_profile(Api*a,uint32_t rx,uint32_t up,unsigned tone){
 RadioState r;if(!profile_valid(rx,up,tone)||a->radio_state(&r)||r.tx)return -1;
 sat_control_finish(a);
 if(t.active||selected_b)return -1;
#ifdef ISS_TX_CONTROL
 if(tx_control_reset())return -1;
#endif
 profile_set(rx,up,tone);step=action=ready=selected_b=restore_failed=phase=0;
 sent_at=last_at=init_rx=init_up=cycle_rx=cycle_up=rx_hz=up_hz=acks=resume_until=0;
 failure=saved_failure=0;return 0;
}
