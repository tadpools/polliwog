# polliwog near-term plan

working memory. this file holds the sitting-by-sitting plan for the
current runway and is replaced at each milestone sitting - roadmap.md
keeps the milestone-level truth, decisions.md keeps the decisions.
stale is worse than short: if this file has not been touched since the
last sitting, it gets rewritten or deleted at the start of the next.

current position: **spawn** (2026.1.0 unreleased). no code exists.
the runway below covers spawn through the hatch gate.

## how the sittings work

one sitting = one step, ending with everything genuinely run: tests
watched passing, warnings checked, the changelog touched if a claim
changed, the decision log touched if a decision was made. the local
machine is windows; local builds are a convenience, the three-runner
ci is the gate. nothing here claims a compiler exists locally until
one has run.

## the spawn runway (four sittings)

### sitting s1 - repo truth

- git init, first commit (docs only), commit convention live
- the referenced-file audit: every file any doc mentions exists
  (LICENSE, CHANGELOG.md, SECURITY.md, dev/decisions.md were closed
  on 2026-09-11 - re-audit rather than trust this list)
- cross-reference sweep: grep every doc for relative links and file
  mentions; fix or delete the dead ones

done-when: no doc references a file that does not exist; the commit
history reads conventional.

### sitting s2 - cmake skeleton, warning-free on windows

- CMakeLists.txt: empty interface-turned-library target
  polliwog::polliwog; POLLIWOG_WITH_ZLIB/ZSTD/LZ4/ZLIB_NG/WARNINGS
  options defined exactly as dev/architecture.md documents
- CMakePresets.json: default + debug presets
- clang-format and clang-tidy configs in repo root
- local build attempt on the windows machine, recorded honestly: what
  toolchain version was found, what warnings fired

done-when: an empty library configures and builds warning-free
locally, with the toolchain version recorded in the sitting note.

sitting note (2026-09-11, closed): done. toolchain verified before
writing: cmake 4.4.2, ninja 1.13, visual studio community 2026
18.9.0, msvc 19.51.36256.0, windows sdk 10.0.26100.0; gcc 16.1.0
(mingw-w64 ucrt) present as backup. default and debug presets both
configured and built clean under msbuild 18.9.1 - zero warnings,
honestly trivial while the target compiles nothing; the gate is
wired as a helper with no caller until s4. clang-format and
clang-tidy are not installed on this machine - their check is a ci
concern at s3, recorded here rather than skipped silently. pr #9,
issue #8.

### sitting s3 - ci truth

- .github/workflows/build.yml: the 3-runner matrix
  (windows-latest, ubuntu-24.04, macos-15), POLLIWOG_WARNINGS=ON
- .github/workflows/format.yml: clang-format check
- catch2 via fetchcontent pinned to v3.16.0 (research-log row 10);
  one smoke test that cannot fail silently (asserts 1 == 1 is not a
  test - it asserts the runner ran and reports the compiler)

done-when: the matrix is green on a real run, watched, not assumed;
the smoke test output is visible in the run log.

sitting note (2026-09-12, closed): done, watched. the matrix ran on
main and all six jobs went green (run 34660291929): ubuntu 39s/52s,
macos 54s/1m12s, windows 1m40s/1m24s, zlib-only and all-backends.
the smoke test output is visible in the job logs (test #1: smoke,
passed; ci runners report msvc 19.51, matching the local machine).
the format check failed on its first run - four hand-formatting
violations in the smoke test - and passed after a chore(fmt) commit
(Pr #13); the gate earned its keep on day one. one real bug found by
watching locally: multi-config ctest presets need a pinned
configuration (fixed before push). honest notes: catch2 v3.16.0 is
fetched per build, so ci pays the clone every run; clang tools stay
local-only via the pip wheel (18.1.8, matching the ci pin); the
checkout action warns about node 20 deprecation - bump the tag at
the next milestone sitting, verified against current releases then.
pr #12, issue #11.

### sitting s4 - the contracts in code, spawn gate

- include/polliwog/error.hpp: the kind enum + error struct +
  exhaustive kind_name switch (compiler-enforced completeness)
- include/polliwog/format.hpp: format enum + typed level types
- include/detail/backend.hpp: the Backend concept with a fake backend
  that satisfies it, compile-checked
- the module map row added for each header that exists
- spawn gate: cold clone on all three runners, zero warnings, tests
  green - watched in the actions log

done-when: the spawn gate passes. 2026.1.0 is tagged. the changelog's
"not yet" section is rewritten to match reality.

## the hatch runway (outline - refined at the spawn-gate sitting)

- h1: the zlib shim - raii handles, the _z entry-point preference
  (research-log row 1), one-shot squeeze/swell for the zlib container;
  round-trip tests first for the pure paths
- h2: gzip (mtime=0, os=unknown) + raw_deflate; the determinism
  fixtures; single-member policy enforced by a test (decision D10)
- h3: streaming compressor/decompressor - push/finish, sync_flush
  only (decision D9); the flush table doc; buffer_too_small and
  stream_corrupt reachable by tests
- h4: the pond corpus - silesia files ingested with their official
  md5s (research-log row 18), enwik8 downloaded with its checksum
  computed and recorded (row 19); round-trip property suite over the
  corpus; allocator-counting tests against the memory table
- h5: google benchmark (pinned to a verified release) on the corpus,
  tracked in ci; the bench workflow; the 5% regression rule live
- hatch gate: the full property suite green, every error kind
  reachable, benchmarks recorded, 2026.2.0 tagged

## what would change this plan

- a zlib 1.3.3 or zstd 1.5.8 landing before hatch: re-verify at the
  sitting (research-log standing rule); a wire-visible change is a
  milestone decision, not a silent pin bump
- the clang/libc++ jthread question (open question, research-log)
  arriving early: if brood design work starts before froglet, the
  clang 20 floor decision gets made at the tadpole sitting, not the
  froglet one
- a real external caller showing up during spawn: their need goes
  through the adoption ladder like anyone's - early callers earn
  priority in ordering, never scope exceptions

## the parking lot (ideas, not in progress)

see roadmap.md. nothing here moves to a sitting without naming its
adoption-ladder step.
