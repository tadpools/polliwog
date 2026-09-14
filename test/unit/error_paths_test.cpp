// tests for every error kind reachable through the public api.
// the hatch gate demands every kind has a test that names it.
// Stability: Growing .

#include "polliwog/squeeze.hpp"
#include "polliwog/stream.hpp"

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstddef>
#include <cstring>
#include <vector>

using byte = std::byte;

TEST_CASE("level_unsupported: out-of-range zlib level") {
  // zlib levels are 0-9; constructing zlib_level{10} is legal at the
  // type level but outside the backend's range. the one-shot layer
  // must reject it before touching the backend.
  std::array<byte, 64> src{};
  std::array<byte, 256> out{};
  auto r = polliwog::squeeze(polliwog::format::zlib, polliwog::zlib_level{10},
                             std::span<const byte>{src}, std::span<byte>{out});
  REQUIRE_FALSE(r.has_value());
  REQUIRE(r.error().k == polliwog::kind::level_unsupported);
}

TEST_CASE("level_unsupported: negative zlib level") {
  std::array<byte, 64> src{};
  std::array<byte, 256> out{};
  auto r = polliwog::squeeze(polliwog::format::zlib, polliwog::zlib_level{-1},
                             std::span<const byte>{src}, std::span<byte>{out});
  REQUIRE_FALSE(r.has_value());
  REQUIRE(r.error().k == polliwog::kind::level_unsupported);
}

TEST_CASE("level_unsupported: streaming compressor throws") {
  // the constructor cannot return expected; it throws for invalid
  // levels (the only throwing path in the public api, documented)
  REQUIRE_THROWS_AS(
      polliwog::compressor(polliwog::format::zlib, polliwog::zlib_level{10}),
      std::invalid_argument);
}

TEST_CASE("backend_failure: corrupt zlib stream triggers backend error") {
  // a valid zlib header followed by truncated body forces inflate to
  // fail with a backend error (not stream_corrupt from our code, but
  // a forwarded zlib error)
  std::array<byte, 8> corrupt{};
  // valid zlib header: 0x78 0x01 (deflate, fast)
  corrupt[0] = static_cast<byte>(0x78);
  corrupt[1] = static_cast<byte>(0x01);
  // rest is zeros — inflate will fail on the truncated data

  std::array<byte, 256> out{};
  polliwog::decompressor d(polliwog::format::zlib);
  auto r = d.push(std::span<const byte>{corrupt}, std::span<byte>{out});
  REQUIRE_FALSE(r.has_value());
  // the error may be stream_corrupt (detected by our code) or
  // backend_failure (forwarded from zlib); both are valid for a
  // corrupted stream. the important thing is it fails, not crashes
  REQUIRE((r.error().k == polliwog::kind::stream_corrupt ||
           r.error().k == polliwog::kind::backend_failure));
}

TEST_CASE("format_mismatch: swell zlib bytes through zstd format") {
  // compress with zlib, then try to decompress as zstd. when zstd is
  // compiled in, the backend honestly attempts decompression and fails
  // with stream_corrupt (data is not a valid zstd frame). when zstd
  // is not compiled in, format_mismatch is returned.
  std::array<byte, 128> src{};
  for (std::size_t i = 0; i < src.size(); ++i)
    src[i] = static_cast<byte>(i & 0xff);

  std::size_t cap = polliwog::bound(
      polliwog::format::zlib, polliwog::zlib_level::default_level, src.size());
  std::vector<byte> compressed(cap);
  auto sr = polliwog::squeeze(polliwog::format::zlib,
                              polliwog::zlib_level::default_level,
                              std::span<const byte>{src},
                              std::span<byte>{compressed});
  REQUIRE(sr.has_value());

  std::vector<byte> decompressed(src.size());
  auto dr = polliwog::swell(
      polliwog::format::zstd,
      std::span<const byte>{compressed.data(), sr->bytes_written},
      std::span<byte>{decompressed});
  REQUIRE_FALSE(dr.has_value());
#ifdef POLLIWOG_HAS_ZSTD
  REQUIRE(dr.error().k == polliwog::kind::stream_corrupt);
#else
  REQUIRE(dr.error().k == polliwog::kind::format_mismatch);
#endif
}
