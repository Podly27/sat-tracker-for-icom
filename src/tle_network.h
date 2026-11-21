#ifndef ISS_TLE_NETWORK_H
#define ISS_TLE_NETWORK_H
#include "icap.h"
/* Experimental transport probe. No radio commands and no SD writes. */
typedef struct {
 char header[1024],line[160],first[70],second[70],chunk[24];
 unsigned header_len,line_len,chunk_len,body_bytes,remaining,chunk_remaining;
 unsigned headers,chunked,chunk_state,length_known,complete,failed,have_first,have_pair;
} TleHttp;
void tle_http_init(TleHttp*);
int tle_http_feed(TleHttp*,const char*,unsigned);
int tle_http_finish(TleHttp*);
int tle_network_supported(const Api*);
typedef struct {int result,error;unsigned elapsed;} TleConnectDiag;
void tle_network_numeric(int enabled);
const TleConnectDiag *tle_network_diagnostics(void);
int tle_network_fetch(Api*,TleHttp*,const char **source);
const char *tle_network_status(void);
#endif
