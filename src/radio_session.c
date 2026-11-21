#include "radio_session.h"
#include "civ_queue.h"
#include "tuner.h"
#include <string.h>
/* Native ICAP callback indexes VFO cache by this byte, with >1 denoting
 * unsupported context. Pin callback instructions/literals before reading.
 */
#ifdef ISS_HOST_TEST
extern int test_selected(void);
static int selected(void){return test_selected();}
#else
static int selected(void){
 const uint16_t *p=(const uint16_t*)0x2037bac8;
 if(p[0]!=0xb510||p[1]!=0x210c||p[2]!=0x4604||
    *(const uint32_t*)0x2037bb1c!=0x20380600||
    *(const uint32_t*)0x2037bb24!=0x203ebc2e)return -1;
 unsigned n=*(volatile const uint8_t*)0x203806f9;return n<=1?(int)n:-1;
}
#endif
static CivOwned request;
static uint32_t saved[2][6],at,busy_since;
static unsigned original,phase_index,waiting,complete,failed,restoring,changed_selection,busy_wait;
static const char*message="SAVE: NOT STARTED";
const char *radio_session_status(void){return message;}
int radio_session_saved(void){return complete;}
/* The only selections made before commit are reversible A/B selections.
 * Initial DUP operation is refused because 0F cannot independently report
 * both hidden split state and duplex in that context.
 */
static int wait_reply(Api*a){
 int result=civ_owned_poll(&request);
 if(!result&&a->ticks()-at<=3000)return 0;
 waiting=0;
 if(result!=1){failed=1;message="RESTORE/SAVE: CIV ERROR";return -1;}
 return 1;
}
static int submit(Api*a,unsigned field,uint32_t value,int read){
 int r=read?civ_setting_read(&request,field):civ_setting_write(&request,field,value);
 if(r){
  /* A transient foreign request must not poison the whole startup snapshot.
   * Do not clear/release any transport state which we do not own. */
  if(r==-1&&!restoring){
   if(!busy_wait){busy_wait=1;busy_since=a->ticks();}
   if(a->ticks()-busy_since<1500){message="SAVE: WAIT CI-V";return 0;}
  }
  failed=1;message=r==-1?"RESTORE/SAVE: CIV BUSY":"RESTORE/SAVE: UNSUPPORTED";return -1;
 }
 busy_wait=0;
 if(!read&&field==CIV_SELECT&&!restoring)changed_selection=1;
 waiting=1;at=a->ticks();return 0;
}
static int in_rx(Api*a){RadioState r={0};return a->radio_state(&r)==0&&!r.tx;}
int radio_session_capture(Api*a){
 if(complete)return 1;
 if(failed)return -1;
 if(!in_rx(a)){message="SAVE: WAIT FOR RX";return 0;}
 if(!phase_index&&!waiting&&!busy_wait){int n=selected();if(n<0){failed=1;message="SAVE: USE VFO A/B";return -1;}original=n;}
 if(waiting){int r=wait_reply(a);if(r<=0)return r;
  if(phase_index%7){unsigned v=phase_index/7,f=phase_index%7-1;saved[v][f]=request.read_value;}
  phase_index++;
 }
 if(phase_index<14){unsigned v=phase_index/7,f=phase_index%7;
  message="SAVING ORIGINAL SETTINGS";
  if(!f)return submit(a,CIV_SELECT,v,0);
  return submit(a,f-1,0,1);
 }
 if(phase_index==14){message="SAVE: RETURNING VFO";return submit(a,CIV_SELECT,original,0);}
 changed_selection=0;
 if(saved[0][5]>1||saved[1][5]>1||saved[0][5]!=saved[1][5]){
  failed=1;message="SAVE: USE SIMPLEX VFO";return -1;
 }
 complete=1;message="ORIGINAL SETTINGS SAVED";return 1;
}
int radio_session_restore(Api*a){
 if(request.active){int r=wait_reply(a);if(r<=0)return r;
  if(restoring){
   if(phase_index>=16&&phase_index<30&&(phase_index-16)%7){
    unsigned v=(phase_index-16)/7,f=(phase_index-16)%7-1;
    if(request.read_value!=saved[v][f]){failed=1;message="RESTORE: READBACK MISMATCH";return -1;}
   }
   phase_index++;
  }
 }
 if(!in_rx(a)){message="RESTORE: RELEASE PTT";return 0;}
 if(!complete){
  if(!changed_selection)return 1;
  if(failed&&request.active)return -1;
  failed=0;changed_selection=0;restoring=1;phase_index=100;
  return submit(a,CIV_SELECT,original,0);
 }
 if(!restoring){if(request.active)return -1;restoring=1;failed=0;phase_index=0;}
 if(failed)return -1;
 if(phase_index>=100)return 1;
 /* Mode first, then frequency (mode changes may quantize frequency).
  * Restore tone/TSQL only after the original frequency is back.
  */
 static const unsigned order[]={CIV_MODE,CIV_FREQ,CIV_TONE,CIV_TONE_ON,CIV_TSQL,CIV_SIMPLEX};
 if(phase_index<14){unsigned v=phase_index/7,k=phase_index%7;
  message="RESTORING ORIGINAL SETTINGS";
  return k?submit(a,order[k-1],order[k-1]==CIV_SIMPLEX?0:saved[v][order[k-1]],0):submit(a,CIV_SELECT,v,0);
 }
 if(phase_index==14)return submit(a,CIV_SELECT,original,0);
 if(phase_index==15)return submit(a,CIV_SPLIT,saved[0][5],0);
 /* Verify both VFOs through canonical reads, including tone and split. */
 if(phase_index<30){unsigned v=(phase_index-16)/7,k=(phase_index-16)%7;
  if(waiting)return 0;
  message="VERIFYING RESTORED SETTINGS";
  return k?submit(a,k-1,0,1):submit(a,CIV_SELECT,v,0);
 }
 if(phase_index==30)return submit(a,CIV_SELECT,original,0);
 message="ORIGINAL SETTINGS RESTORED";return 1;
}
