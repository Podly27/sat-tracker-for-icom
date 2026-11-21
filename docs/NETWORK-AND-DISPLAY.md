# Network and main-screen integration research

Status: 2026-11-21. This is a development report, not a claim of device validation.

## What has changed

Our previous statement that firmware v8 had no networking API was incorrect. The app's local `Api` declaration stopped at offset 0x50; reverse engineering the **extended** table in v8 and v14 found network callbacks beyond that boundary. No firmware update is inherently required just to investigate networking on v8.

The tracker remains targeted at v8. The separate experimental **TLECHECK.app** recognises the network function table of v8 and v14. That does not make the tracker radio-control bindings v14-compatible.

## Pinned evidence

Upstream repository: [SQ6EMM custom firmware](https://github.com/sq6emm/icom-custom-firmware/tree/d890902db03ea716e92848fe8b3c173d71a18e25).

- v14 patch: `patches/705E140_20261119v14.json`; decoded main MD5 `290c76af32ecc54c23aa0b9a0aa797a3`.
- v8 decoded main MD5 `13b956a2b958be7ccfa51f4011584616`.
- Both reconstructed from the owner's original Icom image; neither proprietary image is distributed here.
- The build identifier remains `0x07051405` in both versions; it alone cannot distinguish v8 from v14.

The reverse-engineered ARM32 table offsets are:

| Offset | Inferred callback | v8 function pointer | v14 function pointer |
|---|---|---|---|
| 0x6c | Network readiness, 0 when ready | 0x2037b551 | 0x2037b579 |
| 0x70 | Connect(host, port, flags, timeout_ms) | 0x2037c455 | 0x2037c601 |
| 0x74 | Send(handle, buffer, count, timeout_ms) | 0x2037bb29 | 0x2037bc21 |
| 0x78 | Receive(handle, buffer, count, timeout_ms) | 0x2037bc3d | 0x2037bd35 |
| 0x7c | Close(handle) | 0x2037b611 | 0x2037b639 |
| 0x80 | Last socket error | 0x2037b563 | 0x2037b58b |
| 0x84 | Native seven-line popup/menu | 0x2037c011 | 0x2037c101 |

Run `python3 tools/audit_network_table.py /path/to/privately-decoded-main.bin` to reproduce the read-only table checks.

Pointers are Thumb-tagged. `tle_network_supported()` checks all five readiness/transport pointers plus radio/build/ABI/size before any network call. These are inferred internal interfaces, not an upstream public SDK contract.

In the v14 connect wrapper, flag bit 2 selects UDP (`type=2`, `protocol=17`). With bit 2 clear, flag bit 0 selects `type=1`, `protocol=100`, versus ordinary TCP protocol 6. The secure branch also configures socket option 0x2d with the hostname. We infer a TLS path from this, but **certificate validation, SNI semantics and TLS interoperability have not been verified on the radio**. The probe uses port 443 and flags 1; there is no plaintext HTTP fallback.

## Isolated HTTPS test

Build:

```sh
python3 tools/build.py --probe
```

The result is `build/network-probe/TLECHECK.app`; a matching prebuilt copy is in [`experimental/TLECHECK.app`](../experimental/TLECHECK.app). Copy it into the radio's Apps directory, keep the matching upstream firmware support files installed, connect the radio to a network with internet access, set the clock to UTC, and start the check through Apps. Stop the tracker and other apps first.

The check:

1. Verifies the supported extended function table and network readiness.
2. Tries CelesTrak over HTTPS, then AMSAT if the first attempt fails.
3. Parses bounded HTTP responses, including Content-Length, close-delimited and chunked bodies.
4. Requires one ISS 25544 line pair with correct lengths/checksums, validates the orbit through SGP4 and reports epoch offset against radio UTC.
5. Shows `PASS: ISS TLE <=24h` only after those checks succeed.
6. Closes every successfully opened handle before waiting for STOP.

**No frequency, mode, VFO, SPLIT or PTT commands are present in the probe. It does not write to the SD card and does not replace ISS.TLE.** Replacing the tracker with it is unnecessary: it is a separate diagnostic app.

The probe makes at most two requests per launch. Transport operations use finite timeouts and the application checks STOP between calls. Firmware DNS/TLS internals may have additional blocking behavior that cannot be bounded by these host tests. HTTP redirects, compression, chunk trailers, malformed or oversized input are rejected. This conservative probe can reject a legitimate server response; report the displayed error rather than weakening validation.

Device verification needed: firmware version, source hostname, final status/epoch offset, STOP response. A positive result verifies a software download, not TLS peer authentication. Do not treat network TLE checksum validation as a cryptographic authenticity guarantee.

## Main radio screen and APRS photographs

The supplied APRS photographs demonstrate native RX text under the frequency and a native position/detail dialog. Analysis of APRS.APP confirms it publishes into the original Icom receive-message path: an IC-705 E profile references a receive mailbox at `0x203e23c1`, pending flag `0x205d9dab` bit `0x40`, and receiver-state byte `0x203e212f`. It is not simply drawing arbitrary text through `Api.line()`.

Sending invented APRS/D-STAR receive messages to show satellite status could affect receive history, callsigns and native packet handling. The tracker does not do that. `Api.line()` still writes its Apps window. The seven-line popup at offset 0x84 is a separate native UI helper; it is not the APRS compass/detail API, and is not invoked by the network probe.

![Design proposal only, not implemented: satellite status under the frequency and a dedicated detail view](images/native-sat-proposal.svg)

The image is a layout proposal, not a screenshot or an implemented feature. Its values are illustrative. The intended main-screen display is:

- A short SAT status under the frequency: ISS visibility, elevation and time to AOS/LOS.
- A separate detail view with azimuth/elevation, countdown, RX/TX frequencies and eventually a polar plot.
- Native FM radio operation and meter/scope behavior preserved.
- Explicit ownership and cleanup when the app stops or ceases updating; no fake received packets.

That requires a dedicated, verified firmware/UI bridge or a supported extension from upstream. The existing experimental native-panel firmware in the development workspace has **not** been installed or validated on v14 and is not included in this repository.

## Next integration gate

After a real-radio HTTPS result, integrate downloads into tracker startup with offline fallback, fresh-epoch validation and durable file replacement. Audit v14's radio-state callback and all existing GPS/CI-V/session bindings before enabling v14 tuning. The main-screen UI bridge is a separate task; neither feature is claimed complete by this probe.
