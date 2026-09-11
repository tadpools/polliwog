# why polliwog exists

head-to-head with the tools a c++ developer would reach for today.
the honest claim: nothing here is a better deflate. the backends are
somebody else's and we say so. what is missing from the ecosystem is
the layer above them, and that layer is the whole project.

## the gaps, stated plainly

1. **every wrapper is monogamous.** zstr is zlib. miniz is its own
   deflate. libdeflate is its own deflate. boost filters wrap one
   codec per filter chain. picking a codec today means picking a
   library, an api idiom, and an error style - and switching means
   rewriting call sites. polliwog: one api, codec chosen by enum,
   swappable per call site with zero rewrite.

2. **errors are afterthoughts.** the c apis return ints. most wrappers
   throw strings or wrap the ints. nobody hands back "where in the
   stream it broke". polliwog: std::expected with kind, backend,
   position, note - the tadpole belief, ported to c++.

3. **memory stories are secret.** wrappers either allocate behind
   your back or make you guess output sizes against compressBound
   yourself. polliwog: caller-owned spans in and out, bound() honest
   or max() documented, and a memory table in the docs that tests
   enforce.

4. **parallel batch is out of scope everywhere.** pigz proved the
   demand for parallel gzip; every library leaves it to the caller.
   polliwog: brood, a batch engine with deterministic merge, built on
   std::jthread, holding no compression state of its own.

5. **nothing is honest about its own gaps.** readmes oversell;
   stability is a vibe. polliwog: stability tiers on every header,
   calver milestones, a readme that lists what does not exist yet.

## the comparison table

|                       | polliwog        | zstr        | bxzstr                  | miniz       | libdeflate    | boost iostreams |
|-----------------------|-----------------|-------------|-------------------------|-------------|---------------|-----------------|
| codecs behind one api | zlib+zstd+lz4   | zlib only   | zlib+bz2+lzma+zstd      | own deflate | own deflate   | one per filter  |
| error style           | expected+kind   | throws      | failbit exceptions      | int codes   | int codes     | throws          |
| failure position      | named (pos)     | no          | no                      | no          | no            | no              |
| caller-owned buffers  | spans, enforced | partial     | stream buffers          | manual      | manual        | stream buffers  |
| hidden allocations    | none, tested    | some        | some                    | some        | none          | many (judgment) |
| streaming             | push/pull/finish| istream     | istream                 | some        | yes           | yes             |
| parallel batch        | brood           | no          | no                      | no          | no            | no              |
| determinism contract  | tested, pinned  | unspecified | unspecified             | unspecified | gzip optional | unspecified     |
| archive formats       | never (on purpose) | no       | no                      | zip         | no            | no              |
| language level        | c++23           | c++11       | c++11                   | c           | c             | c++03 (self-declared) |
| license               | MIT             | MIT         | MPL-2.0                 | MIT         | Apache-2.0    | BSL-1.0         |

sources, with provenance (rows 7-9 and 13-15 in dev/research-log.md):

- zstr readme fetched live 2026-09-11: confirms exceptions on zlib
  errors, 1 MB internal buffers, zlib-only, header-only, auto-detect
  of gzip/zlib headers, mit, miniz-compatible; the readme itself
  points to bxzstr for bz2/lzma support.
- bxzstr readme fetched live 2026-09-11: confirms the multi-codec
  iostreams api, failbit error surface, c++11, mpl-2.0, fork of
  zstr, magic-number detection, googletest testing described as
  non-exhaustive.
- miniz fetched live 2026-09-11: current release 3.1.2 (2026-07-01),
  zip-focused, recent releases adding fuzz targets for its zip and
  compression paths, cmake 4 support.
- libdeflate fetched live 2026-09-11: current release v1.26
  (2026-08-22).
- boost.iostreams fetched live 2026-09-11: the readme self-declares
  the framework/filter model, a C++03 property, "requires a link
  library", and BSL-1.0 - the framework and language-level rows are
  the project's own words. the "throws" and "many" allocation cells
  are design judgments from the filter architecture, labeled
  "(judgment)" in the table rather than passed off as measured fact.

where a project grows a feature after these dates, the changelog
wins; this file is amended at milestone sittings with new
research-log rows, not by drive-by edits.

license-row provenance, stated honestly: zlib (zlib license), zstr
(MIT), bxzstr (MPL-2.0), lz4 (BSD-2), and boost (BSL-1.0) were
visible on the fetched pages themselves. the miniz (MIT), libdeflate
(Apache-2.0), and zstd (BSD-3) license cells come from training
knowledge and get verified against their LICENSE files at the first
packaging sitting; they are not yet cited in any argument.

## the position

- **against bxzstr** (the closest competitor, verified live): bxzstr
  already unifies zlib, bz2, lzma, and zstd behind one api, and the
  honesty law says we say so. what polliwog adds where bxzstr stops:
  caller-owned spans instead of iostreams buffering, errors as
  values with byte position instead of failbit, the brood batch
  engine, a tested determinism contract, typed per-backend levels
  instead of 1-9 ints, and cmake options instead of a config header.
  bxzstr remains a fine c++11 iostreams choice for codebases that
  already speak streams.
- **against zstr**: polliwog is what zstr would become if it stopped
  wrapping one backend and started owning the layer. zstr remains a
  fine iostreams filter for zlib-only codebases. bxzstr already
  exists as its multi-codec fork; see the row above.
- **against miniz**: miniz is an archive engine wearing a compression
  hat. polliwog refuses archive formats on purpose (frogspawn's job,
  someday, maybe); the two compose, they do not compete.
- **against libdeflate**: the fastest inflate in the field. if the
  benchmarks say a libdeflate backend shim beats zlib-shim on the pond
  corpus, a libdeflate backend is welcome - the Backend concept exists
  exactly so the fastest engine can win without the api changing.
  parking-lot candidate, honestly noted.
- **against boost**: boost iostreams is a framework; polliwog is a
  library. no dependency on boost, no filter-chain indirection, no
  exceptions. the three lines to adapt polliwog output into an
  ostream live in the caller's code, on purpose.

## what we do not claim

- we are not faster than the backends. the shims add nanoseconds of
  indirection and the benchmarks will show it, in public, tracked in
  ci.
- we are not smaller than libdeflate. core adds the layer; the docs
  name the binary cost and the option to compile backends out.
- we are not a format. polliwog output is the backend's wire format,
  byte for byte - a zlib stream here is a zlib stream anywhere. the
  value is in the api, the errors, and the guarantees, never in a
  proprietary container.
