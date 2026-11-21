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

## Startup sequence (0.16.0)

The startup screen validates ISS.TLE against its orbital epoch and the radio UTC clock, waits for internal GPS, then calculates the next AOS/LOS before entering the receive screen. TLE older than 24 hours is flagged; validated data within the existing seven-day limit can still be used offline. Missing, invalid or expired elements prevent tuning. STOP can cancel startup without changing radio settings.

**Automatic in-radio updates are not enabled in the tracker yet.** Analysis of the extended ICAP tables found socket callbacks in both v8 and v14; our earlier claim that v8 had no network API was incorrect. The separate experimental `TLECHECK.app` probes HTTPS without tuning or SD writes. See [network and native display research](NETWORK-AND-DISPLAY.md). Until device verification is complete, update the card with `tools/update_tle.py` on a computer.
