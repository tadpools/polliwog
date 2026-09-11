# polliwog roadmap

growth stages, in order. versions are calver (YYYY.N.P); a milestone
may break, a patch may not. the naming governance is inherited from
tadpole: versions are growth stages. each milestone has a trigger that
must be true before it starts, and a gate that must be true before it
ships.

the sitting-level plan for the current runway lives in near-term.md
(working memory, replaced at each milestone sitting). this file holds
the milestone-level truth and the scope boundaries.

## 2026.1.0 - spawn (bootstrap)

the repo exists; nothing public does yet. the gate is infrastructure,
not code.

- dev/ docs complete (this folder)
- ci: build matrix (windows/ubuntu/macos), clang-format check,
  warning-free gate, catch2 wired
- .github templates: issues, pull requests, contributing, security
- the Backend concept written and compile-checked with a fake backend
- mit license, changelog scaffold, readme listing what does not exist

trigger: this document. gate: a cold clone configures and builds an
empty library with zero warnings on all three runners.

## 2026.2.0 - hatch (zlib mvp)

the library becomes useful. zlib only, honestly labeled.

- format.hpp, error.hpp, squeeze.hpp, stream.hpp, umbrella header
- one-shot squeeze/swell over spans: zlib, gzip, raw_deflate
- compressor/decompressor: push/pull/finish, sync_flush only
- bound() via compressBound_z/deflateBound_z; max() documented where
  the backend cannot bound
- determinism contract: gzip mtime=0, os=unknown; pond fixtures pin it
- memory-table allocator-counting tests
- google benchmark on the silesia corpus (sources verified live
  2026-09-11, research-log row 18: 12 files, official per-file md5s
  published by the corpus page; enwik8 via mattmahoney.net, row 19).
  the pond manifest records each file's md5 at download time - the
  checksums are computed, never invented

trigger: spawn shipped. gate: round-trip property suite over the pond
corpus, every error kind reachable by a test, benchmarks recorded.

scope boundary (hatch):

- in: zlib/gzip/raw_deflate one-shot; push/finish streaming;
  sync_flush only; bound(); determinism fixtures; allocator-counting
  tests; benchmarks
- out (decided, logged): gzip multi-member concatenation (decision
  D10), flush modes beyond sync_flush (decision D9), dictionaries,
  pull-based views, zstd (next milestone)

## 2026.3.0 - tadpole (zstd + the error taxonomy freezes)

- zstd backend behind POLLIWOG_WITH_ZSTD, stable api only
- error taxonomy frozen: kinds are additive from here on
- libfuzzer/afl++ smoke fuzzing on every pr; deep fuzz weekly
- brood design note (the only writing a froglet feature gets this
  early - it cannot name its adoption step until the streaming layer
  is Stable)

trigger: hatch shipped with a green matrix. gate: fuzz corpus survives
a week of scheduled runs with zero unreported crashes; zstd and
zlib-only builds both pass the full suite.

scope boundary (tadpole):

- in: zstd stable api only; the frozen error taxonomy; fuzzing wired;
  the brood design note (writing only, no brood code)
- out: zstd dictionaries (parking lot), brood implementation,
  libdeflate shim, any new error kind that is not additive

## 2026.4.0 - froglet (brood, mmap, zlib-ng, lz4)

- brood.hpp: parallel batch squeeze over jobs, std::jthread pool,
  deterministic merge, per-job errors
- files.hpp: mmap-backed input where the os provides it (posix +
  windows), plain reads elsewhere
- zlib-ng behind POLLIWOG_WITH_ZLIB_NG (determinism across reuse
  verified by fixtures)
- lz4 backend: frame format Growing, block format Experimental

trigger: tadpole shipped; brood design note reviewed against the
adoption ladder. gate: brood output byte-identical to serial output on
the pond corpus at every thread count from 1 to hardware_concurrency.

scope boundary (froglet):

- in: brood (std::jthread, deterministic merge, per-job errors);
  files.hpp mmap input; the zlib-ng option; lz4 frame (Growing) +
  block (Experimental); the clang/libc++ jthread floor decision
- out: coroutine reader, bzip2/lzma backends, any brood job
  granularity finer than one one-shot call

## 2027.1.0 - frog (1.0-quality)

- vcpkg and conan packaging; docs site from the headers (doxygen or
  equivalent, voice rules still apply)
- abi posture documented: source-and-cmake consumption, per-toolchain
  binaries
- lz4 block format decision: promote or cut
- stability tiers audited: anything still Experimental is named in the
  readme honestly

trigger: froglet shipped and consumed by at least one real project
(ours count). gate: packaging smoke-tested on the three runners plus a
vendored build; the changelog reads like a 1.0, not a wish list.

## the parking lot (ideas, not in progress)

a feature enters in-progress only by naming its step on the adoption
ladder (philosophy.md). everything below is an idea:

- libdeflate backend shim (if the benchmarks justify it)
- zstd dictionaries (needs a design note; stable api only)
- coroutine reader over the streaming layer
- bzip2/lzma backends (only if a real caller asks; the concept makes
  it cheap, cheap is not a reason)
- frogspawn: archive formats as a sibling package, never core

## what will not happen

- archive formats in core
- encryption
- io in core
- plugin/runtime backend registration
- a proprietary container format - output is the backend's wire
  format, byte for byte, always
