#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
int tiny_snprintf(char*,size_t,const char*,...);
int main(void){char b[80];
 int n=tiny_snprintf(b,sizeof(b),"FG:%02X M:%02X TX:%u R:%d",0u,0x70u,0u,0);assert(!strcmp(b,"FG:00 M:70 TX:0 R:0")&&n==(int)strlen(b));
 tiny_snprintf(b,sizeof(b),"UI:%02X/%02X/%02X FW:%u",0u,0x33u,256u,8u);assert(!strcmp(b,"UI:00/33/100 FW:8"));
 tiny_snprintf(b,sizeof(b),"%08lx %X %u %d",(unsigned long)0xdeadbeef,0xabcdefu,42u,INT_MIN);assert(!strcmp(b,"deadbeef ABCDEF 42 -2147483648"));
 tiny_snprintf(b,sizeof(b),"%03u %+.1f %s",7u,9.0,"ok");assert(!strcmp(b,"007 +9.0 ok"));
 n=tiny_snprintf(b,5,"%08X",0x1234u);assert(n==8&&!strcmp(b,"0000"));
 assert(tiny_snprintf(0,0,"%02X:%u",0x70u,12u)==5);
 return 0;}
