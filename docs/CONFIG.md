# SAT.CFG: satellites and priority

Edit `IC-705/Apps/SAT.CFG` as plain text while the app is stopped. Restart SAT after editing. Rows are ordered from highest to lowest priority. Prefix a row with `#` to disable it.

```text
# NAME,NORAD,RX_MHz,TX_MHz,CTCSS_Hz
ISS,25544,437.800,145.990,67.0
SO-50,27607,436.795,145.850,67.0
AO-91,43017,145.960,435.250,0
AO-123,61781,435.400,145.850,67.0
SO-131,101101,436.888,145.925,0
```

`RX_MHz` is the satellite downlink; `TX_MHz` is its uplink. The last field is the transmit CTCSS tone in Hz; **0 switches it off**. The receiver uses no tone squelch. These are nominal frequencies; SAT calculates Doppler corrections separately.

## Selection

- If a configured satellite is currently above the geometric horizon, it wins over a future pass. If several are above the horizon, the first enabled row wins.
- If none is above the horizon, the earliest predicted AOS wins. Equal AOS times are resolved by row order. A later high-priority pass does not hide an earlier pass of another satellite.
- A higher-priority satellite can preempt a lower-priority one when its pass begins. Changing the selected satellite is deferred while PTT is held; the existing uplink remains under its current controller until RX resumes.
- Ended passes are recalculated individually, preserving predictions for other satellites. Major time/location changes and hourly refreshes recalculate the full set. Searching is cooperative, not one long uninterruptible loop.
- Missing, invalid or older-than-seven-day orbital records are excluded. Startup reports how many configured satellites have usable data. Visibility predicts geometry, not signal strength or repeater availability.

Both the Apps window and the scrolling native field below the frequency identify the selected satellite. VFO A is always RX and B is always TX, including AO-91's reversed U/V bands. The radio's own `RX:` label remains. The original radio settings are captured once per app session and restored on STOP, not recaptured after each satellite change.

## Add another satellite

Copy an enabled row, replace all five values with verified information, and put it at the desired priority. Up to **8 enabled profiles** are supported. Names must contain 1–6 uppercase letters, digits or hyphens and be unique. Catalog numbers must be unique integers from 1 to 339999. Supported profiles are FM cross-band 144–148 MHz / 430–440 MHz, with an 80–110 minute low-Earth orbit. Linear transponders and other bands are not supported. Use an IC-705-supported CTCSS frequency or 0. Invalid/empty configurations stop startup before tuning; if the file is absent, the original ISS profile is used.

The app downloads orbital data for newly enabled profiles at startup if Wi-Fi is available. For offline use, regenerate the combined cache with [the update tool](TLE.md). Do not substitute another satellite's orbital elements or edit its epoch.

## Operating notes and sources

- **SO-50:** normal access uses 67 Hz, but its timer needs a separate two-second 74.4 Hz activation carrier. This remains operator-managed: SAT never keys PTT or sends an activation carrier. If activation is needed, STOP SAT first, configure the radio for the 145.850 MHz uplink with 74.4 Hz and transmit the activation yourself for two seconds. Release PTT and restart SAT; it configures normal 67 Hz access. Do not manually switch VFOs under the running tracker just to change this tone. [AMSAT SO-50](https://www.amsat.org/two-way-satellites/so-50-satellite-information/).
- **AO-91:** the current carrier-operated repeater needs **no CTCSS**, so its default is 0 rather than the older 67 Hz profile. Operate only in sunlight; SAT does not calculate eclipse eligibility. [AMSAT AO-91](https://www.amsat.org/two-way-satellites/ao-91/).
- **AO-123:** the FM voice profile is 145.850 / 435.400 MHz with 67 Hz; other listed frequencies may refer to telemetry or other modes. [AMSAT FM satellites](https://www.amsat.org/live-fm-satellites/).
- **SO-131 / UNNE-1B:** 145.925 / 436.888 MHz, without a subtone. It uses the six-digit NORAD identifier 101101 and CelesTrak OMM/CSV orbital input. [AMSAT-EA projects](https://www.amsat-ea.org/proyectos/), [CelesTrak query](https://celestrak.org/NORAD/elements/gp.php?CATNR=101101&FORMAT=CSV).

Profiles are a configuration snapshot, not live repeater-status monitoring. Check operator announcements before transmitting.
