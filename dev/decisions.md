# polliwog decision log

append-only. every non-obvious decision earns two or three sentences
the day it is made: what was chosen, what it cost, what would change
the answer. corrections are new entries that name what they replace -
never silent edits.

provenance note, per the honesty law: the founding decisions (D1-D10)
were made during the founding sittings of 2026-09-11 but are being
recorded here after the fact, in one calm sitting. that backfill is
stated openly rather than pretended into a fake chronology.

---

## D1 - the name is polliwog (founding, recorded 2026-09-11)

a second word for tadpole: two names for one creature, like two forms
of one data. chosen over froglet (implies unfinished), frogspawn
(reads as a build system), clam (closed, not alive), brood (kept as
the batch engine's name), tad (kept as an internal size-hint concept).
cost: none known. would change if: a real collision appears in a
package index search at the packaging sitting.

## D2 - one api over multiple backends, codec by enum (founding)

zlib, zstd, lz4 behind one squeeze/swell surface. this is the core
differentiator (dev/differentiation.md gap 1) and it is why the
Backend concept exists. cost: the shim layer adds indirection the
benchmarks will show publicly. would change if: the benchmarks prove
the indirection costs more than the flexibility is worth for one
backend - unlikely, and the Backend concept survives either way.

## D3 - c++23 baseline (founding)

std::expected with monadic operations (verified minimums: gcc 13,
clang 17, msvc 19.36, apple clang 15.0.0 - research-log row 6) are
load-bearing. cost: clang libc++ lists jthread as partial until 20,
which constrains the brood milestone. would change if: a supported
platform needs the library and cannot ship c++23 - the answer would
be "that platform waits", unless a real caller changes the calculus.

## D4 - errors are values; nothing throws (founding)

one error taxonomy (kind + backend + position + note) through
std::expected. ported from tadpole belief 1. cost: every convenience
overload must thread errors back by value; no try/catch shortcuts.
would change if: never, short of a premise rewrite - this is a root,
not a feature.

## D5 - caller owns the memory; spans in, spans out (founding)

inputs are std::span<const std::byte>, outputs std::span<std::byte>;
the allocating overloads are separate and named (_string, _vector).
the memory table is enforced by allocator-counting tests. cost: the
beginner path is slightly longer than a wrapped string api. would
change if: the convenience overloads prove to be the used surface and
the span path rots - then the docs, not the design, get fixed first.

## D6 - backends are compile-time cmake options, not runtime plugins
(founding)

POLLIWOG_WITH_ZLIB/ZSTD/LZ4/ZLIB_NG; the Backend concept is checked at
compile time. cost: a backend switch means a rebuild. would change if:
a real caller needs runtime codec selection that enum-per-call cannot
express - that is a design discussion, not a plugin system.

## D7 - determinism contract (founding)

same input, same format, same level, same backend version =
byte-identical output everywhere; gzip headers carry mtime=0 and
os=unknown. cost: forbids convenient-but-nondeterministic shortcuts
(timestamped gzip headers, random dictionary ids). would change if: a
caller needs real timestamps - that becomes an explicit option, never
a default.

## D8 - calver YYYY.N.P with growth-stage milestone names (founding)

spawn, hatch, tadpole, froglet, frog; stability tiers on every public
header; ecosystem pins by milestone, never by patch. inherited whole
from the tadpole governance. cost: none beyond diligence. would change
if: never - the versioning IS the honesty system.

## D9 - hatch ships sync_flush only; flush modes beyond it wait
(2026-09-11)

zlib's full flush-mode ladder (partial, block, full) is spec surface
we can test honestly later; shipping all of them half-tested would be
a lie of coverage. cost: an interop edge (resuming a stream mid-flush)
waits for tadpole. would change if: a real streaming consumer needs
partial flush before tadpole - it comes in with its own fixtures, not
alone.

## D10 - hatch reads single-member gzip only (2026-09-11)

rfc 1952 allows concatenated members; zstr auto-handles them. polliwog
hatch does not: a second member after the first trailer is an honest
error (stream position named), with concat support as a parked,
ladder-named feature. cost: a real interop gap with .gz files in the
wild that we state plainly instead of half-supporting. would change
if: the pond corpus files are multi-member - then concat joins hatch
with fixtures.

## D11 - one-shot squeeze/swell are pure functions (2026-09-11)

no globals, no statics, no hidden shared state: concurrent one-shot
calls on distinct buffers are safe by contract; handles stay
single-threaded. cost: none - it forbids laziness we should not have.
would change if: never without a premise rewrite.

## D12 - the near-term plan lives in dev/near-term.md as working
memory (2026-09-11)

sitting-sized steps, replaced at each milestone sitting; roadmap.md
keeps the milestone-level truth. cost: one more file to keep honest.
would change if: the file goes stale once - then it gets deleted
rather than tolerated stale.

## D13 - the label taxonomy is materialized as family:value
(2026-09-11)

dev/github.md's table lists label families and bare values; the repo
labels render each cell as family:value (stage:idea, area:core,
tier:stable, priority:high, community:good-first-issue). chosen so
the four families stay distinguishable in the label list and the
issue templates' defaults (area:core, stage:idea) resolve verbatim.
cost: label text differs from the bare table values, so the table is
read as family + value, not as literal label names. would change if:
the templates or the table ever name labels another way - then both
sides move in one commit.

## D14 - zlib's default level is pinned to 6, not inherited as -1
(2026-09-12)

zlib.h documents Z_DEFAULT_COMPRESSION as -1, "currently equivalent
to level 6"; polliwog's zlib_level::default_level pins 6 explicitly.
chosen so the determinism contract (same input, same level,
byte-identical output) does not depend on how the backend resolves
its own default on a given day or version bump. cost: if zlib ever
changed the -1 mapping, we would not follow it silently - the pin
would be re-decided here, not discovered in a diff. would change if:
a caller need for real -1 semantics shows up; it becomes an explicit
option, never the default (the same rule as D7's timestamps).
