# the shipping rhythm

branches, commits, issues, pull requests, releases, ci. the rules are
few and absolute, like tadpole's. professional does not mean heavy;
it means predictable.

## branches

trunk-based. `main` is protected: no direct pushes, ci green required,
one review minimum.

| branch pattern   | use                                  | life        |
|------------------|--------------------------------------|-------------|
| feat/<verb-phrase> | a feature off the adoption ladder  | days        |
| fix/<symptom>    | a bug with a failing test first      | hours-days  |
| docs/<topic>     | docs-only, may merge without review  | hours       |
| bench/<name>     | benchmark comparisons, never merged with code | days |
| release/YYYY.N.P | the milestone branch, cut from main  | hours       |

long-lived branches are a smell; if one is needed, the milestone is
too big and gets cut in half, calmly.

## commits

conventional commits, 1.0.0. the subject line is lowercase, imperative,
under 72 characters, and says what the change does to the code, not
what you did to it.

```
<type>(<scope>): <subject>

[body: why, not what. two or three sentences when non-obvious.]

[footer: refs #123, BREAKING CHANGE: ...]
```

types: feat, fix, docs, test, refactor, perf, build, ci, chore.
scopes: core, zlib, zstd, lz4, brood, files, error, bench, docs, ci.
a commit with two types is two commits. the formatter never runs in a
commit that changes semantics; layout-only commits say `chore(fmt):`.

rebase-merge is the default; merge commits are for milestone branches
only. history reads like it was written by one calm person.

## decision records

every non-obvious decision earns two or three sentences, the day it is
made, in the decision log (`dev/decisions.md`, append-only): what was
chosen, what it cost, what would change the answer. the commit that
implements the decision references the log line. decisions that live
only in code get relitigated; decisions in the log compound.

## issues

one concern per issue. the templates do the formatting; the writer
does the thinking. label taxonomy:

| label family | values                                          |
|--------------|-------------------------------------------------|
| stage        | idea, ladder-named, in-progress, in-review       |
| area         | core, zlib, zstd, lz4, brood, files, docs, ci    |
| tier         | stable, growing, experimental                    |
| priority     | high, normal, low (three, not seven)             |
| community    | good-first-issue, help-wanted                    |

a feature issue must name its step on the adoption ladder or be
closed to the parking lot. milestone issues are tracking issues; they
link, they do not duplicate.

## pull requests

small or cut in half. the template asks for: what changed, why, which
test proves it, which doc row changed (the module map is the index of
truth). checklist:

- [ ] tests written, failing first where pure logic
- [ ] warning-free on our code
- [ ] clang-format applied (layout-only commit is fine)
- [ ] changelog entry under the next milestone
- [ ] docs updated if a claim changed
- [ ] decision record if the change was non-obvious

ci runs on every pr: build matrix, unit + round-trip suites, fuzz
smoke, format check, benchmark comparison reported (not gated). a
benchmark regression over 5% needs a written reason in the pr, or it
does not merge.

## releases

tags are `vYYYY.N.P`, cut from a release branch. the release notes are
the changelog section, nothing more, nothing marketing. every release
states: backend versions pinned (versions.md), the determinism
contract status, and the stability tier of anything new. patches
(YYYY.N.P) never break; if they would, they are milestones and say so.

## ci shape (github actions)

| workflow          | trigger        | job                                          |
|-------------------|----------------|----------------------------------------------|
| build             | pr, push main  | matrix: 3 runners x {zlib-only, all-backends}|
| test              | build          | catch2 unit + round-trip + contract          |
| fuzz              | pr (smoke), weekly (deep) | libfuzzer corpus + afl++ smoke    |
| format            | pr             | clang-format check, clang-tidy report        |
| bench             | nightly, release | google benchmark on silesia; history kept   |
| sanitizers        | nightly        | asan, ubsan, tsan on the round-trip suite    |

warning-free is a hard gate on our code (POLLIWOG_WARNINGS=ON);
backend-internal warnings are upstream's problem and silenced locally
with a comment naming the issue.

## security

security.md at the root; private vulnerability reporting via github's
security advisories. a compression library is an attack surface:
fuzz findings are security findings by default and get the security
triage lane. reports are acknowledged in 48 hours during milestone
sittings, and honestly when slower.

## the local-only rule, inherited

dev/philosophy.md and dev/style.md are the constitution and the
dialect. they may ship publicly or stay local, per the tadpole
convention - but whatever ships must match what the code does. a doc
that outruns the code is a bug with a deadline.
