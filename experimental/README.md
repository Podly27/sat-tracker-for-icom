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

## 4. FRQTEXT.app — text below the main frequency (test 3)

Reads GPS/TLE and attempts to paint `ISS AZ… EL… IN RANGE/BELOW` at native coordinates x=12, y=96. Start in FM, use EXIT until the ordinary frequency screen is visible and look below the frequency. Drawing is suppressed in menus/Apps, other modes and TX. Use Apps STOP when finished; the app requests a native redraw on exit.

This uses original native drawing routines directly, not invented APRS receive packets. It does not patch firmware code or install rendering hooks. **This app-task drawing path is experimental:** the native renderer may overwrite it or produce flicker. A proper UI-thread bridge is still required for production integration. Native GPU scheduling, LCD placement and complete redraw after STOP need device verification.

Build: `python3 tools/build.py --frq-probe`. The IC-705 E v8/v14 UI callback and drawing signatures must match; other bindings are refused. These UI-only bindings do not make the main tracker v14-compatible.

## Card contents

The Apps scanner has an eight-file limit and macOS `._*.app` metadata can consume entries. For this test round, keep only the four named test apps in Apps. Preserve other apps in a backup directory outside Apps, retain CFG/TLE and matching firmware support BIN files, and move macOS app sidecars outside Apps before ejecting the card.

Test 3 uses normal-radio surface 12 instead of the Apps surface. It adds rows `FG/M/TX/R`, `DRAW/MAIN/ELSE`, observed UI codes and last draw state. Inspect these by returning to the Apps window after using EXIT to check the main frequency screen. A computed position alone does not prove a visible GPU render. The first test was reported to calculate position but show no main-screen text.

The integrated `sd-card/IC-705/Apps/ISS.app` is now version 0.18.0 and **does configure/tune the radio** after valid TLE, GPS and pass search. It is distinct from these read-only/control-free probes. Its network and SD-write startup integration still needs device verification.
