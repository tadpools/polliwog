# changelog

format follows keep a changelog 1.1.0; versions are calver (YYYY.N.P).
a milestone (YYYY.N.0)
may break; a patch (YYYY.N.P) may not. milestone names are growth
stages: spawn, hatch, tadpole, froglet, frog.

every entry is checkable against the tag it ships in. nothing is
described here before it has a test.

## unreleased - 2026.2.0 (hatch)

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
  containers.
- round-trip property tests across formats and levels; determinism
  check; buffer_too_small and format_mismatch error paths.
- streaming compressor and decompressor: push, flush (sync only,
  per D9), finish on the compressor; push on the decompressor.
  move-only, pimpl to keep internal headers out of the public
  surface. streaming round-trip and flush tests.

## 2026.1.0 (spawn) - 2026-09-12

released. the spawn gate was watched green on all six ci jobs (run
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
- the contracts in code, tested (spawn sitting s4)
  - include/polliwog/error.hpp: the kind enum, the error value, the
    exhaustive kind_name and backend_id_name
  - include/polliwog/format.hpp: the five wire formats and the typed
    zlib_level, its anchors pinned to their zlib.h values (D14)
  - include/detail/backend.hpp: the Backend concept, compile-checked
    in both directions with a synthetic fake
- the spawn gate, watched green on all six ci jobs (run
  34662292915): cold clone, warnings gate on, tests passing on all
  three runners

### not yet (stated honestly)

- no compress or decompress exists yet. the zlib shim, one-shot
  squeeze/swell, and streaming land at hatch.
