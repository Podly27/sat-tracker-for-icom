# SAT.CFG

Edit `IC-705/Apps/SAT.CFG` as plain text while SAT is stopped, then restart it.

```text
# NAME,NORAD,RX_MHz,TX_MHz,CTCSS_Hz
ISS,25544,437.800,145.990,67.0
SO-50,27607,436.795,145.850,67.0
AO-91,43017,145.960,435.250,0
AO-123,61781,435.400,145.850,67.0
SO-131,101101,436.888,145.925,0
```

- `RX_MHz` is the downlink; `TX_MHz` is the uplink. SAT calculates Doppler corrections from these nominal frequencies.
- `CTCSS_Hz` sets the TX tone; **0 disables it**. RX tone squelch is disabled.
- Prefix a row with `#` to disable it. Add a satellite by copying a row and entering verified values. Maximum eight enabled profiles; names use 1–6 uppercase letters, digits or hyphens. Only 80–110 minute LEO orbits and cross-band FM between 144–148 and 430–440 MHz are supported.
- Visible satellites take priority over future passes. Overlapping passes use the first enabled row; otherwise the earliest predicted pass wins. Switching satellites waits until RX.

**SO-50:** normal access uses 67 Hz. Its separate 74.4 Hz/two-second activation carrier is operator-managed: STOP SAT before configuring/transmitting it manually, release PTT, then restart SAT. SAT does not transmit an activation carrier.

**AO-91:** this profile uses no TX tone; operate only in sunlight. Predictions are geometric and do not filter eclipses or confirm repeater availability. Check current satellite operating information before use.
