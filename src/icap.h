#ifndef ISS_ICAP_H
#define ISS_ICAP_H
#include <stdint.h>
typedef struct {uint16_t year;uint8_t month,day,hour,minute,second,weekday;} RadioClock;
/* Reverse engineered ICAP ABI3, 705E140. AAPCS base (softfp).
 * Tracker hardware bindings target v8. Network probe pins v8/v14 separately. */
typedef struct {
 uint32_t abi,radio,build,size;
 void (*print)(const char*);void (*line)(unsigned,const char*);void (*clear)(void);
 int (*open)(const char*,unsigned);int (*read)(int,void*,int);int (*write)(int,const void*,int);int (*close)(int);
 void (*sleep)(unsigned);void (*yield)(void);uint32_t (*ticks)(void);
 int (*memory)(void*);int (*radio_state)(void*);int (*stop)(void);uint32_t (*task_id)(void);
 void *region;uint32_t region_size;int (*clock)(RadioClock*);
 void *reserved54,*reserved58,*reserved5c,*reserved60,*reserved64,*reserved68;
 int (*net_ready)(void);
 int (*net_connect)(const char*,unsigned,unsigned,unsigned);
 int (*net_send)(int,const void*,unsigned,unsigned);
 int (*net_recv)(int,void*,unsigned,unsigned);
 int (*net_close)(int);
 int (*net_error)(void);
 int (*popup)(const char*);
 void *reserved88,*reserved8c;
} Api;
#if UINTPTR_MAX == UINT32_MAX
_Static_assert(__builtin_offsetof(Api,clock)==0x50,"ICAP clock offset");
_Static_assert(__builtin_offsetof(Api,net_connect)==0x70,"ICAP network offset");
_Static_assert(__builtin_offsetof(Api,popup)==0x84,"ICAP popup offset");
#endif
#endif
