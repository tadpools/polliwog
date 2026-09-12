## what changed

one paragraph, plain. which layer: convenience, one-shot, streaming,
backend shim, brood, docs.

## why

trace to a project contract or a belief in the readme, or link the
issue. non-obvious decisions: link the decision note.

## which test proves it

name the test file and case. failing-first for pure logic. new error
kind: the test that reaches it. determinism touch: the fixture.

## checklist

- [ ] tests written, failing first where pure logic
- [ ] warning-free on our code
- [ ] clang-format applied
- [ ] changelog entry under the next milestone
- [ ] docs updated if a claim changed (module map is the index of truth)
- [ ] decision record if the change was non-obvious
- [ ] benchmark comparison read; regression over 5% justified below

## benchmark delta

if code paths touched: before/after numbers from the bench workflow,
and the written reason for any regression over 5%. write "n/a" if
bench workflow did not run - it will, before merge.
