# changelog

format follows keep a changelog 1.1.0; versions are calver (YYYY.N.P),
per dev/versions.md and the tadpole governance. a milestone (YYYY.N.0)
may break; a patch (YYYY.N.P) may not. milestone names are growth
stages: spawn, hatch, tadpole, froglet, frog.

every entry is checkable against the tag it ships in. nothing is
described here before it has a test.

## unreleased - 2026.1.0 (spawn)

### added

- the repository bootstrap: docs, contracts, and workflow
  - dev/: philosophy, architecture, style, versions, differentiation,
    roadmap, github workflow, research log, near-term plan
  - AGENTS.md: the operating procedure and the honesty law
  - contributing, security, license (MIT), changelog
  - .github: issue templates (bug, feature), pull request template
- the cmake skeleton: the polliwog::polliwog interface target at
  c++23, the option set from dev/architecture.md, default and debug
  presets, and the clang-format/clang-tidy configs; configured and
  built warning-free locally on msvc 19.51 (cmake 4.4.2)
- ci: the build matrix (windows-latest, ubuntu-24.04, macos-15 x
  zlib-only/all-backends), the clang-format check, and catch2
  v3.16.0 with a smoke test that verifies the c++23 floor and
  reports the compiler
- the founding decisions, recorded in dev/decisions.md with their
  costs and change conditions

### not yet (stated honestly)

- no compress or decompress exists yet. the contracts are in and
  tested: the error taxonomy (error.hpp), the wire-format enum with
  typed zlib levels (format.hpp), and the Backend concept
  (detail/backend.hpp) with a compile-checked fake backend. the zlib
  shim, one-shot squeeze/swell, and streaming land at hatch.

## 2026.2.0 (hatch) - planned

scope boundary in dev/roadmap.md. zlib only, honestly labeled:
one-shot and streaming squeeze/swell over zlib, gzip, and raw_deflate;
the error taxonomy; the determinism contract with pond fixtures;
allocator-counting tests; tracked benchmarks on the silesia corpus.
