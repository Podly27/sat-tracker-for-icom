#include "aprs_status.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#ifndef ISS_HOST_TEST
#define CACHE ((volatile uint8_t*)0x20393020)
#define RX_ELAPSED (*(volatile uint16_t*)0x20381324)
static unsigned lock(void){unsigned c;__asm__ volatile("mrs %0, cpsr\n cpsid i":"=r"(c)::"memory");return c;}
static void unlock(unsigned c){__asm__ volatile("dmb sy\n msr cpsr_c, %0"::"r"(c):"memory");}
static void dirty(void){((void(*)(unsigned))0x200b8f14)(0);}
#else
extern volatile uint8_t test_cache[202];
#define CACHE test_cache
extern volatile uint16_t test_rx_elapsed;
#define RX_ELAPSED test_rx_elapsed
static unsigned lock(void){return 0;}
static void unlock(unsigned c){(void)c;}
extern void test_dirty(void);
static void dirty(void){test_dirty();}
#endif
static uint8_t saved[13],last[13],saved_tail[20],saved_tail_length,saved_length,saved_visible,saved_update;
static unsigned active;
int aprs_status_supported(Api*a){
#ifndef ISS_HOST_TEST
 static const uint32_t notification[]={0xe59f1820,0xe0811000,0xe5d12002,0xe3822004};
 static const uint32_t cache_update[]={0xe92d47f0,0xe1a07000,0xe1a06001,0xe1a09002};
 static const uint32_t hold_setup[]={0xe92d4010,0xe3a02065,0xe0030291,0xe51f21f0};
 static const uint32_t hold_check[]={0xe1d400b4,0xe3500000,0x0a000004,0xe1d910b0};
 if(!a||a->abi!=3||a->radio!=0x705||a->build!=0x07051405)return 0;
 if((uintptr_t)a->radio_state!=0x2037bac9&&(uintptr_t)a->radio_state!=0x2037bbc1)return 0;
 return !memcmp((const void*)0x200b8f14,notification,sizeof(notification))&&
  !memcmp((const void*)0x2004a208,cache_update,sizeof(cache_update))&&
  !memcmp((const void*)0x2004b050,hold_setup,sizeof(hold_setup))&&
  !memcmp((const void*)0x2004b5a4,hold_check,sizeof(hold_check))&&
  *(const uint32_t*)0x2004ae8c==0x20381324;
#else
 return a!=0;
#endif
}
int aprs_status_publish(const char*s){
 uint8_t text[13];unsigned n=0;while(s[n]&&n<13){if((unsigned char)s[n]<32||(unsigned char)s[n]>126)return -1;text[n]=s[n];n++;}
 if(s[n]||!n)return -1;for(unsigned i=n;i<13;i++)text[i]=' ';
 unsigned c=lock();
 if(active&&CACHE[0x50]){unlock(c);return -2;}
 if(active){for(unsigned i=0;i<13;i++)if(CACHE[0x2e + i]!=last[i]){unlock(c);return -2;}}
 int held=active&&CACHE[2]==7&&(CACHE[6]&2)&&(CACHE[4]||CACHE[5]);
 if(!active){
  for(unsigned i=0;i<20;i++)saved_tail[i]=CACHE[0x3c + i];saved_tail_length=CACHE[0x50];
  for(unsigned i=0;i<13;i++)saved[i]=CACHE[0x2e + i];saved_length=CACHE[0x3b];saved_visible=CACHE[0]&0x20;saved_update=CACHE[0x51]&8;active=1;
 }
 for(unsigned i=0;i<13;i++){CACHE[0x2e + i]=text[i];last[i]=text[i];}
 /* Native RX text concatenates the 13-byte name and this 20-byte tail.
  * A stale tail would extend SAT pages and trigger the native scroller. */
 for(unsigned i=0;i<20;i++)CACHE[0x3c + i]=' ';CACHE[0x50]=0;
 CACHE[0x3b]=(uint8_t)n;
 /* In pinned v8 state 7 is the RX name, bit 1 selects its RX variant,
  * and a nonzero hold precedes scrolling. Renew only that owned hold;
  * don't enqueue another message lifecycle while the row is held. */
 if(held)RX_ELAPSED=0;else CACHE[0]|=0x20;
 CACHE[0x51]|=8;unlock(c);dirty();return 0;
}
int aprs_status_end_trace(void (*trace)(unsigned)){
 if(trace)trace(2);
 if(!active){if(trace)trace(4);return 0;}
 unsigned c=lock();int owned=CACHE[0x50]==0;
 for(unsigned i=0;i<13;i++)if(CACHE[0x2e + i]!=last[i])owned=0;
 if(owned){
  for(unsigned i=0;i<20;i++)CACHE[0x3c + i]=saved_tail[i];CACHE[0x50]=saved_tail_length;
  for(unsigned i=0;i<13;i++)CACHE[0x2e + i]=saved[i];CACHE[0x3b]=saved_length;
  CACHE[0]=(CACHE[0]&~0x20)|saved_visible;CACHE[0x51]=(CACHE[0x51]&~8)|saved_update;
 }
 active=0;unlock(c);
 if(trace)trace(3);
 if(owned)dirty();
 if(trace)trace(4);
 return owned?0:-2;
}

int aprs_status_end(void){return aprs_status_end_trace(0);}

/* The loader owns repaint while stopping an app. Do not call a native UI
 * notification or replay a stale snapshot of its visibility/update flags.
 * Blank only our characters; leave newer receiver content untouched. */
int aprs_status_release(void){
 if(!active)return 0;
 unsigned c=lock();int owned=CACHE[0x3b]==13&&CACHE[0x50]==0;
 for(unsigned i=0;i<13;i++)if(CACHE[0x2e + i]!=last[i])owned=0;
 if(owned)for(unsigned i=0;i<13;i++)CACHE[0x2e + i]=' ';
 active=0;unlock(c);return owned?0:-2;
}

/* Observe the real main-screen state without changing its scroll counters.
 * Keep the last main-screen sample visible when Apps is opened. */
const char *aprs_status_debug(unsigned row){
 static char info[2][32];static unsigned peak;
#ifndef ISS_HOST_TEST
 unsigned foreground=*(volatile uint8_t*)0x204b035d;
 volatile uint8_t *scroll=(volatile uint8_t*)0x203930fa;
 unsigned offset=*(volatile uint8_t*)0x204bac93,length=*(volatile uint8_t*)0x204c0371;
#else
 extern volatile uint8_t test_ui_foreground,test_scroll[6],test_rx_offset,test_rx_length;
 unsigned foreground=test_ui_foreground;volatile uint8_t *scroll=test_scroll;
 unsigned offset=test_rx_offset,length=test_rx_length;
#endif
 if(!row&&foreground==0){
  unsigned c=lock();unsigned state=CACHE[2],style=CACHE[3],hold=CACHE[4]|(CACHE[5]<<8),elapsed=RX_ELAPSED,q=CACHE[0];
  unsigned flags=scroll[0],index=scroll[1],other=scroll[5];unlock(c);
  if(index>peak)peak=index;
  snprintf(info[0],32,"N%u/%u H%u E%u Q%02X",state,style,hold,elapsed,q);
  snprintf(info[1],32,"S%02X/%u P%u B%u O%u L%u",flags,index,peak,other,offset,length);
 }
 return info[row?1:0][0]?info[row?1:0]:"UI: hide app for 10 seconds";
}
