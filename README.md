# polliwog

a c++23 compression library: one squeeze/swell interface over zlib,
zstd, and lz4. errors are values. the caller owns the memory.

polliwog is a second word for tadpole - two names for one small
creature, like two forms of the same bytes. compress shrinks a frog
back into a polliwog; decompress lets it grow. it is the compression
package of the wetland ecosystem, sibling to [tadpole](https://github.com/tadpools/tadpole).

## the shape of it

```cpp
// Illustrative
#include <polliwog/polliwog.hpp>

std::vector<std::byte> frog = load_the_frog();
std::vector<std::byte> wog(frog.size());          // caller owns the memory

auto r = polliwog::squeeze(polliwog::format::gzip,
                           polliwog::zlib_level::best,
                           frog, std::span<std::byte>{wog});
if (!r) {
  // r.error(): kind, backend, byte position, note. never an exception.
  report(polliwog::kind_name(r.error().k), r.error().pos);
}

auto back = polliwog::swell(polliwog::format::gzip,
                            std::span<const std::byte>{wog}.first(r->bytes_written),
                            frog);                // round-trip: back == frog
```

one call, codec by enum, no hidden allocations, no exceptions. swap
`format::gzip` for `format::zstd` and nothing else changes - that is
the point.

## what exists (and honestly, what does not)

| component                       | tier          | milestone |
|---------------------------------|---------------|-----------|
| error + kind taxonomy           | Stable (hatch)| in at spawn, tested |
| format + typed zlib levels      | Stable (hatch)| in at spawn, tested |
| backend contract (detail)       | internal      | in at spawn, tested |
| one-shot squeeze/swell (zlib)   | Growing       | in at hatch, tested |
| streaming push/pull/finish      | not yet       | hatch     |
| zstd backend                    | not yet       | tadpole   |
| brood: parallel batch engine    | not yet       | froglet   |
| mmap-backed file helpers        | not yet       | froglet   |
| lz4 backend                     | not yet       | froglet   |
| vcpkg / conan packaging         | not yet       | frog      |

this readme lists what does not exist yet on purpose. nothing is
described here before it has a test.

## versioning

calver: `YYYY.N.P`. a milestone may break; a patch may not. milestones
are growth stages: spawn, hatch, tadpole, froglet, frog. ecosystem
packages pin by milestone, never by patch.

## the contracts

- **round-trip**: swell(squeeze(x)) == x, always, on every platform.
- **determinism**: same input, same format, same level, same backend
  version - byte-identical output, everywhere. gzip writes mtime=0.
- **memory**: no hidden allocations; every allocating overload says so
  in its name and doc.
- **errors**: values, never exceptions; one taxonomy with kind,
  backend, position, note.
- **output is the backend's wire format, byte for byte** - a zlib
  stream here is a zlib stream anywhere. no proprietary container,
  ever.

## backends

zlib 1.3.2 (default) - zlib-ng 2.3.3 (optional) - zstd 1.5.7 -
lz4 1.10.0. versions are pinned by the maintainers; licenses
permissive, verified before packaging.

## building

```sh
cmake --preset default && cmake --build --preset default && ctest --preset default
```

options: `POLLIWOG_WITH_ZLIB` (default on), `POLLIWOG_WITH_ZSTD`,
`POLLIWOG_WITH_LZ4`, `POLLIWOG_WITH_ZLIB_NG`, `POLLIWOG_WARNINGS`.

## contributing

see [CONTRIBUTING.md](CONTRIBUTING.md) for the shipping rhythm:
conventional commits, small pull requests, decision records for
non-obvious calls.

## license

MIT. backend licenses (zlib, BSD-3, BSD-2) are permissive and
redistribution-friendly; each is verified against the backend's own
license file before packaging.

## status

spawn, released as 2026.1.0. the contracts are in and tested; public
code lands at hatch. the versioning section above is the roadmap.
