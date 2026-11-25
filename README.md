# SAT Tracker for Icom

FM satellite tracking on **IC-705** and **IC-9700**, using SQ6EMM firmware **20261119v14**. SAT selects a satellite pass, calculates Doppler correction and adjusts the uplink/downlink frequencies. Profiles for ISS, SO-50, AO-91, AO-123 and SO-131 are included.

**SAT never keys PTT. Use STOP before switching off the radio**, and check that the original frequencies and modes have returned. EXIT only hides the running app.

| Radio | Current app | Position / internet | Tracking display | Install and use |
| --- | --- | --- | --- | --- |
| IC-705 E, Icom 1.40 base | 0.42-DECODE | Internal GPS / Wi-Fi | SAT DECODE window | [IC-705 guide](sd-card/IC-705/README.md) |
| IC-9700 E, Icom 1.50 base | 0.64 | Manual position / LAN | Apps panel | [IC-9700 guide](sd-card/IC-9700/README.md) |

The versions are **not interchangeable**. Obtain the matching firmware and ICAP runtime files from [SQ6EMM](https://github.com/sq6emm/icom-custom-firmware); they are not included here. Stock Icom firmware cannot run these apps.

## IC-705

![SAT tracking ISS on IC-705](docs/images/sat-ic705.png)

Uses the native scrolling SAT DECODE window for pass information, geometry and uplink/downlink frequencies. The photograph shows operator-controlled TX; SAT does not start transmissions.

## IC-9700

Start **SAT TRACKER** from Apps. It downloads orbital data over LAN, then starts tracking automatically. If the download fails, it tries valid saved data. No separate updater or PC/NAS gateway is needed during operation.

The app uses native SAT/DUAL operation with the downlink on MAIN and uplink on SUB. Doppler correction continues during operator-controlled TX. Position is configured in `SATPOS.CFG`; the display is the Apps panel, not DECODE.

## Download structure

```text
sd-card/
  IC-705/          IC-705 guide and Apps/SAT.app, SAT.CFG
  IC-9700/         IC-9700 guide and its Apps files
  SHA256SUMS.txt   Checksums for the distributed app/config files
tools/
  install_9700.py  First installation / update, including private TLS setup
  update_tle.py    Optional computer-based orbital update
```

This repository contains the current app binaries, installation files and short user guides. Development sources, analyses, diagnostic apps and logs remain local.

Experimental software, without warranty. Predictions do not confirm that a satellite repeater is operational; check its current operating information before transmitting. The merged IC-9700 startup and STOP restoration have been tested on the owner's radio; this does not validate every operating condition.

ICAP and the required firmware are by [Dawid SQ6EMM](https://github.com/sq6emm/icom-custom-firmware). The IC-705 DECODE approach was inspired by [EA5JLJ's SKIM](https://github.com/ea5jlj/skim-ic705).

[Satellite configuration](docs/CONFIG.md) · [Orbital data](docs/TLE.md) · [Third-party notices](THIRD_PARTY_NOTICES.md) · [Checksums](sd-card/SHA256SUMS.txt)
