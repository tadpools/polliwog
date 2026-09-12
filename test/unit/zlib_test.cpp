// round-trip tests for the zlib backend: squeeze then swell of the
// same data must be byte-identical, across formats and levels.
// Stability: Growing .

#include "polliwog/squeeze.hpp"

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstddef>
#include <cstring>
#include <span>
#include <vector>

using byte = std::byte;

namespace {

std::vector<byte> make_pattern(std::size_t n) {
  std::vector<byte> v(n);
  for (std::size_t i = 0; i < n; ++i)
    v[i] = static_cast<byte>(i & 0xff);
  return v;
}

} // namespace

TEST_CASE("zlib round-trip across formats and levels") {
  auto const src = make_pattern(4096);

  for (auto f : {polliwog::format::zlib, polliwog::format::gzip,
                 polliwog::format::raw_deflate}) {
    for (auto lvl :
         {polliwog::zlib_level::fastest, polliwog::zlib_level::default_level,
          polliwog::zlib_level::best}) {
      INFO("format=" << polliwog::format_name(f)
                     << " level=" << static_cast<int>(lvl));

      std::size_t cap = polliwog::bound(f, lvl, src.size());
      REQUIRE(cap > 0);
      REQUIRE(cap >= src.size());

      std::vector<byte> compressed(cap);
      auto sr = polliwog::squeeze(f, lvl, std::span<const byte>{src},
                                  std::span<byte>{compressed});
      REQUIRE(sr.has_value());
      REQUIRE(sr->bytes_written > 0);
      REQUIRE(sr->bytes_written <= cap);

      std::vector<byte> decompressed(src.size());
      auto dr = polliwog::swell(
          f, std::span<const byte>{compressed.data(), sr->bytes_written},
          std::span<byte>{decompressed});
      REQUIRE(dr.has_value());
      REQUIRE(dr->bytes_written == src.size());
      REQUIRE(std::memcmp(src.data(), decompressed.data(), src.size()) == 0);
    }
  }
}

TEST_CASE("zlib determinism: same input, same output") {
  auto const src = make_pattern(1024);
  std::size_t cap = polliwog::bound(
      polliwog::format::zlib, polliwog::zlib_level::default_level, src.size());

  std::vector<byte> a(cap), b(cap);

  auto ra = polliwog::squeeze(polliwog::format::zlib,
                              polliwog::zlib_level::default_level,
                              std::span<const byte>{src}, std::span<byte>{a});
  auto rb = polliwog::squeeze(polliwog::format::zlib,
                              polliwog::zlib_level::default_level,
                              std::span<const byte>{src}, std::span<byte>{b});

  REQUIRE(ra.has_value());
  REQUIRE(rb.has_value());
  REQUIRE(ra->bytes_written == rb->bytes_written);
  REQUIRE(std::memcmp(a.data(), b.data(),
                      static_cast<std::size_t>(ra->bytes_written)) == 0);
}

TEST_CASE("zlib buffer_too_small fires before writing") {
  auto const src = make_pattern(4096);
  std::size_t cap = polliwog::bound(
      polliwog::format::zlib, polliwog::zlib_level::default_level, src.size());

  std::vector<byte> tiny(cap - 1);
  auto r = polliwog::squeeze(polliwog::format::zlib,
                             polliwog::zlib_level::default_level,
                             std::span<const byte>{src}, std::span<byte>{tiny});
  REQUIRE_FALSE(r.has_value());
  REQUIRE(r.error().k == polliwog::kind::buffer_too_small);
}

TEST_CASE("unsupported format returns format_mismatch") {
  std::array<byte, 16> src{};
  std::array<byte, 128> out{};
  auto r = polliwog::squeeze(polliwog::format::zstd,
                             polliwog::zlib_level::default_level,
                             std::span<const byte>{src}, std::span<byte>{out});
  REQUIRE_FALSE(r.has_value());
  REQUIRE(r.error().k == polliwog::kind::format_mismatch);
}
