# Validation scope

This is an experimental radio application, not a certified or Icom-supported product.

## Observed on a real IC-705 during development

The operator reported:

- Successful launch through SQ6EMM v8 Apps and live GPS coordinates/fix.
- Verified RX tuning, A/B FM/SPLIT setup, and increasing completed correction cycles.
- Software-confirmed uplink corrections during operator-controlled TX.
- Successful return of original settings after STOP in the predecessor standalone build.

Version 0.15.0 is the rename to `SAT.app` / `SAT.CFG` / `SAT.TLE` and the displayed name `ISS tracker`. Its four controller/session object files were byte-identical to the preceding build when checked locally. That is useful regression evidence, but does not turn it into an independently hardware-tested release. The renamed files still need device confirmation.

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

## 0.21.0 loader deadline fix

Pinned v8 loader inspection: the ordinary STOP watchdog at 0x2037e138–0x2037e15c compares elapsed native ticks with 0x0bb8 (3,000), then terminates the app and sets status 6. Its native-I/O-busy field suppresses this check while a wrapped call is active. This explains why 0.20.0's six intentional 750 ms pauses exceeded the grace period irrespective of radio-command latency. It does not prove a separate deadlock never existed.

Thirty-eight host test methods pass. The display integration test now requires zero sleeps, zero SD log operations, preserved FM/TX gating and restored owned cache. Main STOP still ends radio control before display teardown and verifies radio restoration afterward; its poll interval is 10 ms. Combined hardware verification remains pending.

The actual compiled 0.21.0 ARM controller/session code was also exercised with native v8 display-cache notification, mocked GPS/orbit/SD and simulated CI-V ACK/readback. Two cases began on VFO A with SPLIT off and VFO B with SPLIT on. Both returned normally 310 ms after STOP and restored original A/B frequencies, modes, selection, SPLIT and text cache; observed stack use was 6,640/8,192 bytes. This is simulated response timing, not a measured hardware shutdown time.

## 0.22.0 STOP investigation

The owner reports a full-radio freeze on STOP with 0.21.0. The earlier 310 ms simulation therefore did not establish a hardware fix. The FRQ debug build may have avoided the critical cleanup because its intentional waits triggered force termination. The root cause remains unconfirmed.

This candidate removes native UI notifications, forced redraw writes and replay of saved visibility/update flags from STOP. It clears owned characters only and retains the normal 50 ms radio-restore poll interval. Host tests cover no native notification/log/sleep during display teardown and preserve all fields other than owned characters, including native content after ownership changes. Thirty-eight methods pass.

Compiled ARM tests execute actual display publication and radio-control/session code with private v8 firmware, mocked orbit/SD/clock and CI-V responses. Two A/B and SPLIT configurations each restored original frequencies, modes, selection and SPLIT, blanked owned text, and returned 1,550 simulated ms after STOP. Each run executed 11 native notifications while running and asserted zero native notifications after STOP. Observed stack use was 6,640/8,192 bytes. These checks cannot reproduce real UI scheduling, native rendering or hardware deadlocks. On-radio confirmation is required.

### Hardware feedback — 2026-11-21

After installing SAT 0.22.0, the owner confirms that STOP no longer freezes the radio and that everything works as expected. This confirms the reported outcome on that IC-705/v8 setup; the exact native UI failure mechanism has not been isolated.

## 0.23.0 configurable multi-satellite tracking

Default profiles are ISS, SO-50, AO-91, AO-123 and SO-131, ordered by priority in SAT.CFG. The current AMSAT AO-91 page specifies carrier-operated/no CTCSS, so its default is 0; SO-131 also uses 0. ISS/SO-50/AO-123 use 67 Hz. SO-50 activation remains operator-managed.

Forty-five host test methods pass. New scenarios cover strict configuration, duplicate/invalid/too many entries, overlap priority and earliest future AOS, cache selection, partial download success, offline use, partial writes, readback failure, cancellation, and cooperative prediction for five real historical records. Independent Python SGP4 comparisons cover all five satellites over ±24 hours at four locations (including the generated near-earth-only build). OMM/CSV output agrees within 1 m range, 1e-5 km/s range rate and 1 Hz Doppler in that grid. Malformed IDs, UTC dates, non-finite/scientific fields and unsupported orbits are rejected. Historical fixtures are actual AMSAT/CelesTrak records retrieved 2026-11-21; synthetic epoch changes exist only inside isolated refresh tests.

Private-v8 ARM tests use actual application/controller/session code and native display notification, with mocked orbit geometry/clock/card and CI-V replies. Each default profile reaches its expected A/B frequencies, FM/SPLIT and configured tone state. A simulated AO-91 TX case applies moving UHF uplink corrections using only allowed read requests and frequency writes; no PTT or tone/VFO configuration commands occur during TX. Priority tests switch SO-50 to ISS, including a deferred switch after PTT release. Another case switches AO-91 to higher-priority SO-131, swapping bands and clearing the TX-tone enable. The original radio snapshot survives these changes. Normal STOP restores original frequencies/modes/selection/SPLIT, clears owned text without native notifications, and returns in 1,550 simulated ms.

Separate ARM startup tests use real TLE/OMM parsing and SGP4 initialization with simulated HTTPS/card I/O: fresh cache makes zero connections; refreshing five profiles makes five connections/closes, writes and validates the alternate complete bundle, and preserves the original source. STOP before completed prediction makes no CI-V commands. Maximum observed stack use is 4,616 of 8,192 bytes.

Multi-satellite hardware operation has not yet been verified. Earlier 0.22 STOP behavior was confirmed by the owner; this version retains its no-native-redraw cleanup and single-session restoration design. Real scheduling, TLS peer behavior, RF changes and actual contact success remain outside these simulations.

## 0.24.0 fixed status pages and startup busy retry

The owner reported `RESTORE/SAVE: CIV BUSY` after startup in 0.23.0. This does not establish which native busy guard was set. Snapshot submission previously treated even one busy response as a permanent failure. The new capture-only retry is bounded to 1.5 seconds for a command; it never clears foreign transport state. Unsupported submission remains a separate immediate error. The selected-VFO-change flag is now set only after a selection was actually queued. STOP before any accepted selection therefore makes no unnecessary restore selection.

Forty-seven host methods pass. Fixed native pages are at most 13 characters and alternate every 1,500 ms; tests cover the 1,499/1,500 boundary, stable content, state changes, TX/non-FM gating, timer wrap, invalid overlong pages and cleanup without native notification, sleeps or SD writes. Compact UTC/date and remaining-minute pages use the actual tiny formatter. Real snapshot-state-machine tests cover transient busy, busy between read steps, permanent busy, STOP during initial busy, unsupported bindings and unchanged fail-fast restore behavior. No native busy bytes are reset. These tests do not establish a hardware cure for persistent CI-V busy.

Private-v8 ARM checks additionally inject a foreign busy UART byte at the first SAVE selection. A 400 ms busy period clears only through the simulated foreign transport; SAVE then completes and normal tracking/restoration succeeds. A persistent busy byte remains set, no CI-V frames are published, the original VFO is retained, and the Apps window shows the full busy snapshot. Priority switching and switching deferred across TX also pass with the new pages. Normal STOP still takes 1,550 simulated ms with no native notifications during cleanup. Maximum observed stack use in these controller cases is 4,472/8,192 bytes. This validates the application paths under simulation, not the cause of the owner's hardware busy condition.

## 0.25.0 read-only transport trace

The owner confirms the busy state occurs on the first start after power-on. The image shows Q0/R0/C05 and UART 0101 followed by zeros. This is insufficient to decide whether the state predates the app, follows an owned completion, or belongs to another native producer. No unknown state is reset. The diagnostic adds BOOT/FIRST/last-REPLY/last-FREE/last-BUSY/FAIL snapshots in app RAM and writes SATDBG.LOG only after capture failure. The Apps view adds phase/token/submission counts and labels unconfirmed frequencies as targets.

Forty-seven host methods still pass. Private ARM tests inject exactly Q0/R0/C05/U0101 at the first snapshot attempt. Transient occupation recovers with normal tracking and 1,550 simulated ms STOP, and produces no log. Persistent occupation publishes no CI-V frames, leaves the foreign parser/UART bytes unchanged, retains the original VFO, and writes BOOT/FIRST/BUSY/FAIL with the correct raw state and phase S0/W0/A0/F6/R-1/OK0. These tests validate trace fidelity and unchanged behavior, not a hardware fix. See CI-V-DEBUG.md for the format.

A further ARM case injects the same foreign busy state after the first SELECT reply has been correlated and released. The log contains all six snapshots, with REPLY showing ready/owned state, FREE showing restored idle fields, and BUSY/FAIL showing the newly occupied state. The app publishes no further command, preserves the foreign bytes, and reports incomplete restoration rather than success. This also verifies the new target-frequency labels.

## 0.26.0 RX-line refresh candidate

The owner reports that the native RX line scrolls away and the frequency moves before the next status page. This candidate republishes the current padded 13-character page every 500 ms while preserving the separate 1,500 ms page deadline. There is no intentional blank publication between pages. TX/non-FM gating, ownership checks, startup trace and no-native-notification STOP are retained. Forty-seven host methods pass, covering refresh at 499/500 ms, unchanged page timing, timer wrap and cleanup. Native animation and frequency layout require hardware verification; this is not a confirmed fix for that behavior.

## 0.27.0 native RX suffix candidate

Hardware feedback: WAIT FOR GPS is stable in 0.26, but alternating satellite/date pages still blink. Heartbeat alone has therefore not resolved the problem. Private pinned-v8 inspection of 0x2004b834–0x2004b864 shows separate 13-byte name and 20-byte tail copied to the rendered record, with lengths at cache +0x3b/+0x50; 0x2004ba0c adds both lengths plus a separator for the scroll length. SAT previously retained the tail of a prior native message. Publication now blanks that tail and sets its length to zero, including on page replacement. The diagnostic probe's legacy restoration saves/restores those extra bytes; production STOP still only blanks owned name characters without native notifications. A newly nonempty native tail causes publication/release to yield ownership. All 47 host methods pass, including stale-tail removal and preservation of a later native producer's data. Hardware confirmation that this removes blinking is pending; the owner's actual suffix contents have not been captured.

## 0.28.0 RX refresh margin candidate

The owner reports occasional blinking remaining after 0.27, especially GPS-to-prediction transitions and tuning. WAIT FOR GPS is stable. The suffix change reduced symptoms but has not established a complete fix. This candidate reduces the normal publication interval from 500 to 250 ms while retaining the separate 1,500 ms page deadline, existing FM/RX gating and unchanged STOP. Native pinned-v8 status setup at 0x2004b050 and related routines contains 250/500-count hold transitions; equality between an app refresh deadline and a native transition is a possible timing issue, not a proven cause or an established conversion of native counts to milliseconds. The tighter refresh supplies additional opportunities during normal app loops; it cannot guarantee continuous display during native UI changes or blocking operations. Forty-seven host methods pass, including the updated 249/250 ms refresh boundary. Hardware animation confirmation remains pending.

The compiled 0.28 ARM image also passes the existing simulated transient/permanent CI-V busy cases: normal restoration completes in 1,550 simulated ms with no STOP UI notification, and permanent foreign busy is preserved with a diagnostic log and no commands. Observed stack use is 4,016/8,192 bytes. These tests do not emulate the actual display animation.

## 0.29.0 single RX message

At the owner's request the RX line now shows only one status, refreshed every 500 ms. Waiting is WAIT followed by the chosen satellite; a visible pass is NAME PASS. No date/time or remaining-time page rotates onto the main screen, including when a legacy diagnostic caller supplies a second message. The Apps screen retains full timing information. Errors and RX-wait states also use one message. Host coverage checks repeated waiting over four seconds, 499/500 ms and timer wrap, direct state replacement, ignoring secondary pages, FM/TX gating and unchanged notification-free STOP. Hardware layout verification remains pending.

## 0.30.0 native RX hold and remaining minutes

The owner confirms a single WAIT message works, but still sees blinking every one or two seconds, and requests disabling scrolling. This version renews the pinned-v8 RX hold instead of continually enqueueing a fresh native message. It does so only while SAT already owns the exact name, the native tail length is zero, native state is 7, its RX-variant bit is set, and its hold duration is nonzero. It zeros the per-radio elapsed counter at 0x20381324 under the existing short interrupt guard. Other native states and expired holds follow the original message-admission path. Additional instruction signatures and the timer literal are checked before enabling publication. STOP neither writes this timer nor calls native UI notification. New native content still ends ownership.

Visible-pass status is now NAME Nm, using rounded-up remaining minutes, --m while LOS is unknown, and 0m at/after predicted LOS. Full details stay in Apps. Forty-seven host methods pass, covering hold renewal, expired hold, non-RX/non-status states, foreign content, STOP preservation and minute boundaries.

A private Unicorn check runs the compiled SAT publication/release functions together with the actual pinned-v8 hold setup (0x2004b050) and hold-check branch (0x2004b5a4). Forty simulated renewals retain the native hold and never reach its scroll-advance call; without renewal the same native check expires and reaches that call. Native drawing/scroll helper calls are intercepted. This establishes the tested branch behavior, not on-radio layout stability. The release preserves the timer and all cache fields except owned characters. Existing compiled-controller transient/permanent CI-V cases also pass: normal STOP restores original radio state in 1,550 simulated ms, permanent busy remains untouched with its log. Hardware confirmation is pending.

## 0.31.0 read-only UI investigation

Hardware feedback rejects 0.30 as a scrolling fix: the row still moves and sometimes blinks. The isolated hold-branch test did not cover the complete native presentation path. Inspection also identifies a separate scrolling runtime group and rendered offset/length. Rather than resetting more unobserved fields, this build records a read-only last-main-screen snapshot in two Apps rows. It observes native state/style/hold/event flags, elapsed counter, group 4 flags/index and peak, group 5 index, rendered offset and output length. Snapshot acquisition changes no native data or notification cadence and performs no SD I/O. Opening Apps freezes the last main-screen sample; radio errors retain display priority. Observation remains available if publication yields to new native content. The build retains the 0.30 behavior and does not claim to fix scrolling. Forty-seven host methods pass, including diagnostic output, frozen last-main sample and timer preservation.

The final compiled 0.31 image also passes the simulated transient/permanent CI-V controller cases: original settings restored in 1,550 simulated ms with no STOP UI notification, or persistent foreign busy left unchanged with its log. The normal case populates diagnostic rows; error rows retain their previous content and priority. The full native GUI scheduler is not emulated by those controller cases.
