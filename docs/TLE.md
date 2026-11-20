# ISS orbital data: ISS.TLE

The application needs a real ISS **NORAD 25544** TLE pair. It does not need a TLE for another satellite or a location file. Your observer position comes from the radio's internal GPS.

## Recommended method

Run `python3 tools/update_tle.py` from the repository. It uses:

1. [CelesTrak ISS TLE request](https://celestrak.org/NORAD/elements/gp.php?CATNR=25544&FORMAT=TLE).
2. [AMSAT current orbital elements](https://www.amsat.org/tle/current/nasabare.txt), as a fallback/extraction source.

A source can be unavailable, rate-limited or return an error page. The tool validates content before replacing the destination. It performs a single request per source, not continuous polling.

The tool checks the satellite number, pairing, 69-column line lengths, modulo-10 checksums, epoch, and basic low-Earth-orbit fields. Its default maximum absolute epoch age is 168 hours, matching the app's seven-day rejection window. Use `--max-age-hours 24` to require stricter freshness when downloading:

```sh
python3 tools/update_tle.py --max-age-hours 24
```

An update can fail with that setting even if the source is reachable, because its published element epoch is older than 24 hours. Do not edit the epoch to bypass this check.

## Manual creation

1. Open a source above and find ISS, satellite number **25544**.
2. Save a plain ASCII text file named `ISS.TLE`, containing an optional satellite name followed by the complete two element lines:

   ```text
   ISS (ZARYA)
   <the complete 69-character line beginning with "1 25544">
   <the complete 69-character line beginning with "2 25544">
   ```

   The angle-bracket lines above are placeholders, **not usable orbital data**.
3. Keep all spaces, decimal points, signs and checksum digits. Avoid a word processor or formatting that wraps lines.
4. Validate/extract it with `python3 tools/update_tle.py --input your-file.txt --output ISS.TLE`.
5. Copy it to `IC-705/Apps/ISS.TLE`, safely eject the card and restart the stopped application.

Both LF and CRLF input are accepted. The name line is descriptive; the actual identity is checked in both element lines. Do not copy JSON/OMM data into this text file; this release expects the traditional TLE format.

The file in `tests/fixtures/` is historical test data. It is deliberately **not** an installation file or a claim to current ISS elements. The repository does not ship a predated operational `ISS.TLE`.

## How often to update

As a project operating policy, refresh **before each operating session**, aiming for an epoch less than 24 hours old. Refresh again after known ISS manoeuvres or unexplained timing/direction errors. Stationary radio GPS does not compensate for stale satellite elements.

Freshness is measured from the **element epoch**, not the file modification time or download time. Line1 contains a two-digit epoch year and a fractional day of that year. For example, `26323.84989826` means year 2026, day 323 plus a fraction of a day; it is not a calendar date written as YYYYMMDD.

The seven-day application cutoff is a protective limit. It does not establish that every younger TLE will predict an ISS pass accurately enough for a particular station or antenna.

## Troubleshooting

| Symptom | Check |
|---|---|
| `ISS.TLE invalid / missing` | Exact path/name, plain text, both 25544 lines, length, checksums and supported orbital fields. |
| `ISS.TLE expired (>7 days)` | The element epoch **and** radio date/time in UTC. |
| File downloads but is rejected | A source may have returned HTML/JSON, an incomplete pair, or elements outside the allowed age window. |
| Predicted pass differs from expectation | Check fresh elements, UTC/date, live GPS, and the antenna horizon. Predictions use a geometric horizon and sampled search. |
| Updated file is not used | Stop the app fully, replace the card file, then launch it again. There is no live reload in this release. |

Format reference: [CelesTrak NORAD Two-Line Element Set Format](https://celestrak.org/NORAD/documentation/tle-fmt.php). Repeater availability is separate from orbital data; check [ARISS](https://www.ariss.org/current-status-of-iss-stations.html).
