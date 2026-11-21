# Operating SAT

Requires IC-705 E with SQ6EMM `705E140_20261119v8`. Other firmware versions are not automatically compatible. Set the clock to UTC and enable internal GPS. Begin in VFO A/B with duplex off.

1. Place `SAT.app` and `SAT.CFG` in `IC-705/Apps/`; keep older tracker executables outside that folder.
2. Connect Wi-Fi for initial orbital downloads, or supply a valid combined `SAT.TLE` for offline use.
3. Start **MENU → SET → Apps → SAT.app**.
4. Startup checks configuration and per-satellite orbit age, refreshes missing/stale data and calculates current/future passes. No radio settings are captured or changed before initial prediction completes.
5. The selected satellite is configured automatically: **VFO A = RX**, **VFO B = TX**, FM, SPLIT and the configured TX tone. Selection follows [configuration priority](CONFIG.md).

## Display

The Apps window shows the satellite name, UTC/GPS status, azimuth from true north, elevation, distance, RX/TX frequencies, pass status and tone. `Waiting for GPS` precedes a valid position. `Waiting for SO-131 DD.MM. HH:MM` gives the next AOS in UTC; `SO-131 pass: M:SS left` shows remaining minutes and seconds. Horizon estimates use 30-second sampling.

The native receiver-status field below the main frequency shows one message of at most 13 characters, refreshed every 500 ms without alternating pages. Examples: `WAIT SO-131`, `SO-131 7:15`, or `WAIT FOR GPS`. The visible-pass message gives remaining minutes and seconds (`--:--` until LOS is known, `0:00` at the predicted end). Full date/time details stay in the Apps window. The radio adds `RX:` itself. Native status publication pauses during TX/non-FM; it is not a received-signal or successful-contact indicator. The ordinary modulation label remains FM. There is no DECODE-area overlay or graphical compass.

A visible satellite wins over a future one. Overlaps use the first enabled configuration row. Otherwise the earliest AOS wins. A pending satellite switch waits until RX; the Apps window identifies that destination. Orbital availability, geometric visibility and actual repeater operation are separate things.

## Doppler and transmission

RX control periodically updates both frequencies using owned CI-V request/readback cycles. During operator-controlled TX, only the verified uplink frequency is changed; no VFO/mode/tone/SPLIT configuration changes are made. The app never keys PTT. A/B swap, manual frequency edits or mismatched readback can stop automatic control; STOP and restart establish a new session.

SO-50's separate 74.4 Hz activation remains manual. AO-91's default is no CTCSS and sunlight-only operation; no eclipse filter is implemented. See [profile notes](CONFIG.md).

## Startup CI-V waiting

A transient busy reply during the original-setting snapshot is retried for at most 1.5 seconds for that command. The app does not clear native queue bytes, discard unknown replies or force UART idle. During the snapshot, the native field says `SAVE RADIO` or `CI-V WAIT`; normal pass status starts only after capture succeeds. A sustained busy state blocks tuning and produces `SAVE FAILED`. The Apps window retains the full error and a compact read-only transport snapshot: `Q` is queued request, `R` reply-ready, `C` parser count and `U` the nine UART-state bytes in hexadecimal. Unsupported bindings are a separate error. Restore behavior does not acquire these startup retries or extra STOP delays.

## EXIT and STOP

**EXIT hides the window and leaves tracking running.** Reopen through Apps.

**STOP ends tracking and restores the original session settings**, including both VFO frequencies, mode/data/filter, CTCSS frequency/on-off, tone squelch, simplex, selected VFO and SPLIT. The snapshot is taken once per session and retained across satellite changes. Settings are re-read to verify restoration. Release PTT and wait for complete app exit before switching off or removing SD.

STOP phases are `Drain radio commands`, `Release status text`, and `Restore radio settings`. Display cleanup blanks only app-owned characters and makes no native redraw calls or stale UI-flag restoration. This retains the 0.22 hardware-confirmed freeze fix. The original received label is not replayed. If restoration cannot complete, check the radio; a crash, held PTT or loss of power can prevent it. The RAM snapshot is not a full radio backup.

Multi-satellite changes in 0.23 have software/ARM validation and still require on-radio verification. The [earlier photograph](images/iss-tracker-radio.png) shows an older single-ISS build, not the current UI.

Version 0.25 is a diagnostic candidate for the reported first-start busy condition. If capture fails, it stores read-only transport snapshots in `SATDBG.LOG` and shows the SAVE phase. Return the card after STOP/power-off for inspection. See [CI-V diagnostics](CI-V-DEBUG.md). It does not clear an unknown busy path.

## UI diagnostic builds 0.31–0.33

Automatic scrolling still occurs on the owner's radio with 0.30; disabling it is not yet verified. These builds temporarily replaces the last two normal Apps rows (tone/priority and blank status) with read-only native UI diagnostics when no radio error is present. Radio errors take precedence. Versions 0.31–0.32 retain the 0.30 scrolling behavior. Version 0.33 is a further fixed-row candidate: it adopts an expired RX hold too, clears the owned scroll group's completion/position once, and extends that hold while active. On STOP it returns that owned hold to its prior duration without a native UI notification. No extra SD logs are written.

Hide SAT with EXIT, leave the normal FM/RX screen visible for at least ten seconds while the unwanted motion happens, then reopen the running SAT app and photograph both bottom diagnostic rows. They retain the latest sample from the main screen. `N` is native state/style, `H` its hold duration, `E` its elapsed counter and `Q` its pending-event flags. `S` is native scroll-group 4 flags/index, `P` its peak sampled index, `B` group 5 index, `O` the rendered output offset and `L` the native output-length byte. These raw values are observations, not a diagnosis. Normal tracking/STOP remain available.
