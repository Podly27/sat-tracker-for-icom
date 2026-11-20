# Third-party notices

## SGP4

`vendor/sgp4/SGP4.c` and `.h` originate from
[aholinch/sgp4](https://github.com/aholinch/sgp4), commit
`552cb1489a52c3023ae70cb6c7e239e84c5950fe`.

The upstream [Unlicense](vendor/sgp4/LICENSE) and provenance
[SOURCE.txt](vendor/sgp4/SOURCE.txt) are preserved. Project licensing does not
replace the upstream licence.

## Linked C/compiler runtime

The prebuilt application was built with Zig's ARM musl/libzigc/compiler runtime
archives. Relevant bundled notices are preserved in
[licenses/musl-COPYRIGHT.txt](licenses/musl-COPYRIGHT.txt) and
[licenses/Zig-MIT.txt](licenses/Zig-MIT.txt). musl's notice includes the
additional attributions for individual implementations. The SDK/package used
for building remains a separate dependency, not vendored toolchain code here.

## Alternative firmware

The required ICAP runtime and alternative firmware are the separate work of
[Dawid SQ6EMM](https://github.com/sq6emm/icom-custom-firmware). This repository
uses that interface but does not distribute the runtime, patch, bootstrap or
any Icom firmware image. It is not an upstream SQ6EMM release or endorsement.

## Icom resources and privacy

No Icom firmware, manuals, decoded images, flash dumps or proprietary UI assets
are included. Native addresses and small signatures in the app are
version-specific interoperability bindings.

The screen preview is project-authored SVG, not an Icom screenshot. No supplied
photograph, precise observer position or local hardware log is included.
