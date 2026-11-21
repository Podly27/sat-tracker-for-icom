#include "aprs_status.h"
#include <assert.h>
#include <string.h>
volatile uint8_t test_cache[202];static unsigned notifications;
static unsigned stage;
static void trace(unsigned n){assert(n==++stage+1);if(n==3)assert(notifications==2);if(n==4)assert(notifications==3);}
void test_dirty(void){notifications++;}
int main(void){Api a={0};assert(aprs_status_supported(&a));
 for(unsigned i=0;i<202;i++)test_cache[i]=(uint8_t)i;
 uint8_t original[202];for(unsigned i=0;i<202;i++)original[i]=test_cache[i];
 assert(!aprs_status_publish("ISS A105 E+13"));assert(notifications==1);
 assert(!memcmp((const void*)(test_cache+0x2e),"ISS A105 E+13",13));assert(test_cache[0x3b]==13);
 assert(!aprs_status_publish("ISS A110 E+15"));assert(!aprs_status_end_trace(trace));assert(stage==3);
 for(unsigned i=0;i<202;i++)assert(test_cache[i]==original[i]);
 assert(aprs_status_publish("Too long for this slot")<0);
 assert(!aprs_status_publish("ISS A120 E+20"));test_cache[0x2e]='X';assert(aprs_status_end()==-2);assert(test_cache[0x2e]=='X');
 assert(!aprs_status_publish("Waiting for G"));unsigned count=notifications;
 uint8_t before[202];memcpy(before,(const void*)test_cache,202);
 assert(!aprs_status_release());assert(notifications==count);
 for(unsigned i=0;i<202;i++)assert(test_cache[i]==(i>=0x2e&&i<0x3b?' ':before[i]));
 assert(!aprs_status_release());assert(notifications==count);
 assert(!aprs_status_publish("Waiting for G"));test_cache[0x3b]=4;
 memcpy(before,(const void*)test_cache,202);assert(aprs_status_release()==-2);assert(!memcmp(before,(const void*)test_cache,202));
 assert(!aprs_status_publish("Waiting for G"));test_cache[0x2e]='X';
 memcpy(before,(const void*)test_cache,202);assert(aprs_status_release()==-2);assert(!memcmp(before,(const void*)test_cache,202));
 return 0;}
