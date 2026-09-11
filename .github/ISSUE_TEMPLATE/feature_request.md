---
name: feature request
about: propose a feature; it must name its step on the adoption ladder
title: ''
labels: ['stage:idea']
assignees: ''
---

**the feature, one sentence**

what the library will do that it does not.

**the adoption ladder step**

a feature walks the stairs in dev/philosophy.md: spec/fixture/pure
function/backend/header/convenience. name the step this feature starts
at, or say "parking lot" honestly:

- [ ] a spec or written design note says it exists
- [ ] a fixture can capture it in the pond corpus
- [ ] a pure function can model it, testable
- [ ] a backend or stream type can carry it
- [ ] a header doc can type it, with a stability tier
- [ ] it is a "first hour" need justifying a convenience overload

**which root does it trace to**

root 1 (the bytes are the truth) or root 2 (the caller owns the
memory)? if neither, the answer is probably no, and that is fine.

**what it costs**

name the cost: a new kind in the taxonomy, a cmake option, a header in
the module map, a determinism edge, a dependency. features that name
their cost honestly get read seriously.

**alternatives considered**

including "do nothing with polliwog and use the backend directly."
