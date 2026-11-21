# Validation scope

This is an experimental radio application, not a certified or Icom-supported product.

## Observed on a real IC-705 during development

The operator reported:

- Successful launch through SQ6EMM v8 Apps and live GPS coordinates/fix.
- Verified RX tuning, A/B FM/SPLIT setup, and increasing completed correction cycles.
- Software-confirmed uplink corrections during operator-controlled TX.
- Successful return of original settings after STOP in the predecessor standalone build.

Version 0.15.0 is the rename to `satellites.app` / `satellites.CFG` / `satellites.TLE` and the displayed name `ISS tracker`. Its four controller/session object files were byte-identical to the preceding build when checked locally. That is useful regression evidence, but does not turn it into an independently hardware-tested release. The renamed files still need device confirmation.

No independent spectrum/frequency-counter measurement of active-TX Doppler was performed by the development agent. Radio/CI-V readback and a transaction counter do not establish RF accuracy.

## Software checks

Development used compiled ARM execution with simulated ICAP callbacks, CI-V consumer responses and GPS/radio state, including STOP/restoration, TX deferral, VFO mapping and foreign-state preservation. Some original native getter/parser functions were exercised from the user's private firmware image.

Those tests do not emulate the entire IC-705, native scheduling, SD card, power lifecycle, final LCD hardware or RF chain. They are not included as self-contained tests here because the firmware image is not distributed.

Public tests cover TLE input/age/checksum/atomic replacement and the actual C orbital implementation against a Python SGP4 reference. Building checks the ICAP format, CRC and load-window size. None of these makes a different alternative firmware automatically compatible.

## Practical first use

Before relying on tracking, record your initial VFO frequencies/modes/SPLIT, launch the app, check GPS/UTC and corrected frequencies, then STOP and compare the restored settings. Confirm receive operation before an operator-controlled TX check. Do not start or stop a firmware update as part of an application test.

The original issue where power-on required battery removal after an app session was investigated during development. Later standalone builds were reported to shut down/restart normally after completed STOP. This history is not a guarantee for a different firmware, app build, or interrupted STOP.

## 0.16.0 startup changes

The ARM application builds within the ICAP memory budget. Thirteen host tests pass, including a mocked startup state machine exercising six scenarios: fresh, two-day-old, expired and missing elements, missing GPS, and STOP during initialization. These tests assert that control starts only after pass search and restoration is skipped when no radio settings were captured. They do not exercise real GPS, networking or RF hardware. The local Unicorn runner exited abnormally on this host, so no ARM-emulation success is claimed for this version. Device verification remains pending.

## Experimental network probe

Twenty-seven host test methods pass. Network tests exercise HTTP fragmentation/framing, checksums, duplicate rejection and mocked transport cleanup for success, offline, connect/send/receive failures, cancellation, fallback and timeout. Static audits match the pinned v8/v14 callback tables. The ARM probe builds with 56,420 code bytes and 2,972 BSS bytes inside the loader memory window. No device HTTPS, certificate-authentication or native display integration result is claimed.

A live CelesTrak response (168-byte body) was downloaded on the development computer and accepted by the same C streaming parser with 17-byte fragmentation. This tests server-format compatibility, not the radio socket/TLS implementation.

The native-dialog probe adds a 28th host test method with five scenarios (valid position, missing TLE, expired TLE, no GPS and popup busy). The mock checks seven popup lines, width limits and one-time opening. These tests do not emulate the real native widget or LCD.

## Second diagnostic round

Twenty-nine host test methods pass. The FRQ test covers missing/stale elements, GPS loss, unsupported bindings, menus, non-FM and active TX. The native popup was reported to appear on hardware; the first HTTPS probe was reported to fail connection, so successful radio TLE retrieval is not claimed.

The compiled FRQTEXT ARM image was also run locally with the owner's private v8 firmware under Unicorn outside the sandbox (JIT allocation is blocked inside it). The real Icom GPS helpers and text-wrapper instructions ran. GPU primitives and ICAP callbacks were mocked. Main/RX produced the expected text arguments at x=12/y=96, no CI-V frames, and only the two native UI dirty-flag writes on STOP; menu/TX cases produced no drawing or native writes. Observed stack use was 1,424 bytes. This does not test real GPU scheduling, LCD output or native repaint completion.

## 0.18.0 integrated refresh and FRQ test 3

Both DNS and numeric-IP HTTPS probes are reported to have returned valid fresh ISS elements on the owner's radio. The fixed native text popup is also reported to display correctly. FRQ test 2 calculated a position but displayed nothing under the main frequency. These are user observations, not agent-operated hardware tests.

Thirty public host test methods pass. New store tests cover fresh/stale/missing/corrupt cache, selecting the fresher slot, offline/expired data, failed or invalid/stale downloads, partial writes, write/readback failure and STOP during download/save. The protected valid source remains byte-identical.

The compiled 0.18 ARM startup ran with native v8 GPS helpers and simulated TLS/SD/ICAP callbacks. A fresh-cache run made no connection; a stale-cache run fetched, validated and wrote the alternate slot, closed the network handle and preserved the original file. STOP before pass-search completion returned without CI-V or radio-state writes. Observed stack usage was 5,240 bytes within the 8,192-byte allocation. No real SD controller/TLS peer verification was exercised. The integrated on-radio refresh and file write remain pending.

FRQ test 3's compiled ARM code passed main/menu/TX checks with native drawing entry points intercepted: it targets layer 12 and passes x=12/y=96; no CI-V occurs. The UI diagnostic counters are new. Actual rendering/scheduling on the LCD remains pending.

## FRQ test 4

Thirty-two host test methods pass, including the actual tiny formatter and display-cache ownership/restoration. The final ARM probe ran main-FM, menu-FM, TX and packed-70 cases. The original UI notification executed after cache updates; STOP restored the previous text. TX/non-FM cases published nothing. No CI-V frames or packet-mailbox/pending-flag writes occurred. Observed stack usage was 1,504 bytes. Hardware visibility remains unverified.

FRQ test 5 adds a stale-cache scenario: packed cache 0x70 with current APRS mode group 7 must publish, whereas actual non-FM group 4 and TX must not. Thirty-two host test methods pass with the additional scenario; ARM cases exercise the same distinction. Device text visibility remains pending.

## 0.19.0 pass status and hardware feedback

The owner confirms that ISS tracking works and that FRQTEXT test 5 finally shows main-screen text. However, STOP in FRQTEXT freezes the radio; power-off still works. The owner explicitly identifies FRQTEXT only, not ISS. Earlier simulated display cleanup did not expose this hardware failure. The native cache backend remains excluded from the main tracker.

Thirty-seven host test methods pass. New tests use the actual tiny formatter and cover UTC day/month/year rollover, leap day, rounded-up remaining minutes, expired AOS and no-pass/search states. The ARM build uses 77,364 code bytes plus 3,640 BSS bytes (81,004 total, within the 81,920-byte loader budget), with an 8,192-byte stack. Radio-control/restoration source is unchanged. These new display states still need an on-radio check.

The compiled 0.19.0 ARM startup also passed fresh-cache and stale-cache refresh scenarios with mocked network/SD callbacks: respectively zero and one HTTPS connections, alternate-slot storage preserved the original cache, and STOP during the initial search returned with no CI-V commands. Observed stack use was 5,240 bytes. This does not validate the real network/SD driver or the new pass-status LCD layout.

## 0.20.0 integration

The owner reports FRQTEXT test 6 no longer freezes on STOP. This does not establish the original cause, nor independently validate the combined app. The integration retains file-close checkpoints and 750 ms pacing during display cleanup, with CI-V control finished before display teardown and radio restoration after it.

Thirty-eight host methods pass, including actual cache publication/restoration, scrolling, FM/TX gating, partial log writes, closed checkpoints, no-op cleanup before publication, and STOP ordering. Native binding and hardware-driver behavior remain outside those host tests. The compiled ARM fresh/stale startup tests use the renamed cache paths and pass with 6,640 bytes observed stack use within 8,192 allocated. HTTPS/SD are mocked in that check. The startup HTTP parser moved from persistent BSS to startup stack to fit the fixed loader memory budget; no target memory limit was increased.
