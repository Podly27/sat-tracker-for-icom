#define ISS_HOST_TEST 1
#include "../src/radio_session.c"
#include <assert.h>
#include <stdio.h>
static uint32_t time_ms,busy_until;static int selected_vfo=1,published,unsupported,busy_read_only;
static uint32_t state[2][6]={{26300000,0x010005,0x700600,0,0,0},{7122970,0x020001,0x850800,1,1,0}};
int test_selected(void){return selected_vfo;}
static int state_(void*p){*(RadioState*)p=(RadioState){state[selected_vfo][0],0,0xa0,0,0};return 0;}
static uint32_t ticks_(void){return time_ms;}
static int busy(int read){return time_ms<busy_until&&(!busy_read_only||read);}
int civ_setting_read(CivOwned*t,unsigned field){assert(!t->active);if(unsupported)return -2;if(busy(1))return -1;published++;t->active=1;t->read_value=state[selected_vfo][field];return 0;}
int civ_setting_write(CivOwned*t,unsigned field,uint32_t v){assert(!t->active);if(unsupported)return -2;if(busy(0))return -1;published++;t->active=1;if(field==CIV_SELECT)selected_vfo=v;else if(field==CIV_SIMPLEX)state[selected_vfo][CIV_SPLIT]=0;else state[selected_vfo][field]=v;return 0;}
int civ_owned_poll(CivOwned*t){assert(t->active);t->active=0;return 1;}
int main(int argc,char**argv){assert(argc==2);const char*mode=argv[1];Api a={.ticks=ticks_,.radio_state=state_};uint32_t before[2][6];memcpy(before,state,sizeof(state));
 unsupported=!strcmp(mode,"unsupported");busy_until=unsupported?0:!strcmp(mode,"permanent")||!strcmp(mode,"cancel")?0xffffffffu:400;busy_read_only=!strcmp(mode,"read-busy");
 int result=radio_session_capture(&a);
 if(unsupported){assert(result==-1&&!published&&!changed_selection&&strstr(radio_session_status(),"UNSUPPORTED"));return 0;}
 assert(result==0);if(!busy_read_only)assert(!published&&!changed_selection&&selected_vfo==1);
 if(!strcmp(mode,"cancel")){assert(radio_session_restore(&a)==1&&!published&&selected_vfo==1);return 0;}
 unsigned calls=0;while(result==0){assert(++calls<60);time_ms+=100;result=radio_session_capture(&a);}
 if(!strcmp(mode,"permanent")){assert(result==-1&&time_ms==1500&&!published&&!changed_selection&&strstr(radio_session_status(),"CIV BUSY"));assert(radio_session_restore(&a)==1&&!published);return 0;}
 assert(result==1&&selected_vfo==1&&complete&&!memcmp(saved,before,sizeof(saved)));
 if(!strcmp(mode,"restore-busy")){busy_until=0xffffffffu;int count=published;assert(radio_session_restore(&a)==-1&&published==count);return 0;}
 state[0][0]=437800000;state[1][0]=145990000;state[1][CIV_TONE_ON]=0;
 result=0;calls=0;while(!result){assert(++calls<40);result=radio_session_restore(&a);time_ms+=50;}
 assert(result==1&&selected_vfo==1&&!memcmp(state,before,sizeof(state)));return 0;
}
