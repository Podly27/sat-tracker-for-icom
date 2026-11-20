#ifndef ISS_RADIO_MODE_H
#define ISS_RADIO_MODE_H
#include <stdint.h>
/* ICAP returns the packed native mode/filter byte, NOT a CI-V mode number.
 * Native normalizer 2002e534: nibble A -> group 7; CI-V decoder table
 * 202da364 maps CI-V 05 (FM) -> group 7. Nibble 5 is not FM.
 */
static inline int radio_mode_is_fm(uint8_t packed){return (packed>>4)==10;}
#endif
