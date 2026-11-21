/* Active-TX controller: ONLY 25 01 uplink frequency writes; all other
 * requests are read-only. No VFO selection, mode/tone/split writes or PTT.
 * Mapping must be A=selected RX, B=unselected UP even under split TX.
 * An unexpected mapping is rejected, never guessed or switched in TX.
 */
#include "tx_control.h"
#include "civ_queue.h"
#include "profile.h"
#include "tuner.h"
#include <stdio.h>
enum {MAP_SPLIT,MAP_RX,MAP_UP,MAP_TX,UPDATE,VERIFY_UP,VERIFY_TX};
static CivOwned t;
static unsigned phase,pending,seen_tx,first_write,tx_read_supported=1;
static uint32_t sent_at,last_at,rx_sample,up_sample,up_hz,target,updates;
static const char *failure;
static char text[32];
static uint32_t diff(uint32_t a,uint32_t b){return a>b?a-b:b-a;}
static int valid_mapping(uint32_t rx){return diff(rx_sample,rx)<=10&&profile_target(1,up_sample);}
int tx_control_busy(void){return seen_tx||t.active;}
uint32_t tx_control_up(void){return up_hz;}
uint32_t tx_control_verified(void){return updates;}
const char *tx_control_error(void){return failure;}
const char *tx_control_tick(Api*a,uint32_t up,int gps,int orbit,int stopping,uint32_t rx){
 uint32_t now=a->ticks();RadioState r={0};int have=a->radio_state(&r)==0;
 if(t.active&&!failure){
  int result=civ_owned_poll(&t);
  if(result==1){
   if(pending==MAP_SPLIT){if(t.read_value!=1)failure="TX STOP: SPLIT IS OFF";else phase=MAP_RX;}
   else if(pending==MAP_RX){rx_sample=t.read_value;phase=MAP_UP;}
   else if(pending==MAP_UP){up_sample=t.read_value;phase=MAP_TX;}
   else if(pending==MAP_TX){
    if(!valid_mapping(rx))
     failure="TX STOP: VFO MAPPING";
    else if(diff(t.read_value,up_sample)>10)failure="TX STOP: TX/B MISMATCH";
    else{up_hz=t.read_value;phase=UPDATE;first_write=1;last_at=now-1000;}
   }else if(pending==UPDATE)phase=VERIFY_UP;
   else if(pending==VERIFY_UP){
    up_hz=t.read_value;
    if(diff(up_hz,target)>10)failure="TX STOP: B READBACK";
    else if(tx_read_supported)phase=VERIFY_TX;
    else{if(have&&r.tx)updates++;phase=UPDATE;last_at=now;first_write=0;}
   }else if(pending==VERIFY_TX){
    if(diff(t.read_value,target)>10)failure="TX STOP: TX READBACK";
    else{up_hz=t.read_value;if(have&&r.tx)updates++;phase=UPDATE;last_at=now;first_write=0;}
   }
  }else if(result==2&&(pending==MAP_TX||pending==VERIFY_TX)){
   /* Older firmware may reject 1C03. Do not mistake B readback for an
    * independent TX read. Only this read-only command has a fallback.
    */
   tx_read_supported=0;
   if(pending==MAP_TX){
    if(!valid_mapping(rx))failure="TX STOP: VFO MAPPING";
    else{up_hz=up_sample;phase=UPDATE;first_write=1;last_at=now-1000;}
   }else{if(have&&r.tx)updates++;phase=UPDATE;last_at=now;first_write=0;}
  }else if(result==2)failure="TX STOP: RADIO REJECTED";
  else if(result<0)failure="TX STOP: STATE CHANGED";
  else if(now-sent_at>3000)failure="TX STOP: ACK TIMEOUT";
 }
 if(t.active)return failure?failure:"TX: WAITING FOR REPLY";
 if(stopping){seen_tx=0;return "TX: STOP COMPLETE";}
 if(!have){failure="TX STOP: NO RADIO STATUS";return failure;}
 if(!r.tx){seen_tx=0;phase=MAP_SPLIT;return 0;}
 if(failure)return failure;
 if(!seen_tx){seen_tx=1;phase=MAP_SPLIT;first_write=1;tx_read_supported=1;}
 int result;
 if(phase==UPDATE){
  if(!gps||!orbit)return "TX HOLD: GPS/TLE";
  if(!profile_target(1,up)){failure="TX STOP: TARGET RANGE";return failure;}
  uint32_t wanted=((up+5)/10)*10;
  if(now-last_at<1000||(!first_write&&wanted==target)){
   snprintf(text,sizeof(text),tx_read_supported?"TX DOPPLER / OK %lu":"TX B DOPPLER / OK %lu",(unsigned long)updates);return text;
  }
  target=wanted;result=civ_owned_up_begin(&t,target);
 }else if(phase==MAP_SPLIT)result=civ_owned_split_read(&t);
 else if(phase==MAP_RX)result=civ_owned_rx_read(&t);
 else if(phase==MAP_UP||phase==VERIFY_UP)result=civ_owned_up_read(&t);
 else result=civ_owned_tx_read(&t);
 if(result){failure="TX STOP: CIV BUSY/UNSUPPORTED";return failure;}
 pending=phase;sent_at=now;return phase==UPDATE?"TX: APPLYING UPLINK":"TX: VERIFYING FREQUENCY";
}
void tx_control_finish(Api*a,uint32_t rx){
 while(t.active&&!failure){
  tx_control_tick(a,0,0,0,1,rx);
  if(t.active&&!failure)a->sleep(50);
 }
}

int tx_control_reset(void){if(t.active)return -1;phase=pending=seen_tx=first_write=0;tx_read_supported=1;sent_at=last_at=rx_sample=up_sample=up_hz=target=updates=0;failure=0;return 0;}
