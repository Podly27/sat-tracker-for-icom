# IC-705 — SAT 0.42-DECODE

Requires **IC-705 E / Icom 1.40 base with SQ6EMM 705E140_20261119v14**. Use its matching `ICAPCOLD.BIN` and `ICAPSPOT.BIN` from the [firmware project](https://github.com/sq6emm/icom-custom-firmware). This European app is not for v8, IC-705 J or IC-9700. The [separate J preview](../IC-705-J/README.md) has not been tested on Japanese hardware.

## Install

1. Back up the SD card. Copy this folder's `Apps/SAT.app` to `IC-705/Apps/` on the card. Keep only one SAT app.
2. On first installation, also copy `Apps/SAT.CFG`. On updates, keep your existing configuration and orbital cache files.
3. Set the radio clock to **UTC**, enable GPS and connect Wi-Fi for orbital updates.

## Use

1. In the mode selector, cycle **RTTY → RTTY-R → SAT** to select the SAT decoder. Other installed decoders may change the order.
2. Select **FM** again. Tracking messages appear in **SAT DECODE**; the Apps panel is available through **MENU → SET → Apps**.
3. Use **STOP** to finish and restore saved settings. Check the result before POWER OFF. Hiding the window does not stop tracking.

The display shows the satellite, rise/set time, azimuth, elevation, distance and `Up/Down link` frequencies (uplink first). It is a scrolling history. Starting from Apps without first selecting SAT DECODE prompts for the decoder; automatic activation from an arbitrary mode is not implemented.

Release PTT for initial setup, satellite switching and restoration. SAT never keys PTT or sends SO-50's activation carrier. Saved settings are those captured by SAT, after any manual decoder/mode changes.

[Configuration](../../docs/CONFIG.md) · [Orbital updates](../../docs/TLE.md) · [Back](../../README.md)
