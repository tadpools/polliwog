# polliwog research log

every freshness claim in dev/ traces here. each row: what was fetched,
where, when, and what it changed in the docs. a claim marked "as of a
date" in any other file must have a row in this log. the honesty law
(AGENTS.md) applies: no claim without provenance; unverified things
are labeled unverified.

## verified this sitting (2026-09-11)

| # | source                        | how verified              | finding                                                                                                    | changed in docs |
|---|-------------------------------|---------------------------|------------------------------------------------------------------------------------------------------------|-----------------|
| 1 | zlib.net                      | live fetch                | current release zlib 1.3.2 (2026-02-17): 7A security audit hardening, new _z large-length entry points (compress_z, uncompress_z, compressBound_z, deflateBound_z), cmake rewrite, deflateUsed(), s390x crc vectorization | versions.md backend table; architecture.md shim preference for _z entry points |
| 2 | github.com/zlib-ng/zlib-ng releases/latest | live fetch | current release 2.3.3 (2026-02-03): deterministic deflate output after stream reuse (issue #2102 fix), minor bugfixes | versions.md; determinism-relevant, cited in froglet option |
| 3 | github.com/facebook/zstd releases/latest   | live fetch | current release 1.5.7 (2025-02-19): +10-20% small-block compression speed (4-128 KB), 32-bit session bug fix, multithreaded cli default (max 4 threads), --max mode; dictionary api status unchanged in release notes | versions.md |
| 4 | github.com/lz4/lz4 releases/latest         | live fetch | current release 1.10.0 (2024-07-22): multithreading, dictionary api promoted to stable, new level 2, async io decompression | versions.md |
| 5 | github.com/Kitware/CMake releases/latest   | live fetch | current release v4.4.3 (2026-08-25)                                                                        | versions.md toolchain note |
| 6 | en.cppreference.com/w/cpp/compiler_support  | live fetch + table extraction | std::expected (P0323R12): GCC 12, Clang 16, MSVC 19.33, Apple Clang 15.0.0. monadic operations for expected (P2505R5): GCC 13, Clang 17, MSVC 19.36. std::jthread (C++20, P0660R10): GCC 10, Clang 20 (17 partial), MSVC 19.28. full C++23 library coverage around GCC 16 / Clang 19-20 / MSVC 19.44. C++26 still draft (GCC 16 partial: contracts, reflection) | versions.md compiler floors with per-feature minimums; c++23 baseline confirmed, c++26 stays opt-in |
| 7 | github.com/mateidavid/zstr    | live fetch of readme      | header-only zlib iostreams wrapper; zlib errors throw exceptions; 1 MB internal buffers; auto-detects gzip (1f 8b) and zlib (78 01/9c/da) headers; zlib only; mit; miniz-compatible; readme points to bxzstr for bz2/lzma | differentiation.md row + honesty note on boost row |
| 8 | github.com/ebiggers/libdeflate releases/latest | live fetch | current release v1.26 (2026-08-22)                                                                          | differentiation.md |
| 9 | github.com/richgel999/miniz releases/latest  | live fetch | current release 3.1.2 (2026-07-01): zip-focused; recent releases add fuzz targets for zip validation/writing/compression, cmake 4 support, tinfl decompress loop fix | differentiation.md |
| 10| github.com/catchorg/Catch2 releases/latest   | live fetch | current release v3.16.0 (2026-08-25)                                                                        | versions.md dev tools |
| 11| github.com/google/benchmark releases/latest  | live fetch | current release v1.9.5 (2026-01-21)                                                                         | versions.md dev tools |
| 12| local: tadpole project docs                  | read from disk            | wetland naming governance, calver milestones, stability tiers, adoption ladder, decision records, lowercase voice | the entire repo shape |

## compiler floor reasoning (from row 6)

the polliwog floors are set above the per-feature minimums for the
features the library actually uses, not above full C++23:

| feature we load on | verified minimums (gcc / clang / msvc / apple clang) | polliwog floor |
|--------------------|--------------------------------------------------------|----------------|
| std::expected      | 12 / 16 / 19.33 / 15.0.0                                | covered by floor |
| expected monadic ops (and_then etc.) | 13 / 17 / 19.36 / 15.0.0              | covered by floor |
| std::span (C++20)  | 10 / 7 / 19.26 / 10.0.0                                 | covered by floor |
| std::jthread (C++20) | 10 / 20 (17 partial) / 19.28 / n/a                    | covered on gcc/msvc; clang libc++ jthread full at 20 - see open questions |
| concepts (C++20)   | 10 / 12 / 19.30 era                                     | covered by floor |

floors as set: gcc 13, clang 17, msvc 19.38, apple clang 16. honest
caveat: on clang with libc++, jthread is listed as partial until 20;
the brood milestone (froglet) must either require clang 20 for libc++
builds or provide the documented fallback. recorded now so the
constraint is not discovered late.

## verified this sitting, continued (2026-09-11)

| #  | source                                  | how verified | finding                                                                                                                                    | changed in docs |
|----|------------------------------------------|--------------|--------------------------------------------------------------------------------------------------------------------------------------------|-----------------|
| 13 | github.com/boostorg/iostreams (readme)   | live fetch   | boost.iostreams verified: "a framework for defining filters and attaching them to standard streams and stream buffers"; readme lists properties **C++03** and **"requires a link library"** (not header-only); BSL-1.0. the framework and c++03 claims in differentiation.md now stand on the project's own readme. the "throws" and "many" allocation cells remain design judgments, now labeled as such in the file | differentiation.md |
| 14 | github.com/madler/zlib/releases/latest   | live fetch   | cross-check: latest release tag is v1.3.2 (2026-02-17), matching zlib.net (row 1); notes confirm the 7A security audit and the cmake rewrite; 85 commits on develop since release - a future 1.3.3 is plausible, re-verify at next milestone sitting | research-log only (corroboration) |
| 15 | github.com/tmaklin/bxzstr (readme)       | live fetch   | bxzstr verified and added as the closest competitor: header-only C++11 iostreams wrapper over zlib/libbz2/liblzma/libzstd; fork of zstr; MPL-2.0; magic-number auto-detection; failure surfaces as failbit exception mask; googletest described as non-exhaustive; 65 stars | differentiation.md new row |
| 16 | github.com/microsoft/STL/releases/latest | live fetch   | current release: MSVC Build Tools 14.51, shipped in **VS 2026 18.6** (2026-06-23); current compiler 19.51.36122; their ci requires cmake 4.2.3, clang 20.1.8. the polliwog floor (19.38 / vs2022 17.8+) is unchanged; the current-toolchain note in versions.md now cites this row | versions.md |
| 17 | libcxx.llvm.org status pages             | live fetch (failed) | /cxx20.html and /status/cxx20.html both return 404 - the status pages have moved or been removed. the clang jthread column therefore cites only cppreference (row 6: partial at 17, complete at 20); the exact gap list between 17 and 20 stays an open question | research-log open questions revised |
| 18 | sun.aei.polsl.pl Silesia corpus page     | live fetch   | verified and pinned: 12 files (dickens, mozilla, mr, nci, ooffice, osdb, reymont, samba, sao, webster, xml, x-ray), total 211,938,580 bytes, and the page publishes the **official per-file MD5 checksums**. the pond corpus can adopt these checksums as fixture provenance at the hatch milestone | roadmap.md hatch bullet; open question resolved |
| 19 | mattmahoney.net/dc/                      | live fetch   | verified: enwik8 (100,000,000 bytes) is the Large Text Compression Benchmark / Hutter Prize text file; the site is live and references it. the direct file url and its checksum get pinned in the pond corpus at hatch download time, per the fixture provenance rule | roadmap.md hatch bullet; open question resolved |

## open questions (unverified - flagged honestly)

- **clang/libc++ jthread gap list** (row 17): known status is
  "partial at 17, complete at 20" per cppreference (row 6); the llvm
  status page 404s, so the exact remaining gaps are unenumerated.
  verify at the brood design note (tadpole milestone) - the decision
  is clang 20 floor for libc++ builds or a documented fallback.
- **apple clang 16 floor** (carried from first sitting): inferred
  from xcode 16 + the apple clang column showing expected/monadic
  complete at 15.0.0. conservative, not optimally low; lowering it is
  welcome with a test matrix to back it.
- **enwik8 file checksum** (row 19): source page verified; the file
  itself is not downloaded this sitting. the checksum is computed at
  hatch download time and recorded in the pond corpus manifest - per
  the honesty law, no checksum is invented here.

## standing rule

any doc sentence that says "as of", "current release", or cites a
version number must have a log row or an in-line citation with fetch
date. the log is append-only; corrections are new rows, not edits.
