# security policy

a compression library is an attack surface: it parses untrusted bytes
by design. corrupt input is expected behavior, not a bug - the error
taxonomy exists for it. the bugs that matter here are the ones corrupt
input uses to escape the contract: memory unsafety, unbounded
allocation, infinite loops.

## supported versions

| milestone | status      | security fixes |
|-----------|-------------|----------------|
| spawn     | bootstrap   | n/a - nothing shipped yet |
| hatch+    | supported   | patch releases (YYYY.N.P) for security fixes, named in the changelog |

## reporting

use github's private vulnerability reporting (security tab). do not
open a public issue for anything that could be exploited.

include: the polliwog milestone/tag, the backend versions from the
build log, the compiler, and a minimal input that triggers the
behavior. a failing input is evidence; a description alone is not.

## how findings are triaged

- fuzz findings are security findings by default and get the security
  triage lane.
- reports are acknowledged within 48 hours during milestone sittings,
  and honestly when slower.
- a fix lands as a patch release when the wire contract allows it, or
  as a milestone when it cannot - the changelog says which and why.
- memory-safety findings in a backend (not in polliwog) are reported
  upstream first, with the polliwog changelog noting the pinned
  version's exposure honestly.

## the standing rule

the determinism and memory contracts in the header docs are security
documentation, not marketing: the allocator-counting and fuzz tests
enforce them. a report that a contract row is false is a security
report.
