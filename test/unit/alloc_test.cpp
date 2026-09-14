// allocator-counting tests enforcing the memory table in
// architecture.md. squeeze/swell over spans must not leak heap
// allocations: zlib's internal alloc/free are paired inside each
// call, so the net heap delta is zero. the compressor constructor
// allocates the pimpl; the destructor frees it; the net after
// destruction is also zero.
//
// uses the MSVC CRT debug heap (_CrtMemCheckpoint) on Windows.
// the test compiles to no-ops on other platforms until a portable
// counting strategy is added.
// Stability: Growing .

#include "polliwog/squeeze.hpp"
#include "polliwog/stream.hpp"

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

#ifdef _MSC_VER
// _CrtMemCheckpoint / _CrtMemDifference are available in all MSVC
// build modes (debug and release) when the CRT is linked normally.
// they track every heap allocation made through the CRT allocator,
// which covers operator new, malloc, and zlib's default zalloc.
#include <crtdbg.h>
#endif

using byte = std::byte;

namespace {

#ifdef _MSC_VER

struct heap_snapshot {
  _CrtMemState state;
};

heap_snapshot take_snapshot() {
  heap_snapshot s;
  _CrtMemCheckpoint(&s.state);
  return s;
}

// net heap delta (allocations minus deallocations) since the snapshot
std::int64_t heap_delta(heap_snapshot const &before) {
  _CrtMemState after;
  _CrtMemCheckpoint(&after);
  _CrtMemState diff;
  if (_CrtMemDifference(&diff, &before.state, &after) == 0)
    return 0;
  return static_cast<std::int64_t>(diff.lTotalCount);
}

#else

struct heap_snapshot {};
heap_snapshot take_snapshot() { return {}; }
std::int64_t heap_delta(heap_snapshot const &) { return 0; }

#endif

std::vector<byte> pattern(std::size_t n) {
  std::vector<byte> v(n);
  for (std::size_t i = 0; i < n; ++i)
    v[i] = static_cast<byte>(i & 0xff);
  return v;
}

} // namespace

TEST_CASE("squeeze over spans: zero net heap allocations") {
  // memory table (architecture.md): squeeze(span,span) never
  // allocates beyond the backend handle (stack-local). zlib's
  // internal state is allocated by deflateInit2 and freed by
  // deflateEnd within the same call; the net is zero.
  auto const src = pattern(4096);
  std::size_t cap = polliwog::bound(
      polliwog::format::zlib, polliwog::zlib_level::default_level, src.size());
  std::vector<byte> compressed(cap);

  // snapshot after test setup: vectors are on the heap but stable
  auto cp = take_snapshot();
  auto r = polliwog::squeeze(
      polliwog::format::zlib, polliwog::zlib_level::default_level,
      std::span<const byte>{src}, std::span<byte>{compressed});
  auto const delta = heap_delta(cp);

  REQUIRE(r.has_value());
  REQUIRE(delta == 0);
}

TEST_CASE("swell over spans: zero net heap allocations") {
  // same contract as squeeze: no net heap allocations through the call.
  auto const src = pattern(4096);
  std::size_t cap = polliwog::bound(
      polliwog::format::zlib, polliwog::zlib_level::default_level, src.size());
  std::vector<byte> compressed(cap);
  auto sr = polliwog::squeeze(
      polliwog::format::zlib, polliwog::zlib_level::default_level,
      std::span<const byte>{src}, std::span<byte>{compressed});
  REQUIRE(sr.has_value());

  std::vector<byte> decompressed(src.size());
  auto cp = take_snapshot();
  auto dr = polliwog::swell(
      polliwog::format::zlib,
      std::span<const byte>{compressed.data(), sr->bytes_written},
      std::span<byte>{decompressed});
  auto const delta = heap_delta(cp);

  REQUIRE(dr.has_value());
  REQUIRE(delta == 0);
}

TEST_CASE(
    "compressor lifetime: zero net heap after construction and destruction") {
  // the compressor constructor allocates the pimpl (unique_ptr<impl>)
  // and the z_stream inside it. the destructor frees both. the net
  // heap delta from before construction to after destruction must be
  // zero: no leaked state.
  auto cp = take_snapshot();
  {
    polliwog::compressor c(polliwog::format::zlib,
                           polliwog::zlib_level::default_level);
  }
  auto const delta = heap_delta(cp);

  REQUIRE(delta == 0);
}

TEST_CASE("squeeze + swell round-trip: zero net heap allocations") {
  // the combined path: neither call leaks; the net across both is zero.
  auto const src = pattern(8192);
  std::size_t cap = polliwog::bound(
      polliwog::format::zlib, polliwog::zlib_level::default_level, src.size());
  std::vector<byte> compressed(cap);
  std::vector<byte> decompressed(src.size());

  auto cp = take_snapshot();
  auto sr = polliwog::squeeze(
      polliwog::format::zlib, polliwog::zlib_level::default_level,
      std::span<const byte>{src}, std::span<byte>{compressed});
  auto dr = polliwog::swell(
      polliwog::format::zlib,
      std::span<const byte>{compressed.data(), sr->bytes_written},
      std::span<byte>{decompressed});
  auto const delta = heap_delta(cp);

  REQUIRE(sr.has_value());
  REQUIRE(dr.has_value());
  REQUIRE(dr->bytes_written == src.size());
  REQUIRE(std::memcmp(src.data(), decompressed.data(), src.size()) == 0);
  REQUIRE(delta == 0);
}
