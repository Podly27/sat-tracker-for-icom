# Orbital data updates

GPS gives SAT your position; **TLE/OMM records describe the satellites' orbits**. SAT calculates geometric horizon crossings, not optically visible passes. Times in the app are **UTC**; mobile trackers may use local time, a minimum elevation or visibility filters.

At startup SAT refreshes missing records or epochs more than **24 hours** from radio UTC using Wi-Fi. Ordinary catalog numbers use [CelesTrak TLE](https://celestrak.org/NORAD/elements/) with [AMSAT](https://www.amsat.org/tle/current/nasabare.txt) as fallback. Six-digit numbers such as SO-131 use CelesTrak OMM CSV. Restart SAT to request another check; refresh does not run continuously during tracking.

Refresh before an operating session, preferably daily. Offline records within **7 days** can be used; older or missing satellites are excluded. Never edit epochs or checksums to make old data look fresh.

## Manual update from a computer

Download the repository and run this command from its directory with Python 3 (no extra packages needed):

```sh
python3 tools/update_tle.py --max-age-hours 24
```

The tool reads `sd-card/IC-705/Apps/SAT.CFG` and writes `SAT.TLE` beside it. Copy that file into `IC-705/Apps/` on the card. Custom paths are also supported:

```sh
python3 tools/update_tle.py --config /path/to/SAT.CFG --output /path/to/SAT.TLE --max-age-hours 24
```

All enabled objects must validate before the computer tool replaces its output. A failed download leaves the previous file intact.

## Files on the card

`SAT.TLE` is a combined cache: each satellite has a name and its original two TLE lines, or `CSV,` followed by the original CelesTrak OMM record for a six-digit catalog number. It is not a pure TLE list for arbitrary third-party trackers.

SAT selects a valid complete bundle from `SAT.TLE` and `SAT.BAK` and writes verified updates to the other slot, preserving the previous source. A successful download may therefore update either file. These operational files are not supplied as fixed data in this repository because they become outdated.
