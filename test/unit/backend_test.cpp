// the backend contract tests: the concept admits a conforming fake,
// rejects an empty type, and the fake's handle is move-only as the
// contract demands. the fake is synthetic by design (honesty law:
// labeled synthetic, deterministic, no captured data).
// Stability: internal (detail).

#include "detail/backend.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <iterator>
#include <type_traits>
#include <utility>

namespace {

// a synthetic engine that satisfies the contract without touching a
// real codec. the bound constant below is a made-up number for shape
// only - it is not a measured bound and pins nothing (the real
// engines arrive with the zlib shim)
struct fake_backend {
  struct handle {
    handle() = default;
    handle(handle &&) = default;
    handle &operator=(handle &&) = default;
    std::uint64_t in{0};
    std::uint64_t out{0};
  };

  // the fake has no backend identity; none is the honest spelling
  static constexpr polliwog::backend_id id{polliwog::backend_id::none};

  static std::span<const polliwog::format> formats() {
    static constexpr std::array fmts{polliwog::format::zlib};
    return fmts;
  }

  using level = polliwog::zlib_level;

  static handle make_squeezer(polliwog::format, level) { return handle{}; }
  static handle make_sweller(polliwog::format) { return handle{}; }

  static std::size_t bound(level, std::uint64_t in_size) {
    return in_size + 64;
  }

  static polliwog::detail::result push(handle &h, std::span<const std::byte> in,
                                       std::span<std::byte> out) {
    auto const n = std::min(in.size(), out.size());
    std::ranges::copy(in.first(n), out.begin());
    h.in += n;
    h.out += n;
    return polliwog::detail::progress{h.in, h.out, n == in.size()};
  }

  static polliwog::detail::result flush(handle &, std::span<std::byte>) {
    return polliwog::detail::progress{};
  }

  static polliwog::detail::result finish(handle &h, std::span<std::byte>) {
    return polliwog::detail::progress{h.in, h.out, true};
  }
};

// a type with nothing in it must not satisfy the concept: the check
// has to be able to say no
struct empty_type {};

} // namespace

static_assert(polliwog::detail::backend<fake_backend>);
static_assert(!polliwog::detail::backend<empty_type>);
static_assert(std::movable<fake_backend::handle>);
static_assert(!std::copyable<fake_backend::handle>);

TEST_CASE("the backend contract admits the fake and it behaves") {
  auto h = fake_backend::make_squeezer(polliwog::format::zlib,
                                       polliwog::zlib_level::best);
  REQUIRE(h.in == 0);

  std::array<std::byte, 4> in{std::byte{1}, std::byte{2}, std::byte{3},
                              std::byte{4}};
  std::array<std::byte, 4> out{};

  auto r = fake_backend::push(h, in, out);
  REQUIRE(r.has_value());
  REQUIRE(r->bytes_in == 4);
  REQUIRE(r->bytes_out == 4);
  REQUIRE(r->done);
  REQUIRE(std::ranges::equal(in, out));

  auto f = fake_backend::finish(h, out);
  REQUIRE(f.has_value());
  REQUIRE(f->done);

  auto g = std::move(h); // move-only by contract; this must compile
  REQUIRE(g.out == 4);
}
