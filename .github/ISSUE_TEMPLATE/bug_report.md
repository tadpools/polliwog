---
name: bug report
about: a round-trip breaks, an error kind fires wrongly, or a contract fails
title: ''
labels: ['area:core']
assignees: ''
---

**what happened**

one paragraph. what call, what input shape, what came out instead.

**the code**

```cpp
// minimal, complete, runnable. real spans, real formats. no pseudocode.
```

**the error value**

if the api returned an error, paste it verbatim: kind, backend, pos,
note. do not paraphrase - the note is evidence.

**environment**

- polliwog milestone/tag:
- backend versions (zlib/zstd/lz4, from your build log):
- compiler and standard library:
- os:
- cmake options that were on:

**which contract failed**

- [ ] round-trip (swell(squeeze(x)) != x)
- [ ] determinism (same input, different bytes)
- [ ] memory (an allocation the table does not name)
- [ ] errors (wrong kind, missing position, swallowed failure)
- [ ] crash / hang

**can you reproduce it**

yes/no, and how. a failing test written first closes issues fastest.
