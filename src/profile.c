#include "profile.h"
uint32_t profile_rx=437800000,profile_tx=145990000;unsigned profile_tone=670;
static int band(uint32_t hz){return hz>=144000000&&hz<=148000000?1:hz>=430000000&&hz<=440000000?2:0;}
int profile_valid(uint32_t rx,uint32_t tx,unsigned tone){
 static const unsigned short tones[]={0,670,693,719,744,770,797,825,854,885,915,948,974,1000,1035,1072,1109,1148,1188,1230,1273,1318,1365,1413,1462,1514,1567,1598,1622,1655,1679,1713,1738,1773,1799,1835,1862,1899,1928,1966,1995,2035,2065,2107,2181,2257,2291,2336,2418,2503,2541};
 if(!band(rx)||!band(tx)||band(rx)==band(tx))return 0;
 for(unsigned i=0;i<sizeof(tones)/sizeof(*tones);i++)if(tone==tones[i])return 1;return 0;
}
void profile_set(uint32_t rx,uint32_t tx,unsigned tone){profile_rx=rx;profile_tx=tx;profile_tone=tone;}
int profile_target(unsigned up,uint32_t hz){uint32_t nominal=up?profile_tx:profile_rx;return band(hz)==band(nominal)&&hz+30000>=nominal&&hz<=nominal+30000;}
