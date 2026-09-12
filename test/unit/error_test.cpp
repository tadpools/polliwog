// the error taxonomy tests: every name resolves, every name is
// distinct, the value is honest about its defaults.
// Stability: Stable (hatch).

#include "polliwog/error.hpp"

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <string>

TEST_CASE("kind_name covers every kind, distinctly") {
  constexpr std::array kinds{
      polliwog::kind::stream_corrupt,
      polliwog::kind::buffer_too_small,
      polliwog::kind::level_unsupported,
      polliwog::kind::format_mismatch,
      polliwog::kind::backend_failure,
      polliwog::kind::out_of_memory,
      polliwog::kind::internal_contract_violation,
  };

  // std::string copies, not string_view captures: catch2 v3.16
  // declares StringMaker<string_view> without defining it, so the
  // names are materialized before they reach a REQUIRE
  std::array<std::string, kinds.size()> names{};
  for (std::size_t i = 0; i < kinds.size(); ++i) {
    names[i] = std::string{polliwog::kind_name(kinds[i])};
    INFO(names[i]);
    REQUIRE_FALSE(names[i].empty());
  }

  for (std::size_t i = 0; i < names.size(); ++i) {
    for (std::size_t j = i + 1; j < names.size(); ++j) {
      REQUIRE(names[i] != names[j]);
    }
  }
}

TEST_CASE("backend_id_name covers every id, distinctly") {
  constexpr std::array ids{
      polliwog::backend_id::none,    polliwog::backend_id::zlib,
      polliwog::backend_id::zlib_ng, polliwog::backend_id::zstd,
      polliwog::backend_id::lz4,
  };

  std::array<std::string, ids.size()> names{};
  for (std::size_t i = 0; i < ids.size(); ++i) {
    names[i] = std::string{polliwog::backend_id_name(ids[i])};
    INFO(names[i]);
    REQUIRE_FALSE(names[i].empty());
  }

  for (std::size_t i = 0; i < names.size(); ++i) {
    for (std::size_t j = i + 1; j < names.size(); ++j) {
      REQUIRE(names[i] != names[j]);
    }
  }
}

TEST_CASE("a default error is safe and meaningless") {
  // the caller checks the expected's bool first; a default error
  // never travels on its own. pos 0 and backend none are the
  // documented meaningless states (architecture.md)
  polliwog::error const e{};
  REQUIRE(e.b == polliwog::backend_id::none);
  REQUIRE(e.pos == 0);
  REQUIRE(e.note.empty());
}
