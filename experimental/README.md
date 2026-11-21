# Experimental HTTPS probe

`TLECHECK.app` is a separate diagnostic application, not a replacement for `ISS.app`. It has not been verified on radio hardware. It targets the pinned IC-705 E v8/v14 network callback tables, makes no tuning/PTT commands, and does not write to the SD card.

Copy it to `IC-705/Apps` alongside the matching firmware support files, stop other apps, connect the radio to the internet and start it through Apps. Record the source and final status, then use STOP.

See [the full test procedure and limitations](../docs/NETWORK-AND-DISPLAY.md). Build with `python3 tools/build.py --probe`.

## Native ISS text popup

`DISPCHK.app` is a separate experimental native information-dialog check. It reads internal GPS and `ISS.TLE` (falling back to `SATELITE.TLE`), computes ISS azimuth/elevation/range and opens the firmware's seven-line text popup. It uses no CI-V/PTT or SD writes and does not inject receive packets. Close the popup with BACK, then stop the app through Apps. Run again for an updated snapshot.

This is **text with a direction marker, not a live graphical compass**. It does not yet display status under the main frequency. That requires the separate native display bridge described in the research notes. Build with `python3 tools/build.py --display-probe`. Hardware behavior remains to be verified.

The firmware Apps scanner accepts at most eight application files. If adding both probes exceeds that limit, temporarily move an unused app outside Apps rather than deleting it; restore it after testing.
