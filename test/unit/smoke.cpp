// the s3 smoke test (dev/near-term.md): proves the runner runs, the
// c++23 floor holds, and the toolchain reports itself.
// Stability: Stable.

#include <catch2/catch_test_macros.hpp>

#include <expected>
#include <string>

namespace {

std::string compiler_id()
{
#if defined(_MSC_VER)
  return "msvc " + std::to_string(_MSC_VER);
#elif defined(__clang__)
  return "clang " + std::to_string(__clang_major__);
#elif defined(__GNUC__)
  return "gcc " + std::to_string(__GNUC__);
#else
  return "";
#endif
}

} // namespace

TEST_CASE("the runner runs, the floor holds, the compiler reports")
{
  auto const compiler = compiler_id();
  INFO("compiler: " << (compiler.empty() ? std::string("unrecognized") : compiler));

  // monadic expected is the load-bearing c++23 feature
  // (dev/versions.md); a toolchain that cannot do this sits below
  // the floor and must fail here, loudly
  auto const r = std::expected<int, int>{1}.and_then(
    [](int v) { return std::expected<int, int>{v + 1}; });
  REQUIRE(r.value() == 2);

  // an unrecognized toolchain must fail loudly, not pass quietly
  REQUIRE_FALSE(compiler.empty());
}
