# Experimental HTTPS probe

`TLECHECK.app` is a separate diagnostic application, not a replacement for `ISS.app`. It has not been verified on radio hardware. It targets the pinned IC-705 E v8/v14 network callback tables, makes no tuning/PTT commands, and does not write to the SD card.

Copy it to `IC-705/Apps` alongside the matching firmware support files, stop other apps, connect the radio to the internet and start it through Apps. Record the source and final status, then use STOP.

See [the full test procedure and limitations](../docs/NETWORK-AND-DISPLAY.md). Build with `python3 tools/build.py --probe`.
