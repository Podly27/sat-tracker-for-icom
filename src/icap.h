#ifndef ISS_ICAP_H
#define ISS_ICAP_H
#include <stdint.h>
typedef struct {uint16_t year;uint8_t month,day,hour,minute,second,weekday;} RadioClock;
/* Reverse engineered only for 705E140_20261119v8. AAPCS base (softfp). */
typedef struct {
 uint32_t abi,radio,build,size;
 void (*print)(const char*);void (*line)(unsigned,const char*);void (*clear)(void);
 int (*open)(const char*,unsigned);int (*read)(int,void*,int);int (*write)(int,const void*,int);int (*close)(int);
 void (*sleep)(unsigned);void (*yield)(void);uint32_t (*ticks)(void);
 int (*memory)(void*);int (*radio_state)(void*);int (*stop)(void);uint32_t (*task_id)(void);
 void *region;uint32_t region_size;int (*clock)(RadioClock*);
} Api;
#if UINTPTR_MAX == UINT32_MAX
_Static_assert(__builtin_offsetof(Api,clock)==0x50,"ICAP clock offset");
#endif
#endif
