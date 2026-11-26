# IC-705 J — SAT 0.42-J1 volunteer preview

**Not tested on a physical Japanese IC-705. The author owns only a European IC-705. A volunteer with an IC-705 J is needed to validate this preview.** Passing emulation does not prove RF, display, timing or restoration on real hardware.

This app targets **705J140 / SQ6EMM 20261119v14**, ICAP ABI3/build `0x0705140a`. It is not the European app and is not for IC-9700. It does not install or modify firmware.

## Install

1. Use the exact Japanese v14 firmware and **Japanese v14 SD-apps bundle** from [SQ6EMM](https://sq6emm.github.io/icom-custom-firmware/). Keep its matching `ICAPCOLD.BIN` and `ICAPSPOT.BIN`. Do not substitute the E bundle.
2. Back up your radio settings and existing app files. Copy this folder's **`Apps/SAT.app` to `IC-705/Apps/SAT.app` on the card**. The download folder is named IC-705-J only to distinguish builds; the radio's folder is still IC-705. Keep only one SAT app.
3. On first installation, copy `Apps/SAT.CFG` too. On updates, preserve your configuration and TLE caches. This J preview restricts satellite profiles/targets to 144–146 and 430–440 MHz; it does not unlock TX limits.
4. Set the radio clock to UTC, enable internal GPS and connect Wi-Fi for orbital updates.

## First volunteer test

1. Start in RX with GPS fixed, VFO mode, and DUP/SPLIT off on both VFO A and B. Record the original frequencies, modes and tone settings. No transmission is needed for this first test.
2. Cycle the mode selector **RTTY → RTTY-R → SAT** (other installed decoders may change the order), then select **FM**. The Apps entry is named **SAT J TEST**; tracking appears in SAT DECODE.
3. Check GPS, geometry and that the radio's actual frequencies follow the displayed targets. Calculations alone do not confirm tuning. If SAVE/restore errors appear, STOP and report them; some initial SPLIT/DUP states are refused.
4. Use **STOP**, wait for the result and check both VFOs against the recorded original settings. **STOP before POWER OFF**; EXIT only hides the app. SAT never keys PTT.
5. Report the exact J firmware/bundle version, a photo of the Apps panel, initial/final A/B settings, GPS/update/tuning results and STOP result. Send `SATDBG.LOG` only if it exists; not every error creates this file. Operator-controlled TX/Doppler still needs a later separate hardware test with an appropriate dummy load.

## What was checked

The matching J firmware was verified against the upstream hashes. Emulation covered native J GPS bindings, UART parser, app registration/loading, DECODE/COLD output and region rejection. Whole-app scenarios checked startup, modeled TX Doppler, failures and STOP restoration. The compiled ARM SGP4/Doppler calculation was also compared with an independent implementation.

The whole-app tests model the ICAP callbacks, RF/CI-V consumer and pass geometry. They do not emulate the complete radio scheduler, Wi-Fi/TLS hardware or RF chain. **Physical Japanese-radio compatibility remains unconfirmed.**

[Satellite configuration](../../docs/CONFIG.md) · [IC-705 E guide](../IC-705/README.md) · [Back](../../README.md)
