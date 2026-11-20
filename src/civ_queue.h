#ifndef ISS_CIV_QUEUE_H
#define ISS_CIV_QUEUE_H
#include <stdint.h>
const char *civ_busy_status(void);
int civ_read_probe_begin(void);
int civ_read_probe_pending(void);
int civ_iss_frequency(unsigned unselected,uint32_t hz);
int civ_iss_profile(unsigned step);
/* Experimental internal controller: only bounded selected-RX frequency writes.
 * Release requires our exact pending request, untouched transport snapshot,
 * and an E0/A4 FB-or-FA/FD response. No arbitrary reset/busy override.
 */
typedef struct {
 uint8_t active,length,kind,uart_before[9],counter_before[6],payload[7];
 uint32_t read_value;
} CivOwned;
int civ_owned_rx_begin(CivOwned*,uint32_t hz);
int civ_owned_up_begin(CivOwned*,uint32_t hz);
int civ_owned_profile_begin(CivOwned*,unsigned step);
int civ_owned_up_read(CivOwned*);
int civ_owned_rx_read(CivOwned*);
int civ_owned_mode_read(CivOwned*);
/* Read-only 1C 03, never 1C 00 (PTT). */
int civ_owned_tx_read(CivOwned*);
int civ_owned_split_read(CivOwned*);
enum {CIV_FREQ,CIV_MODE,CIV_TONE,CIV_TONE_ON,CIV_TSQL,CIV_SPLIT,CIV_SELECT,CIV_SIMPLEX};
/* Session snapshot/restore only. Fixed typed commands, never arbitrary CI-V. */
int civ_setting_read(CivOwned*,unsigned field);
int civ_setting_write(CivOwned*,unsigned field,uint32_t captured_value);
/* 0 pending, 1 ACK/released, 2 NAK/released, -3 ownership mismatch (no writes). */
int civ_owned_poll(CivOwned*);
#endif
