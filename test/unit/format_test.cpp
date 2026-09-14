// the format and level tests: the wire-format names resolve, and the
// zlib level pins match the upstream documentation they cite.
// Stability: Stable .

#include "polliwog/format.hpp"

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <string>

TEST_CASE("format_name covers every wire format, distinctly") {
  constexpr std::array formats{
      polliwog::format::zlib,        polliwog::format::gzip,
      polliwog::format::raw_deflate, polliwog::format::zstd,
      polliwog::format::lz4_frame,
  };

  // std::string copies, not string_view captures: catch2 v3.16
  // declares StringMaker<string_view> without defining it
  std::array<std::string, formats.size()> names{};
  for (std::size_t i = 0; i < formats.size(); ++i) {
    names[i] = std::string{polliwog::format_name(formats[i])};
    INFO(names[i]);
    REQUIRE_FALSE(names[i].empty());
  }

  for (std::size_t i = 0; i < names.size(); ++i) {
    for (std::size_t j = i + 1; j < names.size(); ++j) {
      REQUIRE(names[i] != names[j]);
    }
  }
}

TEST_CASE("the zlib level anchors hold their documented values") {
  // provenance: zlib 1.3.2's zlib.h - Z_BEST_SPEED
  // is 1, Z_BEST_COMPRESSION is 9, and Z_DEFAULT_COMPRESSION is -1,
  // documented as currently equivalent to level 6. polliwog pins 6
  // so the determinism contract does not ride on the backend's
  // default resolution.
  STATIC_REQUIRE(int(polliwog::zlib_level::fastest) == 1);
  STATIC_REQUIRE(int(polliwog::zlib_level::default_level) == 6);
  STATIC_REQUIRE(int(polliwog::zlib_level::best) == 9);
}

TEST_CASE("the zstd level anchors hold their documented values") {
  // provenance: zstd 1.5.7's zstd.h - ZSTD_minCLevel() returns 1,
  // ZSTD_CLEVEL_DEFAULT is 3, ZSTD_maxCLevel() returns 19 (or 22
  // with advanced api, but we pin 19 as the safe stable ceiling).
  STATIC_REQUIRE(int(polliwog::zstd_level::fastest) == 1);
  STATIC_REQUIRE(int(polliwog::zstd_level::default_level) == 3);
  STATIC_REQUIRE(int(polliwog::zstd_level::best) == 19);
}
