# ISS tracker for IC-705

## Current development status — 2026-11-21

| Capability | Status |
|---|---|
| Standalone ISS GPS/Doppler tracking and STOP restoration | Existing v8 target; see hardware/software validation notes below |
| Startup TLE-age check and AOS/LOS search | Implemented locally in 0.16.0; device verification pending |
| Internet TLE retrieval | Extended v8/v14 socket API identified; separate experimental `TLECHECK.app` built and host-tested; radio verification pending |
| Tracker radio control on v14 | Not yet supported/validated; v14 probe compatibility does not imply tracker compatibility |
| Native ISS detail popup | Experimental `DISPCHK.app` shows azimuth/elevation and a direction marker in a native text dialog; snapshot only; radio verification pending |
| SAT status under the main frequency and graphic compass | APRS native path analysed; dedicated UI bridge still required; not implemented |

**Correction:** an earlier development note said v8 exposed no network API. It does: our original header omitted the extended entries. See [network research, test procedure and native display findings](docs/NETWORK-AND-DISPLAY.md).


A standalone **ISS FM cross-band repeater tracker** that runs on the Icom IC-705 as an ICAP application from its microSD card. It uses the radio's internal GPS, calculates the ISS position and Doppler correction, and controls VFO A/B through the radio's native CI-V processing.

**Alternative firmware is required. Stock Icom firmware cannot run this application.** This release targets **SQ6EMM `705E140_20261119v8`**, ICAP ABI 3 / build `0x07051405`. Other SQ6EMM versions, including newer releases, are not automatically compatible.

This repository contains the application and its source. It does **not** contain Icom firmware, firmware patches, decoded radio images, manuals, or the separate experimental native-screen firmware.

## What it does

- Tracks **ISS only**, using SGP4 and an `ISS.TLE` file.
- Reads a live fix from the IC-705's **internal GPS**; no manually entered location is used.
- Displays UTC, azimuth, elevation, slant range, the corrected RX/TX frequencies, and the next acquisition or loss of signal (AOS/LOS).
- Configures **VFO A for downlink reception**, **VFO B for uplink transmission**, FM on both VFOs, **67.0 Hz CTCSS** on the uplink, and **SPLIT**.
- Applies Doppler correction during reception and updates the uplink while the operator transmits.
- Captures the pre-session VFO settings and restores them after **STOP**, with readback checks.
- Never presses PTT or starts a transmission. The operator controls PTT.

| ISS repeater profile | Nominal value |
|---|---:|
| Downlink / RX A | 437.800 MHz FM |
| Uplink / TX B | 145.990 MHz FM |
| Uplink CTCSS | 67.0 Hz |

Check [ARISS station status](https://www.ariss.org/current-status-of-iss-stations.html) before operating. An ISS position above your horizon does not establish that the repeater is currently available.

## Screen preview

![ISS tracking on an Icom IC-705 during a pass](docs/images/iss-tracker-radio.png)

**Actual IC-705 display photographed during an ISS pass.** The photograph shows an earlier build named `SATELITE`; the current application is named `ISS tracker` and is loaded from `ISS.app`. No observer coordinates are shown. Displayed times and frequencies describe that photographed pass, not current conditions.

| Display item | Meaning |
|---|---|
| `ISS tracker - running` | Application name and running state in the Apps window. |
| `ISS / FM REPEATER BELOW` | `BELOW` means the calculated ISS position is below the geometric horizon; the photograph shows `IN RANGE`, meaning elevation is at least 0°. |
| UTC / `GPS FIX` | Radio time and a valid internal GPS position. Set the radio clock to UTC. |
| `AZ`, `EL`, distance | Azimuth clockwise from north, elevation above/below the horizon, and slant range in kilometres. |
| `RX A`, `TX B`, `SPLIT` | Doppler-corrected frequencies used by the tracker. At startup they can be computed targets; after control/readback they are the last confirmed values. |
| `AOS` / `LOS` | Estimated beginning of the next visible ISS pass or end of the current pass, in UTC. |
| EXIT footer | Hides the application window; **it does not stop tracking**. |

See [screen and operation details](docs/OPERATION.md). This standalone release does **not** replace the normal `FM` label with `SAT` and does not put information in the native DECODE/scope area.

## Requirements and firmware compatibility

| Requirement | Supported target |
|---|---|
| Radio | Icom IC-705, E firmware family |
| Firmware base | Icom `705E140`, with **SQ6EMM `20261119v8`** |
| Loader interface | ICAP ABI 3, build `0x07051405` |
| Position | Internal GPS with a live fix |
| Time | Correct radio date/time in UTC |
| Start context | VFO A or B; **DUP+ / DUP− disabled** |
| Orbit data | Valid ISS elements, NORAD **25544**, in `ISS.TLE` |

Upstream: [SQ6EMM custom firmware](https://github.com/sq6emm/icom-custom-firmware), [the version-specific v8 patch](https://github.com/sq6emm/icom-custom-firmware/blob/main/patches/705E140_20261119v8.json).

Installing alternative radio firmware is a separate operation with a risk of making the radio unusable. Follow the upstream backup/recovery instructions and use your own official Icom firmware file. **Do not substitute the latest upstream version just because it has Apps support.** This application's native bindings are version-specific; its checks are not a guarantee against every incompatible image.

## Install the application

If the supported alternative firmware is already installed, **no firmware update is needed for this application**.

1. Download [`ISS.app`](sd-card/IC-705/Apps/ISS.app?raw=true) and [`ISS.CFG`](sd-card/IC-705/Apps/ISS.CFG?raw=true), or clone this repository.
2. Create a fresh `ISS.TLE` as described below.
3. With the tracker stopped and the card removed from the radio, place these files on the card:

   ```text
   IC-705/
     Apps/
       ISS.app
       ISS.CFG
       ISS.TLE
   ```

4. Back up old tracker files outside the active `Apps` folder. Keep only one tracker executable; do not run old `SATELITE.app` and `ISS.app` together.
5. Safely eject the card, insert it into the powered-off radio, and start the radio.
6. Open **MENU → SET → Apps → ISS.app**. The window is named **ISS tracker**.

`ISS.CFG` is currently a reserved placeholder. This release does not read settings from it. The application reads `ISS.TLE`; renaming an old TLE file without updating its elements does not make it current.

On macOS, `._*` AppleDouble sidecar files can confuse the older Apps loader. Remove only those sidecars from the card's Apps directory, keep its application count within the loader limit, and eject the card properly. The file extension may appear uppercase in the radio's FAT filename listing.

## Create and update ISS.TLE

TLE means **Two-Line Element set**: orbital data with an epoch and checksums, not your station coordinates. **Download real elements; do not invent the numbers or derive them from GPS.**

With Python 3.9 or later:

```sh
python3 tools/update_tle.py
```

The tool requests NORAD 25544 from CelesTrak, falls back to AMSAT if necessary, validates the pair, and writes `sd-card/IC-705/Apps/ISS.TLE`. It reports the source, epoch and age. A failed download or invalid response leaves an existing output file intact.

To write directly to a mounted card, use its actual mount path, for example on macOS:

```sh
python3 tools/update_tle.py --output "/Volumes/NO NAME/IC-705/Apps/ISS.TLE"
```

For an already downloaded multi-satellite TLE text file:

```sh
python3 tools/update_tle.py --input downloaded-elements.txt --output ISS.TLE
```

**Recommended workflow:** update before an operating session and aim for elements whose **epoch is less than 24 hours old**. Refresh before the next pass after known ISS manoeuvres or if predictions look inconsistent. This is an operational recommendation, not a guaranteed accuracy threshold. Daily refresh does not mean frequent polling of the data service is needed.

The application pauses control when the absolute difference between its radio UTC and the TLE epoch exceeds **7 days**. That is a rejection limit, **not** a recommendation to use week-old ISS elements. Restart the stopped application after replacing `ISS.TLE`; this release loads it at startup.

[Detailed TLE instructions, manual creation and troubleshooting](docs/TLE.md).

## STOP and restoration

Use **STOP in the Apps menu**, release PTT, and wait for the application to finish. EXIT only hides the window.

The tracker restores the captured A/B frequencies, modes/data/filter selections, CTCSS frequency and enable state, tone-squelch state, simplex operation, original VFO selection and SPLIT. It verifies readbacks. If it cannot complete restoration, it reports an error rather than claiming success.

The snapshot is kept in RAM for the current session. It is not a power-loss backup. Do not remove the battery/card or cut power while restoration is still running.

## Status and limitations

- The project is experimental and is not Icom software or endorsed by Icom or SQ6EMM.
- Hardware feedback confirmed GPS reception, RX correction cycles, software-confirmed uplink changes during operator-controlled TX, and successful restoration after STOP in the predecessor build.
- Version **0.15.0** renames that standalone application and its files to ISS. The four radio-control object files were byte-identical to the predecessor when checked locally; the renamed build still needs its own device confirmation.
- Software readback is not an independent measurement of the transmitted RF frequency.
- Manual tuning/mode changes can stop automatic control. Use STOP and start a fresh session to resume.
- There is no satellite selection, multi-satellite pass list, Doppler toggle, native SAT/DECODE panel, automatic PTT, or in-radio Internet TLE update in this release.
- AOS/LOS searching uses a 30-second sampling step and is an estimate, not a second-accurate prediction.

See [validation scope](docs/VALIDATION.md) and [changelog](CHANGELOG.md).

## Build from source

```sh
python3 -m venv .venv
source .venv/bin/activate
python -m pip install -r requirements.txt
python tools/build.py
```

The output is `build/ISS.app`. Building does not require, fetch, or modify an Icom firmware file and does not access a radio. See [build details](docs/BUILDING.md).

## Credits and licensing

- ICAP runtime/alternative firmware and reverse-engineering reference: [Dawid SQ6EMM](https://github.com/sq6emm/icom-custom-firmware).
- SGP4 C implementation: [aholinch/sgp4](https://github.com/aholinch/sgp4), pinned in [SOURCE.txt](vendor/sgp4/SOURCE.txt), under the Unlicense.
- Project-authored application/tooling/documentation: [MIT](LICENSE). See [third-party notices](THIRD_PARTY_NOTICES.md) for vendored code and linked toolchain runtime notices.

Icom firmware and manuals are not licensed or distributed by this repository. Icom and IC-705 are names/trademarks of their respective owner.

## Startup sequence (0.16.0)

The startup screen validates ISS.TLE against its orbital epoch and the radio UTC clock, waits for internal GPS, then calculates the next AOS/LOS before entering the receive screen. TLE older than 24 hours is flagged; validated data within the existing seven-day limit can still be used offline. Missing, invalid or expired elements prevent tuning. STOP can cancel startup without changing radio settings.

**Automatic in-radio updates are not enabled in the tracker yet.** Analysis of the extended ICAP tables found socket callbacks in both v8 and v14; our earlier claim that v8 had no network API was incorrect. The separate experimental `TLECHECK.app` probes HTTPS without tuning or SD writes. See [network and native display research](docs/NETWORK-AND-DISPLAY.md). Until device verification is complete, update the card with `tools/update_tle.py` on a computer.
