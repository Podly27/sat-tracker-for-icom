# SAT Tracker for Icom

An experimental FM satellite tracker running directly on the radio from its microSD card. **Currently supports IC-705 only; IC-9700 support is in preparation.** It uses internal GPS, refreshes outdated TLE data over the internet, predicts passes and applies RX/TX Doppler correction using VFO A/B and SPLIT. A single short status below the frequency, such as `WAIT SO-131`, refreshes every 500 ms; pass details stay in the app window. STOP restores the original radio settings; the app never keys PTT. Includes **ISS, SO-50, AO-91, AO-123 and SO-131**. [SAT.CFG](docs/CONFIG.md) selects up to eight profiles; the earliest pass wins and overlaps follow list order.

![SAT running on an IC-705, showing SO-131 pass information](docs/images/sat-ic705.png)

SO-131 tracking: azimuth/elevation, Doppler-corrected RX/TX frequencies, next pass and CTCSS/priority.

## Install

1. Requires **SQ6EMM `705E140_20261119v8` alternative firmware**. Stock Icom firmware cannot run the app; other versions are not supported. Follow the [upstream firmware instructions](https://github.com/sq6emm/icom-custom-firmware).
2. Copy [SAT.app](sd-card/IC-705/Apps/SAT.app?raw=true) and [SAT.CFG](sd-card/IC-705/Apps/SAT.CFG?raw=true) to `IC-705/Apps/` on the card. Remove older tracker executables from that folder.
3. Set the radio clock to **UTC**, enable internal GPS and connect Wi-Fi for the initial TLE download. Offline use requires a valid `SAT.TLE` — see [TLE instructions](docs/TLE.md).
4. Start **Apps → SAT.app**. EXIT hides the window; **STOP ends tracking and restores settings**. Wait for complete exit before switching off.

**Work in progress. Experimental software — use entirely at your own risk. Alternative firmware and this app may cause malfunctions or loss of settings. No warranty is provided.**

[Configuration](docs/CONFIG.md) · [Operation](docs/OPERATION.md) · [Build from source](docs/BUILDING.md)
