#ifndef ISS_PASS_STATUS_H
#define ISS_PASS_STATUS_H
#include <stddef.h>
#include <stdint.h>
/* UTC pass status. The caller selects the satellite; no CFG selection yet. */
void pass_status_format(char*,size_t,const char *satellite,uint32_t now,uint32_t aos,uint32_t los,int in_range,int search_complete);
#endif
