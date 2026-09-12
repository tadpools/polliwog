# contributing to polliwog

the rules are few and absolute. this file is the operating
procedure; the header docs carry the contracts.

## before you write code

1. find the feature's step on the adoption ladder: does a spec or a
   design note say it exists, can a fixture capture it, can a pure
   function model it, can a backend carry it, can a header type it?
   if it cannot name a step, open an issue instead - ideas go to the
   parking lot, calmly.
2. a header that is not in the module map does not exist; adding a
   row is a milestone event, never a drive-by.
3. one blessed idiom per task. if a review cannot name which rule a
   new function follows, it does not go in.

## the pull request checklist

- [ ] tests written, failing first where pure logic
- [ ] round-trip property updated if a new format/level path exists
- [ ] warning-free on our code (`POLLIWOG_WARNINGS=ON`)
- [ ] clang-format applied (a layout-only `chore(fmt):` commit is fine)
- [ ] changelog entry under the next milestone
- [ ] docs updated if a claim changed
- [ ] a decision note if the change was non-obvious (what was
      chosen, what it cost, what would change the answer)
- [ ] benchmark comparison read; a regression over 5% needs a written
      reason

## commits

conventional commits. lowercase, imperative, under 72 characters:

```
feat(zstd): wire stable-api streaming shim behind POLLIWOG_WITH_ZSTD

the shim wraps only stable symbols; dictionary entry points stay out
until a design note passes the adoption ladder.

refs #42
```

types: feat, fix, docs, test, refactor, perf, build, ci, chore.
scopes: core, zlib, zstd, lz4, brood, files, error, bench, docs, ci.
a commit with two types is two commits.

## issues

one concern per issue. use the templates. a feature issue names its
adoption-ladder step or is moved to the parking lot.

## the gates

- cold clone builds warning-free on all three ci runners
- every error kind reachable by a test
- determinism fixtures green across the matrix
- no exceptions in the public api; `std::expected` only
- no allocations the memory table does not name

these are not aspirations; they are what ci enforces. failure is a
calm fix, not a debate.

## good first issues

labeled `community:good-first-issue` in the tracker. typical shape: a
missing error-kind test, a fixture for an underspecified header, a
doc claim that outran the code.
