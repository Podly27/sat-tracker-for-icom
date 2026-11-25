# Orbital data

Set the radio clock to **UTC**. SAT uses geometric rise/set predictions; these do not indicate optical visibility or repeater availability. Offline records up to **seven days** old can be used; older/missing satellites are excluded.

| Radio | Update at startup | Saved files in its Apps folder |
| --- | --- | --- |
| IC-705 | Wi-Fi; refreshes missing data or epochs more than 24 hours old, using CelesTrak / AMSAT | `SAT.TLE`, `SAT.BAK` |
| IC-9700 | LAN; downloads configured satellites from SatNOGS over verified HTTPS, then starts the tracker | `SATN0.TLE`, `SATN1.TLE`; also reads `SAT.TLE`, `SAT.BAK` |

Updates alternate between cache files and are checked before use. If the network update fails, SAT tries valid saved data. Refresh runs at startup, not continuously during tracking. On IC-9700, stop and restart SAT TRACKER to request another update; on IC-705, stop and select its decoder again.

Caches contain combined satellite records: names and two TLE lines, or `CSV,` plus an OMM record. They are generated during use, not supplied as fixed data in the repository. Keep them when upgrading. Do not edit epochs/checksums to make old data look fresh.

## Optional computer update

Python 3, no extra packages:

```sh
python3 tools/update_tle.py --config /path/to/SD/IC-705/Apps/SAT.CFG --output /path/to/SD/IC-705/Apps/SAT.TLE --max-age-hours 24
```

For IC-9700, use its `IC-9700/Apps/` paths instead. The script checks all enabled records before replacing its output; unavailable upstream data leaves the old file intact. IC-9700 normally updates directly over LAN and does not need this tool during operation.

[IC-705 guide](../sd-card/IC-705/README.md) · [IC-9700 guide](../sd-card/IC-9700/README.md)
