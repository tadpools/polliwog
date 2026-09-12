# polliwog

a c++23 compression library: one squeeze/swell interface over zlib,
zstd, and lz4. errors are values. the caller owns the memory.

## the shape of it

```cpp
#include <polliwog/squeeze.hpp>

std::vector<std::byte> data = load_data();
std::vector<std::byte> out(data.size());

auto r = polliwog::squeeze(polliwog::format::gzip,
                           polliwog::zlib_level::best,
                           data, std::span<std::byte>{out});
if (!r) {
  report(polliwog::kind_name(r.error().k), r.error().pos);
}

auto back = polliwog::swell(polliwog::format::gzip,
                            std::span<const std::byte>{out}.first(r->bytes_written),
                            data);
```

one call, codec by enum, no hidden allocations, no exceptions. swap
`format::gzip` for `format::zstd` and nothing else changes - that is
the point.

## what exists (and honestly, what does not)

| component                       | tier          | status    |
|---------------------------------|---------------|-----------|
| error + kind taxonomy           | Stable        | in 2026.1.0, tested |
| format + typed zlib levels      | Stable        | in 2026.1.0, tested |
| backend contract (detail)       | internal      | in 2026.1.0, tested |
| one-shot squeeze/swell (zlib)   | Growing       | in 2026.2.0, tested |
| streaming push/pull/finish      | Growing       | in 2026.2.0, tested |
| zstd backend                    | not yet       | 2026.3.0  |
| brood: parallel batch engine    | not yet       | 2026.4.0  |
| mmap-backed file helpers        | not yet       | 2026.4.0  |
| lz4 backend                     | not yet       | 2026.4.0  |
| vcpkg / conan packaging         | not yet       | 2027.1.0  |

this readme lists what does not exist yet on purpose. nothing is
described here before it has a test.

## versioning

calver: `YYYY.N.P`. a milestone may break; a patch may not. versions
are pinned in releases.

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
lz4 1.10.0. pinned versions; each backend keeps its own license.

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

2026.1.0 released, 2026.2.0 in progress. the contracts are in and
tested; one-shot and streaming compress/decompress work over zlib.
