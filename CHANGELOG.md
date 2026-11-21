# Changelog

## 0.17.0-probe — 2026-11-21 (experimental)

- Identify extended socket callbacks in v8 and v14, correcting the earlier networking limitation claim.
- Add isolated TLECHECK HTTPS probe with pinned interfaces, bounded HTTP/TLE parsing and cleanup.
- Document APRS native receive-message UI and the need for a separate satellite display bridge.
- Host tests passed; no radio validation or automatic tracker integration claimed.

## 0.16.0 — 2026-11-20

- Add startup TLE epoch/UTC age check, GPS wait, and next AOS/LOS calculation before radio control begins.
- Warn above 24 hours and preserve the seven-day hard limit; STOP during startup leaves the radio untouched.
- Initially reported no network API. Subsequent analysis corrected this: the extended v8/v14 ICAP tables contain socket callbacks; in-radio downloads still require device verification.
- Add host startup state-machine tests for fresh/old/expired/missing TLE, no GPS and cancellation.
- ARM build and 13 host tests passed. Not yet verified on the radio.

## 0.15.0 — 2026-11-20

Initial public app-only repository, based on the standalone development build v15.

- Display name: `ISS tracker`.
- SD filenames: `ISS.app`, `ISS.CFG`, `ISS.TLE`.
- Internal live GPS, ISS SGP4 position/AOS/LOS and RX/TX Doppler.
- VFO A/B FM, CTCSS 67.0 Hz and SPLIT setup.
- Original-settings snapshot, STOP restoration and readback verification.
- Observer coordinates and ACK counters hidden from the normal screen.
- English documentation, a clearly labelled privacy-preserving layout preview,
  source builder and a validated manual TLE update/extraction tool.

The radio-control objects were unchanged from the predecessor where the operator
confirmed restoration after STOP. This rename is not a firmware update.
