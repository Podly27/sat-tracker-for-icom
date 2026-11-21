# Orbital data: SAT.TLE and SAT.BAK

SAT 0.23 reads the enabled profiles in `SAT.CFG` and loads orbital data for each satellite. GPS supplies your location; it cannot supply satellite orbital elements.

At startup, missing records or epochs more than **24 hours** from radio UTC trigger HTTPS refresh. Five-digit catalog numbers use [CelesTrak TLE](https://celestrak.org/NORAD/elements/) with [AMSAT](https://www.amsat.org/tle/current/nasabare.txt) as fallback. Six-digit numbers, including SO-131 / 101101, use CelesTrak's **OMM CSV** because its TLE feed does not carry those objects. See [CelesTrak's format documentation](https://celestrak.org/NORAD/documentation/gp-data-formats.php).

## Combined cache

`SAT.TLE` remains the filename, but it is now a combined application cache. Ordinary objects use a name followed by their original two 69-column TLE lines. Six-digit objects use a name followed by `CSV,` and the original CelesTrak CSV data record. This is **not a pure TLE file** for importing into arbitrary third-party trackers. The app directly initializes SGP4 from OMM mean elements; it does not fabricate or re-encode them as TLEs.

Each record is checked for catalog identity, epoch, supported orbit and successful propagation initialization. TLEs additionally require their standard checksums. CSV records use the documented 17-field CelesTrak layout and strict numeric/date validation. Only 80–110 minute LEOs are supported.

The app evaluates `SAT.TLE` and `SAT.BAK` as complete bundles. It chooses the usable bundle with the most configured satellites, breaking ties by total epoch distance from UTC. It never merges per-satellite records from both slots and then overwrites their only protected sources. After refresh it writes the alternate slot, closes it, reopens it, validates every saved object and compares its record content. A failed write leaves the selected source untouched. This is recoverable two-slot storage, not a guarantee of atomic FAT writes or power-loss immunity.

If only some downloads succeed, valid cached records for the others remain. Offline records within **7 days** can be used; older or missing objects are excluded from prediction/tuning. If no usable objects remain, no radio-control session starts. The startup usable/configured count exposes missing data. Refresh runs at startup, not continuously while tracking; restart to request another update.

## Manual update from a computer

From the repository, run:

```sh
python3 tools/update_tle.py
```

The tool reads the supplied `SAT.CFG`, fetches all enabled objects and writes `sd-card/IC-705/Apps/SAT.TLE`. To use another configuration or destination:

```sh
python3 tools/update_tle.py --config /path/to/SAT.CFG --output /path/to/SAT.TLE --max-age-hours 24
```

For offline validation/extraction from an existing combined cache:

```sh
python3 tools/update_tle.py --input /path/to/combined-cache.TLE --output /path/to/SAT.TLE
```

The input must contain every enabled object; a plain AMSAT TLE list may omit six-digit objects. The computer tool replaces its destination only after all profiles validate, using a temporary file and atomic rename. A failed download or validation leaves an existing output intact. Its default age limit is 168 hours; `--max-age-hours 24` requires fresh data. Published epochs can lag even when a source is reachable.

Refresh before a planned operating session, preferably daily. Never edit dates, catalog numbers or checksums to make old data appear fresh. Historical test fixtures are not operational downloads.
