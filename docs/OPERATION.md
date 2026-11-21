# Operating SAT

Requires IC-705 E with SQ6EMM `705E140_20261119v8`. Other firmware versions are not automatically compatible. Set the clock to UTC and enable internal GPS. Begin in VFO A/B with duplex off.

1. Place `SAT.app` and `SAT.CFG` in `IC-705/Apps/`; keep older tracker executables outside that folder.
2. Connect Wi-Fi for initial orbital downloads, or supply a valid combined `SAT.TLE` for offline use.
3. Start **MENU → SET → Apps → SAT.app**.
4. Startup checks configuration and per-satellite orbit age, refreshes missing/stale data and calculates current/future passes. No radio settings are captured or changed before initial prediction completes.
5. The selected satellite is configured automatically: **VFO A = RX**, **VFO B = TX**, FM, SPLIT and the configured TX tone. Selection follows [configuration priority](CONFIG.md).

## Display

The Apps window shows the satellite name, UTC/GPS status, azimuth from true north, elevation, distance, RX/TX frequencies, pass status and tone. `Waiting for GPS` precedes a valid position. `Waiting for SO-131 DD.MM. HH:MM` gives the next AOS in UTC; `SO-131 pass: N min left` rounds remaining time up. Horizon estimates use 30-second sampling.

The same waiting/pass message scrolls through the native 13-character receiver-status field below the main frequency. The radio adds `RX:` itself. Native status publication pauses during TX/non-FM; it is not a received-signal or successful-contact indicator. The ordinary modulation label remains FM, not SAT. There is no DECODE-area overlay or graphical compass.

A visible satellite wins over a future one. Overlaps use the first enabled configuration row. Otherwise the earliest AOS wins. A pending satellite switch waits until RX; `NAME waits for RX` identifies that destination. Orbital availability, geometric visibility and actual repeater operation are separate things.

## Doppler and transmission

RX control periodically updates both frequencies using owned CI-V request/readback cycles. During operator-controlled TX, only the verified uplink frequency is changed; no VFO/mode/tone/SPLIT configuration changes are made. The app never keys PTT. A/B swap, manual frequency edits or mismatched readback can stop automatic control; STOP and restart establish a new session.

SO-50's separate 74.4 Hz activation remains manual. AO-91's default is no CTCSS and sunlight-only operation; no eclipse filter is implemented. See [profile notes](CONFIG.md).

## EXIT and STOP

**EXIT hides the window and leaves tracking running.** Reopen through Apps.

**STOP ends tracking and restores the original session settings**, including both VFO frequencies, mode/data/filter, CTCSS frequency/on-off, tone squelch, simplex, selected VFO and SPLIT. The snapshot is taken once per session and retained across satellite changes. Settings are re-read to verify restoration. Release PTT and wait for complete app exit before switching off or removing SD.

STOP phases are `Drain radio commands`, `Release status text`, and `Restore radio settings`. Display cleanup blanks only app-owned characters and makes no native redraw calls or stale UI-flag restoration. This retains the 0.22 hardware-confirmed freeze fix. The original received label is not replayed. If restoration cannot complete, check the radio; a crash, held PTT or loss of power can prevent it. The RAM snapshot is not a full radio backup.

Multi-satellite changes in 0.23 have software/ARM validation and still require on-radio verification. The [earlier photograph](images/iss-tracker-radio.png) shows an older single-ISS build, not the current UI.
