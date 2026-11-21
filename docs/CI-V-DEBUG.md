# CI-V startup diagnostics (0.25)

The owner reported `RESTORE/SAVE: CIV BUSY` on the first app start after radio power-on. The photographed snapshot is `Q0R0C05 U010100000000000000`: no queued request/reply-ready flag, but parser count 5 and UART bytes 0/1 both 1. It identifies a blocked admission guard; it does not identify the producer or authorize clearing it.

This diagnostic version retains the same idle/ownership guards. It adds **read-only native transport snapshots stored in app RAM**, taken at:

- `BOOT`: after firmware pin checks, before configuration/TLE startup.
- `FIRST`: immediately before the first owned transaction attempts admission.
- `REPLY`: the most recent correlated reply accepted by the existing owner checks, before release.
- `FREE`: immediately after releasing that same owned transaction.
- `BUSY`: the most recent refused admission.
- `FAIL`: the final snapshot when SAVE fails, before file I/O.

Only existing/valid snapshots are written. Snapshots cannot reset parser/UART flags, consume unknown replies or dispatch commands. Native transport reads run inside the existing short interrupt guard. No filesystem calls occur there.

After a capture error, SAT writes the snapshots once to **`IC-705/Apps/SATDBG.LOG`**, using the regular ICAP file API and closing it immediately. Normal operation does not generate a new log; STOP does not write it. An older log is replaced on the next capture failure. If writing fails or is cancelled, the file may be missing/partial; the on-screen phase still helps.

The Apps window replaces the pass-status row with `S… W… A… F… R… OK…`: snapshot phase, waiting flag, active owned token, requested field, last submission result and number of accepted submissions. For example, `S0 W0 A0 F6 R-1 OK0` means the first SELECT request never entered the queue. Target RX/TX values are explicitly labelled **target** until a confirmed controller value exists; a computed orbit is not proof that the radio tuned.

## Log format

Each row is `LABEL T=… K=… N=… P=… D=…`. `T` is native milliseconds, `K` the typed request kind, `N` its length, and `P` the seven-byte padded requested payload. `D` is 42 bytes of raw hexadecimal:

| Bytes | Native source |
|---|---|
| 0–8 | Request buffer bytes 0–8 (clipped prefix, not every possible frame) |
| 9 | Request length at queue + 0x82 |
| 10–25 | First 16 reply-buffer bytes at queue + 0x83 |
| 26 | Reply-ready flag at queue + 0x103 |
| 27–35 | UART-state bytes 0–8 |
| 36–41 | Parser-state bytes 0–5 |

Return the card only after stopping SAT and switching the radio off. The log is diagnostic data, not a proven diagnosis. Keep device logs out of the public repository; `.gitignore` excludes `*.LOG`. This build does not claim to fix the underlying startup busy state.
