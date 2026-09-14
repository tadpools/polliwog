# changelog

format follows keep a changelog 1.1.0; versions are calver (YYYY.N.P).
a milestone (YYYY.N.0)
may break; a patch (YYYY.N.P) may not. milestone names are growth
stages: 2026.1.0, 2026.2.0, 2026.3.0, 2026.4.0, 2027.1.0.

every entry is checkable against the tag it ships in. nothing is
described here before it has a test.

## unreleased - 2026.3.0

tadpole: zstd backend, error taxonomy freeze, fuzzing, brood design
note.

### added

- zstd backend (POLLIWOG_WITH_ZSTD, opt-in): raii handles over
  ZSTD_CCtx/ZSTD_DCtx, stable api only (zstd 1.5.7). zstd_level
  enum (fastest=1, default_level=3, best=19). one-shot squeeze/swell
  and streaming push/pull/finish. round-trip and determinism tests.
- cmake multi-backend build: zstd fetched via FetchContent with
  SOURCE_SUBDIR build/cmake. POLLIWOG_HAS_ZLIB / POLLIWOG_HAS_ZSTD
  as PUBLIC compile definitions. stub types for missing backends.
- error taxonomy frozen (D16): kinds are additive from here on.
  header doc updated; five of seven kinds have real test paths;
  out_of_memory and internal_contract_violation are untestable by
  design.
- fuzz targets (POLLIWOG_FUZZ, clang required): one-shot round-trip
  and streaming push/pull fuzz targets with libfuzzer. fuzz ci
  workflow: smoke 60s on pr, deep 5min weekly.
- brood design note (Experimental): parallel batch engine header,
  design only, no implementation. adoption step named (froglet).
- verified: zlib-only, both backends, zstd-only configs all pass the
  full suite.

## 2026.2.0 - 2026-09-12

zlib only, honestly labeled: one-shot and streaming
squeeze/swell over zlib, gzip, and raw_deflate; the determinism
contract with pond fixtures; allocator-counting tests; tracked
benchmarks on the silesia corpus.

### added

- zlib backend (POLLIWOG_WITH_ZLIB, default on): raii handles over
  z_stream, the _z entry points preferred (zlib 1.3.2), window bits
  per container. fetches zlib from the official mirror when no
  system package is found.
- one-shot squeeze(format, level, in, out) and swell(format, in,
  out): compress and decompress in a single call, pure functions,
  no hidden state. bound() via compressBound, honest for all three
  containers. level_unsupported rejected before touching the backend.
- round-trip property tests across formats and levels; determinism
  check; buffer_too_small, format_mismatch, level_unsupported, and
  backend_failure error paths all reachable by tests.
- streaming compressor and decompressor: push, flush (sync only,
  per D9), finish on the compressor; push on the decompressor.
  move-only, pimpl to keep internal headers out of the public
  surface. streaming round-trip and flush tests.
- allocator-counting tests enforcing the memory table: squeeze/swell
  over spans produce zero net heap allocations; compressor
  construction and destruction are balanced.
- google benchmark targets (POLLIWOG_BENCH=OFF by default):
  synthetic data at 1 KB, 64 KB, and 1 MB across zlib levels and
  formats; streaming round-trip; corpus benchmarks over the silesia
  pond when present.
- bench ci workflow: nightly and on release, three runners,
  benchmark results uploaded as artifacts with 90-day retention.

## 2026.1.0 - 2026-09-12

released. the gate was watched green on all six ci jobs (run
34662292915).

### added

- the repository bootstrap: contracts and workflow
  - contributing, security, license (MIT), changelog
  - .github: issue templates (bug, feature), pull request template
- the cmake skeleton: the polliwog::polliwog interface target at
  c++23, the option set from the readme, default and debug
  presets, and the clang-format/clang-tidy configs; configured and
  built warning-free locally on msvc 19.51 (cmake 4.4.2)
- ci: the build matrix (windows-latest, ubuntu-24.04, macos-15 x
  zlib-only/all-backends), the clang-format check, and catch2
  v3.16.0 with a smoke test that verifies the c++23 floor and
  reports the compiler
- the founding decisions, recorded in the decision log with their
  costs and change conditions
- the contracts in code, tested (2026.1.0)
  - include/polliwog/error.hpp: the kind enum, the error value, the
    exhaustive kind_name and backend_id_name
  - include/polliwog/format.hpp: the five wire formats and the typed
    zlib_level, its anchors pinned to their zlib.h values
  - include/detail/backend.hpp: the Backend concept, compile-checked
    in both directions with a synthetic fake
- the gate, watched green on all six ci jobs (run
  34662292915): cold clone, warnings gate on, tests passing on all
  three runners

### not yet (stated honestly)

- no compress or decompress exists yet. the zlib shim, one-shot
  squeeze/swell, and streaming land in 2026.2.0.
