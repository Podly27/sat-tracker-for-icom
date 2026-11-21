# Building and repository layout

The build is for the standalone ICAP ABI3 application, not radio firmware.

## Build

Use Python 3.9+ on a host supported by the pinned `ziglang` Python package:

```sh
python3 -m venv .venv
source .venv/bin/activate
python -m pip install -r requirements.txt
python tools/build.py
```

On Windows, activate with `.venv\Scripts\activate` and use `python`. macOS is the development host; other hosts need their own build verification.

The script targets ARMv7-A Cortex-A9, ARM mode, softfp/NEON, links at `0x20831000`, and packages a 64-byte ICAP header. It checks the code+BSS load-window budget and writes the CRC. Declared stack size is 8192 bytes.

Zig's target libc/compiler archives are generated in the isolated `.cache/` directory. `build/` contains objects, ELF, `SAT.app` and a JSON size/hash report. Neither cache nor intermediate binaries are committed.

Only the target executable is built; it is never run as a Linux program. The app uses the ICAP/native radio bindings, not Linux syscalls or a host runtime. No supplied Icom firmware is needed to build this repository.

The prebuilt `sd-card/IC-705/Apps/SAT.app` is the version 0.36.0 clean-display artifact. Hashes in `sd-card/SHA256SUMS.txt` identify shipped files. A fresh libc archive/link can produce different function ordering and a different final hash; **bit-for-bit reproducibility is not claimed**.

## Tests

```sh
python -m pip install -r requirements-dev.txt
python -m unittest discover -s tests -v
```

The TLE tooling tests use only the standard library. Orbit tests compile the actual C orbit/SGP4 module with a host C compiler and compare it to Python SGP4. They are skipped if optional dependencies or a host compiler are absent.

Device/Unicorn tests from development use privately supplied Icom images and are not bundled. This repository does not download copyrighted firmware to run tests. See `VALIDATION.md` for the distinction between software checks and hardware observations.

## Layout

```text
src/                       Application, pinned GPS/CI-V/session bindings
vendor/sgp4/               Unmodified upstream SGP4 C source and licence
sd-card/IC-705/Apps/        Prebuilt SAT.app and reserved SAT.CFG
                            SAT.TLE is generated locally and git-ignored
tools/build.py             Standalone application builder
tools/update_tle.py        Validated ISS TLE download/extraction
tests/fixtures/            Historical orbit test data, not for installation
docs/                      English operating/build/TLE notes and preview
```

The experimental custom native-panel firmware and its app bridge are intentionally outside this repository. Do not add firmware `.dat`, decoded `.bin`, full flash dumps, manuals, local serial logs or photos with observer coordinates.

## Multi-satellite memory budget

Version 0.23 uses byte-oriented freestanding string/memory helpers instead of large target-libc vector routines. The build generates `SGP4-near-earth.c` from the unmodified vendored source, removing only deep-space branches; both TLE and OMM entry points reject orbits outside 80–110 minutes. The generated artifact is not a general deep-space SGP4 library. Tests compare this variant against independent Python SGP4 for all five default satellites. Eight fixed profile slots and two Orbit workspaces avoid heap allocation; the same ICAP code+BSS and stack limits apply.
