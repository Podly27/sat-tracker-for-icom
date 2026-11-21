#include "aprs_status.h"
#include <assert.h>
#include <string.h>
volatile uint8_t test_cache[202];volatile uint16_t test_rx_elapsed,test_scroll_elapsed;volatile uint8_t test_ui_foreground,test_scroll[6],test_rx_offset,test_rx_length;static unsigned notifications;
static unsigned stage;
static void trace(unsigned n){assert(n==++stage+1);if(n==3)assert(notifications==2);if(n==4)assert(notifications==3);}
void test_dirty(void){notifications++;}
int main(void){Api a={0};assert(aprs_status_supported(&a));
 for(unsigned i=0;i<202;i++)test_cache[i]=(uint8_t)i;
 uint8_t original[202];for(unsigned i=0;i<202;i++)original[i]=test_cache[i];
 assert(!aprs_status_publish("ISS A105 E+13"));assert(notifications==1);assert(test_cache[0x50]==0);
 for(unsigned i=0;i<20;i++)assert(test_cache[0x3c+i]==' ');
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
 assert(!aprs_status_publish("WAIT ISS     "));test_cache[0x50]=5;
 memcpy(before,(const void*)test_cache,202);assert(aprs_status_publish("21.11 15:20Z ")==-2);assert(aprs_status_release()==-2);assert(!memcmp(before,(const void*)test_cache,202));
 memset((void*)test_cache,0,202);test_rx_elapsed=321;
 assert(!aprs_status_publish("WAIT ISS     "));assert(test_rx_elapsed==321);assert(test_cache[0]&0x20);
 /* Simulate native acknowledgement and its RX hold before scrolling. */
 test_cache[0]&=~0x20;test_cache[2]=7;test_cache[6]=2;test_cache[4]=0xf4;test_cache[5]=1;
 test_scroll[0]=3;test_scroll[1]=14;test_scroll_elapsed=200;
 for(unsigned i=0;i<8;i++){test_rx_elapsed=501;assert(!aprs_status_publish(i<4?"WAIT ISS     ":"ISS 7m       "));assert(!test_rx_elapsed&&!(test_cache[0]&0x20));assert(test_cache[4]==255&&test_cache[5]==255);assert(test_scroll[0]==1&&!test_scroll[1]&&!test_scroll_elapsed);}
 test_rx_elapsed=123;memcpy(before,(const void*)test_cache,202);count=notifications;
 assert(!aprs_status_release());assert(test_rx_elapsed==123&&notifications==count);
 for(unsigned i=0;i<202;i++)assert(test_cache[i]==(i>=0x2e&&i<0x3b?' ':i==4?0xf4:i==5?1:before[i]));
 assert(!aprs_status_publish("WAIT ISS     "));test_cache[0]&=~0x20;test_cache[4]=test_cache[5]=0;test_rx_elapsed=555;
 assert(!aprs_status_publish("ISS 6m       "));assert(!(test_cache[0]&0x20));assert(!test_rx_elapsed);assert(test_cache[4]==255&&test_cache[5]==255);assert(!aprs_status_release());assert(!test_cache[4]&&!test_cache[5]);
 assert(!aprs_status_publish("WAIT ISS     "));test_cache[0]&=~0x20;test_cache[2]=5;test_cache[4]=0xf4;test_cache[5]=1;test_rx_elapsed=456;
 assert(!aprs_status_publish("ISS 5m       "));assert(test_cache[0]&0x20);assert(test_rx_elapsed==456);assert(!aprs_status_release());
 assert(!aprs_status_publish("WAIT ISS     "));test_cache[2]=7;test_cache[6]=0;test_rx_elapsed=321;
 assert(!aprs_status_publish("ISS 4m       "));assert(test_rx_elapsed==321);assert(!aprs_status_release());
 assert(!aprs_status_publish("WAIT ISS     "));test_cache[2]=7;test_cache[6]=2;test_cache[0x2e]='X';test_rx_elapsed=222;
 assert(aprs_status_publish("ISS 3m       ")==-2);assert(test_rx_elapsed==222);assert(aprs_status_release()==-2);
 test_cache[2]=7;test_cache[3]=2;test_cache[4]=0xf4;test_cache[5]=1;test_cache[0]=0;test_rx_elapsed=123;
 test_scroll[0]=1;test_scroll[1]=3;test_scroll[5]=2;test_rx_offset=2;test_rx_length=13;test_ui_foreground=0;
 assert(!strcmp(aprs_status_debug(0),"N7/2 H500 E123 Q00"));assert(!strcmp(aprs_status_debug(1),"S01/3 P3 B2 O2 L13"));
 test_ui_foreground=0x33;test_scroll[1]=0;test_rx_elapsed=456;
 assert(!strcmp(aprs_status_debug(0),"N7/2 H500 E123 Q00"));assert(!strcmp(aprs_status_debug(1),"S01/3 P3 B2 O2 L13"));
 assert(test_rx_elapsed==456&&test_scroll[1]==0);test_ui_foreground=0;
 assert(!strcmp(aprs_status_debug(0),"N7/2 H500 E456 Q00"));assert(!strcmp(aprs_status_debug(1),"S01/0 P3 B2 O2 L13"));
 return 0;}
