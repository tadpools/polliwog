# polliwog core concepts and beliefs

this is the constitution. it says why. architecture.md says what;
style.md says how it reads; github.md says how it ships. amend this
file in calm sittings, like the code. written 2026-09-11.

## premise

polliwog is compression truth for c++. not an archive tool, not a
filesystem, not magic. the library's job: squeeze bytes into the
smallest honest form, swell them back bit-perfect, and hand everything
else to the developer. every belief below is derived from that
sentence.

## the two roots

everything should be derivable from two ideas. if a proposal cannot be
traced to one of them, it probably does not belong.

### root 1: the bytes are the truth

compression is defined by round-trip: swell(squeeze(x)) == x, every
time, on every platform, forever. the only authorities are the format
specifications (rfc 1950/1951/1952, the zstd and lz4 frame specs) and
the bytes themselves. we read the specs row by row, we capture edge
cases as fixtures, and we never guess.

- in practice: every format decision is a pure, tested function mirrored
  against the spec (headers, trailers, bound calculations, flush
  semantics). the pond test corpus pins behavior the specs underspecify.
  when a backend release surprises us, a fixture and a test land the
  same day.
- we do not: invent behavior the specs do not state, wrap backend
  quirks in a friendly lie, or ship a determinism claim the test suite
  cannot back.

### root 2: the caller owns the memory

the library allocates nothing the caller did not ask for, hides
nothing, and takes nothing away. buffers in and out are caller-provided
spans; the convenience overloads that allocate are separate functions,
documented as allocating. the backend handles live behind raii types
whose destructor is the only cleanup, and whose internals stay public
for the developer who needs them.

- in practice: one-shot apis take an output span and report the bytes
  written; no output-size guessing, no hidden scratch buffers, no
  exceptions across the boundary. every allocation is named in the doc
  of the function that makes it.
- we do not: take a choice away from the developer without saying so
  out loud, cache results the caller did not request, or make core
  depend on an opinion.

## the beliefs

### 1. errors are values; degradation is data

nothing in the public api throws. every fallible call returns
std::expected carrying structure (backend, code, byte position, human
note), so callers switch on kinds instead of parsing strings. a
truncated stream degrades to an error that names how much was swelled
before the corruption, not a crash.

in practice: error is one taxonomy (kind + backend + position + note),
like tadpole's TadpoleError. the one kind that means "this is our bug"
is internal_contract_violation, and its note asks for a report. we do
not: swallow errors, throw across the api, or let a corrupt stream
kill a process.

### 2. the beginner api is not a cage

the path of least resistance (squeeze(format, level, in, out), four
lines to a compressed file) and the path of full control (raw backend
handles, flush modes, dictionary state) use the same objects, the same
types, the same behavior. a beginner's one-shot call and a streaming
pipeline are built from the same parts at different heights.

in practice: the one-shot call is a loop over the streaming type; the
loop is public and readable. every internal layer stays public and
documented. we do not: gate capability behind the abstraction, or make
the simple path a special case that breaks under load.

### 3. fail loud, stay boring

a corrupt input fails on the call that reads it, with the position
named. a wrong output span fails before any bytes are written. the
library does not retry, does not guess, does not paper over. boring
internals, delightful api.

in practice: bounds are checked before work begins. flush modes map
one-to-one to backend flush semantics, documented in a table, no
invented modes. we do not: buffer "just in case", add locking "just in
case", or mistake clever for efficient.

### 4. the platform is an asset; use it on purpose

modern c++ is chosen, not tolerated. the language and the standard
library supply the hard parts, and the library leans on them visibly:

- std::span and std::byte for memory; no raw char* in the public api.
- std::expected for errors; no exceptions, no out-params for status.
- raii for backend handles; unique_ptr with custom deleters wrapping
  the c apis; the c api never leaks past detail/.
- concepts for the backend contract, so a new backend is checked at
  compile time, not discovered at runtime.
- std::jthread and standard primitives for brood; no hand-rolled
  thread pool with secret locks.
- the type system does the guarding: formats are enums, levels are
  typed per backend, a zlib level cannot reach a zstd stream.

we do not: fight the runtime through cisms where a c++ way exists, bury
state in statics, or mistake templates for architecture. where the
platform is weak (no stable abi, modules still uneven), the docs say so
and the decision is deliberate.

### 5. the dependency tree is part of the surface

backends ship into every user's binary, so the default tree stays
small: zlib is the only required backend; zstd and lz4 are cmake
options, compiled out when absent. no third-party c++ dependencies in
core, ever. test and benchmark dependencies (catch2, google benchmark)
are dev-only and never leak into the installed target.

in practice: POLLIWOG_WITH_ZSTD=OFF must build and pass the suite. a
new core dependency is a milestone event with a written reason, chosen
so it could be vendor-replaced if it died.

### 6. privacy is default

compression libraries see user data; polliwog logs nothing, ever. no
telemetry, no phone-home, no hidden files. the brood engine's worker
threads are std::jthread with names, and that is the whole story.

### 7. honesty over polish

stability tiers on every public header (Stable, Growing,
Experimental), calver so a milestone is a statement of what the
library is today, the changelog and test suite as the contract, and a
readme that lists what does not exist yet. nothing is described in
public before it has a test.

in practice: "internals" means usable-but-moving, stated plainly.
pre-frog, the changelog is candid about gaps. we do not: publish a
milestone whose live gate has not passed, describe vaporware, or let a
stability claim outlive its evidence.

calver semantics, spelled out: a milestone (YYYY.N.0) may break and
usually says something about the api or a backend jump; a patch
(YYYY.N.P) is fixes and additions that cannot break. the pinned
backend versions live in exactly one place (versions.md); the day a
backend forces a jump, that is a milestone, not a patch. ecosystem
packages pin core by milestone, never by patch - the tadpole rule,
inherited whole.

### 8. the docs teach compression

polliwog's docs are part of the product. a reader who has never
touched deflate should finish the compressor header understanding
bound calculation, flush semantics, and why a corrupt trailer fails
late, not just the function signatures. the format tables live in our
docs because that is where a developer looks when something breaks at
2am.

in practice: every format-facing header carries the wire rules it
implements; the guide explains why gzip output is deterministic here
(mtime=0, no os byte) and what that costs. we do not: hide format
knowledge behind "see rfc 1951", ship a doc table no test pins, or let
docs lag a backend bump.

## how we treat features

### the adoption ladder

a feature walks the same stairs every time, one step per sitting:

1. the spec (or a written design note) says it exists
2. a fixture captures it in the pond corpus
3. a pure function models the decision or shape, with tests
4. a backend or stream type carries it
5. the header doc types it, with a stability tier
6. a convenience overload wraps it only if it is a "first hour" need

a feature that cannot name its step is not in progress; it is an idea,
and ideas live in the parking lot (roadmap.md).

### decision records

every non-obvious decision earns two or three sentences in a decision
record the day it is made: what was chosen, what it cost, and what
would change the answer. decisions that live only in code get
relitigated every time someone reads the code; decisions that live in
the log compound.

### what core will never hold

- an archive format. zip and tar are frogspawn's job, someday, maybe;
  core knows streams, not containers.
- an io layer. files and sockets are the caller's; the file helpers
  in files.hpp are thin conveniences over caller-provided buffers.
- encryption. a compressed stream is not a secure stream, and the docs
  say so out loud.
- a plugin system. backends are compile-time cmake options, not
  runtime registrations.

## constitution vs procedure

- this file: why. stable. amend rarely, in calm sittings, with a
  decision record.
- dev/architecture.md: the shape. layers, types, contracts.
- dev/style.md: the c++ dialect. how modules read, name, test, evolve.
- dev/github.md: the shipping rhythm. branches, commits, issues,
  releases.
- dev/versions.md: the pinned backends and toolchain floor.

a conflict resolves upward: style yields to architecture, architecture
yields to beliefs, beliefs yield to the premise. if the premise stops
being true, that is a rewrite, not a patch.
