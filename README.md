# SAT Tracker for Icom

Experimental FM satellite tracking directly on the radio. **Currently IC-705 only; IC-9700 support is in preparation.**

SAT uses internal GPS, downloads outdated orbital data, selects passes and applies RX/TX Doppler correction with VFO A/B, SPLIT and CTCSS. Includes **ISS, SO-50, AO-91, AO-123 and SO-131**; [configuration order](docs/CONFIG.md) sets priority for overlapping passes. The main screen shows `WAIT ISS` or a countdown such as `ISS 2:15`. STOP restores the original radio settings; SAT never keys PTT. Occasional blinking during retuning remains a known limitation.

![SAT running on an IC-705](docs/images/sat-ic705.png)

## Installation

1. Requires **SQ6EMM `705E140_20261119v8` alternative firmware**. Stock Icom firmware cannot run SAT; other versions are not supported. Follow the [firmware instructions](https://github.com/sq6emm/icom-custom-firmware).
2. Copy [SAT.app](sd-card/IC-705/Apps/SAT.app?raw=true) and [SAT.CFG](sd-card/IC-705/Apps/SAT.CFG?raw=true) to `IC-705/Apps/` on the microSD card. Current app: **0.36**.
3. Set the radio clock to **UTC**, enable internal GPS and connect Wi-Fi for orbital updates. See [TLE updates](docs/TLE.md) for offline use and manual downloads.
4. Start **MENU → SET → Apps → SAT.app**. EXIT hides the window; **STOP ends tracking and restores settings**. Wait for complete exit before switching off.

**Work in progress. Experimental software — use entirely at your own risk. No warranty is provided.**

[Configuration](docs/CONFIG.md) · [TLE updates](docs/TLE.md) · [Checksums](sd-card/SHA256SUMS.txt)
