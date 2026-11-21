#ifndef ISS_PASS_STATUS_H
#define ISS_PASS_STATUS_H
#include <stddef.h>
#include <stdint.h>
/* UTC pass status and fixed 13-character native-display pages. */
void pass_status_format(char*,size_t,const char *satellite,uint32_t now,uint32_t aos,uint32_t los,int in_range,int search_complete);
unsigned pass_status_pages(char pages[2][14],const char*,uint32_t,uint32_t,uint32_t,int,int);
#endif
