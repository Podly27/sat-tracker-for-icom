# Operating the standalone application

## Start

Use the supported SQ6EMM v8 firmware. Set the radio clock to UTC, enable the internal GPS and obtain a valid fix. Start from VFO A or B with DUP+ and DUP− off. The app refuses an unsupported starting context rather than guessing the original state.

Choose `MENU → SET → Apps → ISS.app`. Its displayed name is `ISS tracker`. Give it time to capture the original settings, establish GPS/time/TLE validity and set the profile; avoid changing settings during initialization.

The tracker sets A/B to FM, RX A around 437.800 MHz and TX B around 145.990 MHz, CTCSS 67.0 Hz on the uplink, TSQL off, simplex on both VFOs and SPLIT on. These are automatic radio configuration changes; it does not key PTT.

## Display

The [radio photograph](images/iss-tracker-radio.png) shows an actual ISS pass on an earlier build labelled `SATELITE`. The current app is named `ISS tracker`. The photographed values are historical, not live telemetry.

- **BELOW / IN RANGE:** calculated elevation below/above the geometric horizon. This is not a received-signal indicator or a guarantee that the repeater is on.
- **AZ:** direction from true north, clockwise in degrees. **EL:** elevation in degrees; negative values are below the horizon.
- **km:** slant range to ISS, not ground-track distance.
- **RX A / TX B:** corrected frequencies. Startup can show computed targets; after controller/readback activity the fields use the app's last confirmed values. They are not an RF counter measurement.
- **AOS / LOS:** sampled estimates for the next rise or current set, in UTC. The app initially shows search progress.
- **GPS:** a fix is required. When GPS or time/TLE validity is lost, new tuning commands are paused; this does not constitute a hardware transmit inhibit.

Observer coordinates and ACK/debug counters are intentionally hidden in this version. Operational errors can still be shown. The ordinary radio screen continues to say FM; this release does not provide a native SAT mode label or DECODE-area overlay.

## Doppler and operator-controlled TX

During RX the app periodically corrects both directions using an owned CI-V transaction/readback cycle. RX cycles are rate-limited with a minimum two-second interval after a completed cycle, plus transaction latency.

While the operator transmits, the controller verifies the selected/unselected VFO mapping and SPLIT, then only updates the uplink frequency. These writes are limited to at most one per second plus readback latency. It does not change VFO selection, tones, modes or SPLIT during active TX, and never sends a PTT command.

A supported TX-frequency read is used when available. Some firmware rejects it; then the app relies on B-VFO readback. That fallback does not independently prove that the emitted RF changed. A/B swap, manual edits or unexpected readback can stop control. Use STOP and restart the app to establish a fresh session.

## EXIT, STOP and restoration

**EXIT hides the window and keeps the app running.** Reopen it through Apps.

**STOP in Apps ends the session.** Release PTT and wait for complete exit before removing the card or switching power. Restoration returns:

- Both VFO frequencies and mode/data/filter values.
- CTCSS frequency, tone enable and tone-squelch state on each VFO.
- Simplex, the originally selected VFO and SPLIT.

The app re-reads settings to verify restoration. It waits for RX rather than restoring settings during TX. This is bounded; if restoration fails or PTT remains held, an incomplete-restore message is shown. Check the radio instead of treating that as success.

The snapshot is volatile RAM. Removing power, a crash or interrupted shutdown can prevent restoration; the app does not provide a full radio backup or persistent recovery mechanism.

## Startup sequence (0.18.0)

Startup checks radio UTC and the epochs in `ISS.TLE`, `ISS.BAK` and the legacy `SATELITE.TLE`. It selects the usable candidate closest to radio UTC. If none is usable, or the selected elements are more than 24 hours from UTC, it tries a hostname-based HTTPS update. Valid elements already within 24 hours skip the network request.

Downloaded elements must pass checksums, NORAD identity, orbit initialization and a 24-hour epoch limit. The app saves them into the alternate `ISS.TLE` / `ISS.BAK` slot and verifies the bytes by reopening it. The selected valid file is never opened for writing. There is no rename/fsync API, so this is a recoverable two-slot cache, not a claim of atomic FAT updates or immunity to card/power failure. If saving fails, verified downloaded data can still be used in RAM for that session.

When networking fails, a validated cached orbit within seven days can be used offline. If no usable orbit exists, no radio control starts. UTC changes or an expired cache pause control; restart to perform a new startup download attempt. Online checks run at startup only.

After loading TLE, the app waits for internal GPS and computes the next AOS/LOS before starting its existing radio-control session. STOP during startup cancels before radio settings are captured or changed. The integrated update/save path has host and simulated ARM validation; it still needs a device run. The separate DNS and numeric-IP HTTPS probes have both downloaded and validated a fresh ISS TLE on the owner's IC-705.
