# SAT for IC-705

An experimental ISS tracker running directly on the Icom IC-705 from its microSD card. It uses internal GPS, refreshes outdated TLE data over the internet, predicts passes and applies RX/TX Doppler correction using VFO A/B and SPLIT. Status appears below the frequency. STOP restores the original radio settings; the app never keys PTT. Currently supports **ISS only**.

## Install

1. Requires **SQ6EMM `705E140_20261119v8` alternative firmware**. Stock Icom firmware cannot run the app; other versions are not supported. Follow the [upstream firmware instructions](https://github.com/sq6emm/icom-custom-firmware).
2. Copy [SAT.app](sd-card/IC-705/Apps/SAT.app?raw=true) and [SAT.CFG](sd-card/IC-705/Apps/SAT.CFG?raw=true) to `IC-705/Apps/` on the card. Remove older tracker executables from that folder.
3. Set the radio clock to **UTC**, enable internal GPS and connect Wi-Fi for the initial TLE download. Offline use requires a valid `SAT.TLE` — see [TLE instructions](docs/TLE.md).
4. Start **Apps → SAT.app**. EXIT hides the window; **STOP ends tracking and restores settings**. Wait for complete exit before switching off.

**Work in progress. Experimental software — use entirely at your own risk. Alternative firmware and this app may cause malfunctions or loss of settings. No warranty is provided.**

[Operation](docs/OPERATION.md) · [Build from source](docs/BUILDING.md)
