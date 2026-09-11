# polliwog versions and dependencies

pinned backend versions, the toolchain floor, and the upgrade policy.
this file is the one place backend versions are pinned. researched
2026-09-11 from the upstream release pages; re-verify at each
milestone sitting.

## backends

| backend  | pinned version | released    | license | role                                        |
|----------|----------------|-------------|---------|---------------------------------------------|
| zlib     | 1.3.2          | 2026-02-17  | zlib    | default backend; rfc1950/1951/1952 containers |
| zlib-ng  | 2.3.3          | 2026-02-03  | BSD-3   | optional fast fork behind the same shim     |
| zstd     | 1.5.7          | 2025-02-19  | BSD-3   | modern backend (tadpole milestone)          |
| lz4      | 1.10.0         | 2024-07-22  | BSD-2   | speed floor backend (froglet milestone)     |

### why these

- **zlib 1.3.2** is the first release after a formal security audit
  (the "7A" audit): hardened crc32_combine, inflateCopy/deflateCopy
  state fixes, zeroed inflate state, a rewritten cmake build, and new
  `_z`-suffixed entry points (compress_z, uncompress_z,
  compressBound_z, deflateBound_z) that accept size_t-scale lengths
  for large inputs. the shim prefers the `_z` entry points when
  present and records the preference in one place.
- **zlib-ng 2.3.3** keeps deflate deterministic across stream reuse
  (fixed in 2.3.3), which matters because the determinism contract is
  load-bearing here. it stays off by default: same wire format, faster
  engine, different binary - an option, not an opinion.
- **zstd 1.5.7** improved small-block compression speed 10-20% (the
  sizes that matter for brood jobs), fixed the long-running 32-bit
  session bug, and made multithreaded cli default. dictionaries are a
  parking-lot feature; the shim wraps only stable api.
- **lz4 1.10.0** brought multithreading and a stable dictionary api.
  the shim targets the frame format for wire safety, with block
  format exposed as Experimental for callers who own framing.

### upgrade policy

- backend bumps are patch-level (YYYY.N.P) when the wire format and
  determinism contract are unchanged - the changelog names the bump
  and the fixtures prove the contract held.
- a backend bump that changes output bytes, error surfaces, or drops
  a compiler floor is a milestone, never a patch.
- the shim compiles a static_assert against the backend's version
  macros, so a mismatched vendored copy fails the build with a message
  naming versions.md.

## toolchain floor

verified against live sources 2026-09-11; provenance rows 5 and 6 in
research-log.md.

| tool            | floor                  | note                                        |
|-----------------|------------------------|---------------------------------------------|
| cmake           | 3.28                   | deliberate minimum; current release is 4.4.3 (2026-08-25); ci uses current 4.x |
| msvc            | 19.38 (vs2022 17.8+)   | expected monadic ops need 19.36 - floor above it. current toolchain is VS 2026 18.6 / compiler 19.51 (Build Tools 14.51, 2026-06-23) |
| gcc             | 13                     | expected monadic ops need 13 exactly - floor meets it |
| clang           | 17                     | expected monadic ops need 17; jthread partial on libc++ until 20 (see below) |
| apple clang     | 16 (xcode 16)          | expected + monadic complete at 15.0.0; floor kept conservative |
| python          | none                   | no build-time python. scripts are cmake     |

### per-feature minimums (why the floors are what they are)

the floors cover the features polliwog loads on, not all of C++23
(full C++23 needs roughly gcc 16 / clang 19-20 / msvc 19.44 per
cppreference, fetched 2026-09-11):

| feature              | minimum (gcc / clang / msvc / apple) |
|----------------------|---------------------------------------|
| std::expected        | 12 / 16 / 19.33 / 15.0.0              |
| expected monadic ops | 13 / 17 / 19.36 / 15.0.0              |
| std::span (C++20)    | 10 / 7 / 19.26 / 10.0.0               |
| std::jthread (C++20) | 10 / 20* / 19.28 / n/a                |

\* clang libc++ lists jthread as partial until 20. honest constraint,
recorded: the brood milestone (froglet) either requires clang 20 for
libc++ builds or ships a documented fallback. tracked in
research-log.md open questions.

c++26 remains draft (gcc 16 partial: contracts, reflection); the
POLLIWOG_CXX26 option stays opt-in and never required.

## c++ standard

baseline is c++23. the load-bearing features: std::expected, concepts,
std::span/std::byte, std::jthread, std::string_view, exhaustive enum
switches. c++26 experiments (if anything earns interest) live behind a
POLLIWOG_CXX26 option and are never required.

the stable-abi question is answered honestly: c++ has no stable abi,
so polliwog publishes one for source-and-cmake consumption only.
binary distribution is per-toolchain, documented in the packaging
milestone (frog).

## dev-only dependencies (never leak into the installed target)

current releases verified live 2026-09-11 (research-log.md rows 10-11).

| tool              | floor  | current (2026-09-11) | job                                  |
|-------------------|--------|----------------------|--------------------------------------|
| catch2            | v3     | v3.16.0 (2026-08-25) | unit + round-trip + contract tests   |
| google benchmark  | v1.8   | v1.9.5 (2026-01-21)  | perf tracking in ci, silesia corpus  |
| clang-format      | 17     | 17+                  | layout, llvm-based repo config       |
| clang-tidy        | 17     | 17+                  | smell; config in repo root           |
| libfuzzer / afl++ | whatever ci | whatever ci     | fuzz smoke on every pr, deep weekly  |

exact dev pins live in the ci workflow files and are refreshed at
milestone sittings; this file records the floors.

## licensing story

polliwog is MIT. the backends keep their own licenses, all permissive
and redistribution-friendly. provenance, per the honesty law:
zlib's zlib license and lz4's BSD-2 were stated on pages fetched live
2026-09-11 (research-log rows 1 and 4); zstd's BSD-3 and zlib-ng's
BSD-3 are training knowledge pending verification against their
LICENSE files at the first packaging sitting. the docs carry a
license table naming each backend so a downstream vendor never has to
chase it - once each cell is verified.

## vendoring policy

a vendored copy of each backend ships in vendor/ as a fallback path
(FetchContent pulls the same pinned tags). vendored copies are exact
upstream snapshots, never patched. if a patch is ever needed, it gets
a decision record and an upstream issue first - we do not fork in the
dark.
