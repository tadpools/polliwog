# polliwog near-term plan

working memory. this file holds the sitting-by-sitting plan for the
current runway and is replaced at each milestone sitting - roadmap.md
keeps the milestone-level truth, decisions.md keeps the decisions.
stale is worse than short: if this file has not been touched since the
last sitting, it gets rewritten or deleted at the start of the next.

current position: **hatch** (2026.2.0 unreleased). spawn shipped:
v2026.1.0 tagged and released 2026-09-12, spawn gate watched green on
all six ci jobs (run 34662292915). the runway below covers hatch
through the hatch gate.

## how the sittings work

one sitting = one step, ending with everything genuinely run: tests
watched passing, warnings checked, the changelog touched if a claim
changed, the decision log touched if a decision was made. the local
machine is windows (msvc 19.51, cmake 4.4.2, clang-format 18.1.8 via
the pip wheel); local builds are a convenience, the three-runner ci
is the gate. merges are done locally (rebase, then push) so the
committer stays the real identity; server-side rebase merges stamp
the account's primary email and are avoided.

## the hatch runway (five sittings)

### sitting h1 - the zlib shim

- detail/zlib.hpp: raii handles over z_stream; the _z entry-point
  preference (research-log row 1: compress_z, uncompress_z,
  compressBound_z, deflateBound_z) with a static_assert against
  ZLIB_VERSION per versions.md's upgrade policy
- the zlib backend type satisfying the Backend concept; found via
  find_package(ZLIB) with the fetchcontent fallback pinned to 1.3.2
  (versions.md); POLLIWOG_WITH_ZLIB=OFF must still configure
- one-shot squeeze/swell for the zlib container over spans, pure
  functions (D11), bound via the _z bound call
- round-trip tests first for the pure paths; allocator-counting
  tests begin against the memory table

done-when: squeeze then swell of synthetic pond data round-trips on
all three runners, watched; POLLIWOG_WITH_ZLIB=OFF builds an empty
backend cleanly.

### sitting h2 - gzip, raw_deflate, determinism

- gzip headers written with mtime=0 and os=unknown (D7); byte
  fixtures pin the header layout the specs underspecify
- raw_deflate as the third container; format dispatch by enum
- single-member gzip policy enforced by a test (D10): a second
  member after the first trailer is stream_corrupt with the
  position named

done-when: determinism fixtures green across the ci matrix; the
concat edge has a failing-input test, not a doc promise.

### sitting h3 - streaming

- compressor/decompressor: push/pull/finish over the zlib shim;
  sync_flush only (D9); the flush table in the header doc maps
  one-to-one to backend flush semantics
- buffer_too_small and stream_corrupt reachable by tests, position
  named by the error
- every error kind reachable by a test (the hatch gate demands it;
  the taxonomy exists since s4)

done-when: a streaming round-trip over chunked spans matches the
one-shot result byte for byte on all runners.

### sitting h4 - the pond corpus

- silesia files ingested with the official per-file md5s
  (research-log row 18); enwik8 downloaded with its checksum
  computed and recorded (row 19); the manifest carries both -
  checksums are computed, never invented
- round-trip property suite over the corpus: seeded, deterministic
  prng, swell(squeeze(x)) == x across formats and levels
- allocator-counting tests against the memory table completed

done-when: the property suite is green on the matrix; the corpus
manifest's checksums verify on a fresh clone.

### sitting h5 - benchmarks, then the gate

- google benchmark pinned to a verified release (versions.md:
  v1.9.5 current as of research-log row 11; re-verify live at this
  sitting before pinning), bench targets over the pond corpus
- the bench workflow: nightly + release, history kept, the 5%
  regression rule live in pr review
- hatch gate: the full property suite green on the matrix, every
  error kind reachable, benchmarks recorded, 2026.2.0 tagged with
  the changelog's not-yet section rewritten to match

## what would change this plan

- a zlib 1.3.3 or zstd 1.5.8 landing before hatch: re-verify at the
  sitting (research-log standing rule); a wire-visible change is a
  milestone decision, not a silent pin bump
- catch2 cloned per ci build: if the fetch flails on runners, vendor
  the pinned copy per versions.md's vendoring policy - with a
  decision record, not quietly
- the checkout action's node 20 deprecation annotation: bump the
  action tag at a sitting after verifying the current release, and
  update research-log with the fetch
- a real external caller showing up during hatch: their need goes
  through the adoption ladder like anyone's - early callers earn
  priority in ordering, never scope exceptions

## the parking lot (ideas, not in progress)

see roadmap.md. nothing here moves to a sitting without naming its
adoption-ladder step.
