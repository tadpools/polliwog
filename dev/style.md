# the polliwog dialect

how a polliwog module is shaped, named, tested, and evolved, so
development is linear - header forty reads like header four. premise
inherited from tadpole: the formatter owns layout (clang-format); this
file owns everything else. when an old module disagrees with this
file, the old module gets one calm fix commit.

## the one-way rule

one blessed idiom per task. documented exceptions:

- the `_or_error` suffix never exists - the only api is
  expected-returning; there is no throwing twin to disambiguate.
- `with_` free functions on option types
  (with_level(opt, zlib_level::best), with_threads(opt, 4)). option
  types get make() plus with_ updates and nothing else.
- verb pairs: squeeze/swell for compress/decompress; push/pull/finish
  on streams; make_* factories. nothing else justifies a synonym.
- convenience overloads that allocate are suffixed `_string` or
  `_vector` so the cost is in the name: squeeze_string(...) returns
  std::string. the span versions never allocate.

if a review cannot name which rule a new function follows, it does not
go in.

## header anatomy

every public header follows stream.hpp's shape, in this order:

1. one-line purpose, ending with the stability tier line
   (`// Stability: Growing.`)
2. who reaches for this header directly (and who almost never does)
3. configuration or state explained field by field, if any
4. the load-bearing tables (flush modes, bounds, memory story)
5. failure modes, naming the kind and the likely cause
6. a worked example, marked `// Illustrative` unless copied from a
   real dev/ program or test
7. see also, at most three or four related headers

function docs: a one-line summary, then behavior and cost where real
(which calls allocate, the bound relationship, what a flush costs),
then an example if the shape is not obvious from the signature.

## naming

types and concepts:

- PascalCase; snake_case files matching the primary type
- formats are enum class format values named for the wire container
  (zlib, gzip, raw_deflate, zstd, lz4_frame), never for the backend
  brand alone (a gzip stream is gzip, produced by the zlib backend)
- levels are typed per backend (zlib_level, zstd_level) with named
  anchors: best, fastest, default
- opaque by default. a class's members go public only when tests or
  legitimate callers must construct it, and the doc says so
- the one taxonomy: polliwog::error, kind enum in error.hpp. no
  function invents its own error type

functions:

- squeeze / swell - the two verbs. never compress/decompress as api
  names (they live in comments where clarity demands)
- push / pull / finish - streaming phases; pull returns
  expected<span> over what was swelled into the caller's buffer
- make_* - factories (make_squeezer, make_sweller)
- is_* - Bool predicates (is_done, is_flushed)
- *_name - value to human text (kind_name, format_name)
- _ms suffix on every duration; digit separators in constants (10'000)

argument order:

- destination or options first, subject last, mirroring
  squeeze(format, level, input, output). labeled-parameter feel is
  achieved with small option structs, not parameter packs
- spans: input span before output span, always, in every signature

namespaces:

- polliwog - everything public
- polliwog::detail - shims and the backend contract; reachable, honest,
  moving faster, documented as such
- no macros that escape the namespace. POLLIWOG_* cmake and
  preprocessor options are the only macros with the prefix

## errors

- one taxonomy: polliwog::error + kind. see architecture.md
- new kinds land as additive milestones; kind_name covers every value,
  exhaustively switched, so the compiler flags a missed one
- internal_contract_violation is reserved for our bugs; its note asks
  for a report
- rendering lives in one place (kind_name + note); format-facing
  headers never assemble error text themselves

## tests

- three layers, mirroring src paths (test/pond/..., test/unit/...):
  - unit: pure, fast, every error kind reachable by an input
  - round-trip property: seeded, deterministic prng over the pond
    corpus; swell(squeeze(x)) == x across the matrix
  - contract: fixtures pinning bytes the specs underspecify (gzip
    header layout, determinism across the ci matrix)
- failing test first for pure logic; fixtures captured the same day a
  backend surprises us
- allocator-counting tests enforce the memory table in architecture.md
- warning-free is a hard gate, scoped to our code; backend warnings
  are upstream's problem
- the suite count is culture: every milestone grows it, never shrinks it

## deprecation and evolution

- [[deprecated("use squeeze_string instead")]] - the message always
  names the replacement
- deprecated things survive one milestone, die at the next; the
  changelog notes both the deprecation and the removal
- additive milestones: new kinds, new backends, new optional fields;
  breaking changes only at a milestone boundary, and only with the
  changelog saying so
- the module map in architecture.md is the index of truth: a header
  that is not in the table does not exist

## documentation shape

- the voice is lowercase, direct, plain. no emoji, no marketing voice,
  no enthusiasm a maintainer in a bad week cannot sustain
- performance notes only where real: which calls allocate, the cost of
  a flush, what bound() guarantees and what it does not
- examples in ```cpp fences; byte examples use real-shaped hex
  (1f 8b 08 00 ...), never "..."
- failure modes name the kind and the likely cause: stream_corrupt
  means bad bytes or truncation; buffer_too_small means the caller's
  span, check the bound
- every doc claim was checkable against this version's source; a doc
  that outruns the code is a bug

## what this file does not own

- layout: clang-format owns it, always wins
- smell: clang-tidy config lives in the repo root; disagreements land
  there, not here
- the why behind any of this: dev/philosophy.md

a conflict resolves upward: style yields to architecture, architecture
yields to the philosophy. when a rule stops serving linear development,
amend this file in a calm sitting with a decision record - never
bypass it quietly.
