#include "../src/catalog.c"
int tle_network_supported(const Api*a){(void)a;return 0;}
int tle_network_fetch_sat(Api*a,TleHttp*p,const char**s,uint32_t n,const char*i,int c,int(*r)(const char*,void*),void*x){(void)a;(void)p;(void)s;(void)n;(void)i;(void)c;(void)r;(void)x;return -1;}
/* Oracle-test bindings accept source records rather than re-encoded elements. */
int test_csv_orbit(Orbit*o,const char*s,uint32_t n){return csv_orbit(o,s,n);}
int test_parse_number(const char*s,double*v){return number(s,v);}
