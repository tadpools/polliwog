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
- the founding decisions, recorded in dev/decisions.md with their
  costs and change conditions

### not yet (stated honestly)

- no library code exists. no backend shim, no public header, no ci
  workflow. the spawn gate is infrastructure only: a cold clone
  configures and builds an empty library, warning-free, on all three
  ci runners.

## 2026.2.0 (hatch) - planned

scope boundary in dev/roadmap.md. zlib only, honestly labeled:
one-shot and streaming squeeze/swell over zlib, gzip, and raw_deflate;
the error taxonomy; the determinism contract with pond fixtures;
allocator-counting tests; tracked benchmarks on the silesia corpus.
