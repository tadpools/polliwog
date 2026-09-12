// gzip determinism fixtures and the single-member policy (D10).
// byte-level pins on the gzip header layout; raw_deflate no-container
// verification; concatenated gzip is stream_corrupt, not
// buffer_too_small.

#include "polliwog/squeeze.hpp"

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstddef>
#include <vector>

using byte = std::byte;

TEST_CASE("gzip pins mtime=0 and os=unknown") {
  // rfc 1952 section 2.3.1, byte layout of the gzip header
  std::array<byte, 100> src{};
  for (std::size_t i = 0; i < src.size(); ++i)
    src[i] = static_cast<byte>(i & 0xff);

  std::size_t cap = polliwog::bound(
      polliwog::format::gzip, polliwog::zlib_level::default_level, src.size());
  std::vector<byte> out(cap);
  auto r = polliwog::squeeze(polliwog::format::gzip,
                             polliwog::zlib_level::default_level,
                             std::span<const byte>{src}, std::span<byte>{out});
  REQUIRE(r.has_value());
  REQUIRE(r->bytes_written >= 10);

// cast to unsigned char before comparing: catch2 v3.16 declares
// StringMaker<std::byte> without defining it, same quirk as the
// string_view case. the cast is the signal that the comparison is
// byte-level, not a style choice.
#define REQUIRE_BYTE_AT(buf, idx, expected)                                    \
  REQUIRE(static_cast<unsigned char>((buf)[(idx)]) == (expected))

  REQUIRE_BYTE_AT(out, 0, 0x1f); // ID1
  REQUIRE_BYTE_AT(out, 1, 0x8b); // ID2
  REQUIRE_BYTE_AT(out, 2, 0x08); // CM = deflate
  REQUIRE_BYTE_AT(out, 3, 0x00); // FLG: no extra/name/comment
  REQUIRE_BYTE_AT(out, 4, 0x00); // MTIME byte 0
  REQUIRE_BYTE_AT(out, 5, 0x00); // MTIME byte 1
  REQUIRE_BYTE_AT(out, 6, 0x00); // MTIME byte 2
  REQUIRE_BYTE_AT(out, 7, 0x00); // MTIME byte 3
  // byte 8 is XFL, engine-specific, not pinned
  REQUIRE_BYTE_AT(out, 9, 0xff); // OS = 255 (unknown)
}

TEST_CASE("raw_deflate carries no container") {
  std::array<byte, 64> src{};
  for (std::size_t i = 0; i < src.size(); ++i)
    src[i] = static_cast<byte>(i);

  std::size_t cap =
      polliwog::bound(polliwog::format::raw_deflate,
                      polliwog::zlib_level::default_level, src.size());
  std::vector<byte> out(cap);
  auto r = polliwog::squeeze(polliwog::format::raw_deflate,
                             polliwog::zlib_level::default_level,
                             std::span<const byte>{src}, std::span<byte>{out});
  REQUIRE(r.has_value());
  REQUIRE(r->bytes_written > 0);

  // raw deflate has no header at all; the gzip magic must not appear
  bool has_gzip_magic =
      (r->bytes_written >= 2 && static_cast<unsigned char>(out[0]) == 0x1f &&
       static_cast<unsigned char>(out[1]) == 0x8b);
  REQUIRE_FALSE(has_gzip_magic);
}

TEST_CASE("single-member gzip only: concat is stream_corrupt") {
  std::array<byte, 100> src{};
  for (std::size_t i = 0; i < src.size(); ++i)
    src[i] = static_cast<byte>(i & 0xff);

  // squeeze two independent gzip streams
  std::size_t cap = polliwog::bound(
      polliwog::format::gzip, polliwog::zlib_level::default_level, src.size());
  std::vector<byte> a(cap), b(cap);
  auto ra = polliwog::squeeze(polliwog::format::gzip,
                              polliwog::zlib_level::default_level,
                              std::span<const byte>{src}, std::span<byte>{a});
  auto rb = polliwog::squeeze(polliwog::format::gzip,
                              polliwog::zlib_level::default_level,
                              std::span<const byte>{src}, std::span<byte>{b});
  REQUIRE(ra.has_value());
  REQUIRE(rb.has_value());

  // concatenate the two streams: valid gzip + valid gzip = two members
  std::vector<byte> concat;
  concat.insert(concat.end(), a.begin(),
                a.begin() + static_cast<std::ptrdiff_t>(ra->bytes_written));
  concat.insert(concat.end(), b.begin(),
                b.begin() + static_cast<std::ptrdiff_t>(rb->bytes_written));

  std::vector<byte> decompressed(src.size());
  auto r =
      polliwog::swell(polliwog::format::gzip, std::span<const byte>{concat},
                      std::span<byte>{decompressed});

  // D10: single-member policy; a second member after the first trailer
  // is stream_corrupt with the byte position named
  REQUIRE_FALSE(r.has_value());
  REQUIRE(r.error().k == polliwog::kind::stream_corrupt);
  REQUIRE(r.error().pos > 0);
  // the position names where extra data was detected: at or near
  // the first member boundary (zlib may peek past the trailer)
  REQUIRE(r.error().pos >= ra->bytes_written);
}
