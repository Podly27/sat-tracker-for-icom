# Experimental radio checks

These are separate diagnostic apps, not a replacement for the tracker. Stop each app before starting the next. They make no CI-V/PTT commands and do not write to SD. Device results remain necessary.

## 1. TLEDNS.app — hostname

First opens ordinary TCP port 443 to `celestrak.org` (no HTTP request and no TLS). Then tries HTTPS TLE retrieval from CelesTrak and AMSAT using hostnames. It reports `TCP443 C:… E:…`, `CL TLS C:… E:…` and `AM TLS C:… E:…` plus elapsed milliseconds. `C >= 0` is an opened handle; negative values are wrapper failures. `E` is the captured native socket error at failure. `NOT ATTEMPTED` is not a success.

## 2. TLEIP.app — direct IPv4

Same test using `104.168.149.178` for CelesTrak and `104.20.25.253` for AMSAT, resolved on 2026-11-21. The HTTP Host headers remain the proper hostnames. These IPs are diagnostic fixtures and may change.

**Compare TCP results first.** Hostname TCP failing while numeric TCP succeeds points toward name resolution. Numeric HTTPS failing does not alone diagnose DNS: supplying an IP also changes the name seen by the firmware TLS setup (SNI/certificate checking). The tests do not disable TLS checks and do not download over plaintext HTTP. Neither test stores the downloaded TLE.

Build: `python3 tools/build.py --probe` and `python3 tools/build.py --probe --probe-ip`. These supersede the original `TLECHECK.app`.

## 3. DISPCHK.app — native information dialog

Reads internal GPS and `ISS.TLE` (falling back to `SATELITE.TLE`), calculates azimuth/elevation/range and opens a text popup. The first four lines carry the information; the last three are reserved for native CLOSE. The native error-dialog icon is part of that widget, not a claim that ISS tracking failed.

The popup is a **snapshot with a text direction marker, not a live graphical compass**. Close with CLOSE/BACK, then use Apps STOP. Run again for a fresh snapshot.

Build: `python3 tools/build.py --display-probe`.

## 4. FRQTEXT.app — APRS-derived receiver-status text (test 5)

Uses the final receiver-status display cache used after APRS/native header processing. It publishes a compact `ISS A105 E+13` status and invokes the original UI notification. It does not write the RX packet mailbox or pending-receive flag and does not call the receive-history/packet-processing path. Rendering is left to the normal UI task, including when a menu is currently open.

Set the ordinary radio mode to **FM** first, then start the app and use EXIT to view the frequency screen. The current normalized mode is read from the same field used by APRS; `LIVE:7` permits FM publication. The ICAP cached byte `C:70` can be stale and no longer blocks it. TX suppresses publication. The `PUB` counter counts cache updates, not confirmed LCD frames. `C/LIVE/TX/R` and observed UI codes are now formatted correctly. Use Apps STOP to restore the original cache if it is still owned; foreign updates are preserved.

Earlier diagnostics used unsupported `%X` in the tiny formatter, omitted those fields and shifted later arguments. Consequently the photographed `R:112` was not a reliable callback return code; the shifted value was consistent with packed mode `0x70`, which the FM-only test rejects. Test 4 includes actual formatter regression tests.

This is an experimental UI cache probe, not a confirmed production SAT overlay or a graphic compass. Native signatures are pinned for IC-705 E v8/v14 UI bindings; the main tracker's RF-control target remains v8. No tuning/PTT, firmware/code patches or SD writes occur in this probe.

Build: `python3 tools/build.py --frq-probe`.

## Card contents

The Apps scanner has an eight-file limit and macOS `._*.app` metadata can consume entries. For this test round, keep only the four named test apps in Apps. Preserve other apps in a backup directory outside Apps, retain CFG/TLE and matching firmware support BIN files, and move macOS app sidecars outside Apps before ejecting the card.

The integrated `sd-card/IC-705/Apps/ISS.app` is now version 0.18.0 and **does configure/tune the radio** after valid TLE, GPS and pass search. It is distinct from these read-only/control-free probes. Its network and SD-write startup integration still needs device verification.
