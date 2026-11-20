#ifndef ISS_GPS_H
#define ISS_GPS_H
#include <stdint.h>
typedef struct {double lat,lon;int fix;} GpsPosition;
enum {GPS_OK=0,GPS_NO_FIX=-1,GPS_BAD_COORDINATES=-2,GPS_UNSTABLE=-3,GPS_UNSUPPORTED=-4};
int gps_coordinate(const uint8_t raw[4],int latitude,double *out);
int gps_read(GpsPosition *out);
#endif
