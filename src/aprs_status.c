#include "aprs_status.h"
#include <string.h>
#include <stdint.h>
#ifndef ISS_HOST_TEST
#define CACHE ((volatile uint8_t*)0x20393020)
static unsigned lock(void){unsigned c;__asm__ volatile("mrs %0, cpsr\n cpsid i":"=r"(c)::"memory");return c;}
static void unlock(unsigned c){__asm__ volatile("dmb sy\n msr cpsr_c, %0"::"r"(c):"memory");}
static void dirty(void){((void(*)(unsigned))0x200b8f14)(0);}
#else
extern volatile uint8_t test_cache[202];
#define CACHE test_cache
static unsigned lock(void){return 0;}
static void unlock(unsigned c){(void)c;}
extern void test_dirty(void);
static void dirty(void){test_dirty();}
#endif
static uint8_t saved[13],last[13],saved_length,saved_visible,saved_update;
static unsigned active;
int aprs_status_supported(Api*a){
#ifndef ISS_HOST_TEST
 static const uint32_t notification[]={0xe59f1820,0xe0811000,0xe5d12002,0xe3822004};
 static const uint32_t cache_update[]={0xe92d47f0,0xe1a07000,0xe1a06001,0xe1a09002};
 if(!a||a->abi!=3||a->radio!=0x705||a->build!=0x07051405)return 0;
 if((uintptr_t)a->radio_state!=0x2037bac9&&(uintptr_t)a->radio_state!=0x2037bbc1)return 0;
 return !memcmp((const void*)0x200b8f14,notification,sizeof(notification))&&
  !memcmp((const void*)0x2004a208,cache_update,sizeof(cache_update));
#else
 return a!=0;
#endif
}
int aprs_status_publish(const char*s){
 uint8_t text[13];unsigned n=0;while(s[n]&&n<13){if((unsigned char)s[n]<32||(unsigned char)s[n]>126)return -1;text[n]=s[n];n++;}
 if(s[n]||!n)return -1;for(unsigned i=n;i<13;i++)text[i]=' ';
 unsigned c=lock();
 if(active){for(unsigned i=0;i<13;i++)if(CACHE[0x2e + i]!=last[i]){unlock(c);return -2;}}
 else{
  for(unsigned i=0;i<13;i++)saved[i]=CACHE[0x2e + i];saved_length=CACHE[0x3b];saved_visible=CACHE[0]&0x20;saved_update=CACHE[0x51]&8;active=1;
 }
 for(unsigned i=0;i<13;i++){CACHE[0x2e + i]=text[i];last[i]=text[i];}
 CACHE[0x3b]=(uint8_t)n;CACHE[0]|=0x20;CACHE[0x51]|=8;unlock(c);dirty();return 0;
}
int aprs_status_end_trace(void (*trace)(unsigned)){
 if(trace)trace(2);
 if(!active){if(trace)trace(4);return 0;}
 unsigned c=lock();int owned=1;
 for(unsigned i=0;i<13;i++)if(CACHE[0x2e + i]!=last[i])owned=0;
 if(owned){
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
