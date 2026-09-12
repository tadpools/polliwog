// round-trip property tests over the pond corpus. each file is
// squeezed then swelled across formats and levels; the result must
// be byte-identical. the prng seed is fixed so the test is
// deterministic across runs and platforms.

#include "polliwog/squeeze.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>
#include <vector>

using byte = std::byte;
namespace fs = std::filesystem;

namespace {
fs::path find_corpus_dir() {
#ifdef _MSC_VER
  char *val = nullptr;
  std::size_t len = 0;
  _dupenv_s(&val, &len, "POLLIWOG_CORPUS_DIR");
  if (val) {
    fs::path p{val};
    free(val);
    return p;
  }
#else
  if (auto *env = std::getenv("POLLIWOG_CORPUS_DIR"))
    return fs::path{env};
#endif
  return fs::path{TEST_POND_CORPUS_DIR};
}

bool corpus_available() {
  auto dir = find_corpus_dir();
  return fs::is_directory(dir) && fs::exists(dir / "dickens");
}

std::vector<byte> load_file(fs::path const &p) {
  std::ifstream f(p, std::ios::binary | std::ios::ate);
  auto sz = f.tellg();
  f.seekg(0);
  std::vector<byte> buf(static_cast<std::size_t>(sz));
  f.read(reinterpret_cast<char *>(buf.data()), sz);
  return buf;
}

std::vector<byte> random_chunk(std::vector<byte> const &src,
                               std::size_t chunk_size, std::mt19937 &rng) {
  std::uniform_int_distribution<std::size_t> dist(0, src.size() - chunk_size);
  auto off = dist(rng);
  return {src.begin() + static_cast<std::ptrdiff_t>(off),
          src.begin() + static_cast<std::ptrdiff_t>(off + chunk_size)};
}

} // namespace

TEST_CASE("pond corpus round-trip", "[.pond]") {
  // the [.pond] tag means this test only runs when explicitly
  // requested (catch2 tag convention); the ci workflow runs all
  // tests including tagged ones
  if (!corpus_available()) {
    WARN("corpus not found, skipping");
    return;
  }

  auto dir = find_corpus_dir();
  std::mt19937 rng(42);

  std::array files{"dickens", "mozilla", "mr",  "nci",     "ooffice", "osdb",
                   "reymont", "samba",   "sao", "webster", "xml",     "x-ray"};

  for (auto name : files) {
    auto data = load_file(dir / name);
    REQUIRE(data.size() > 0);

    // test up to 4 random chunks per file, 64kb each
    constexpr std::size_t chunk = 65536;
    std::size_t n_chunks =
        std::min(static_cast<std::size_t>(4), data.size() / chunk);

    for (std::size_t i = 0; i < n_chunks; ++i) {
      auto sample = random_chunk(data, chunk, rng);

      for (auto f : {polliwog::format::zlib, polliwog::format::gzip,
                     polliwog::format::raw_deflate}) {
        for (auto lvl : {polliwog::zlib_level::fastest,
                         polliwog::zlib_level::default_level,
                         polliwog::zlib_level::best}) {
          INFO("file=" << name << " format=" << polliwog::format_name(f)
                       << " level=" << static_cast<int>(lvl));

          std::size_t cap = polliwog::bound(f, lvl, sample.size());
          REQUIRE(cap > 0);

          std::vector<byte> compressed(cap);
          auto sr = polliwog::squeeze(f, lvl, std::span<const byte>{sample},
                                      std::span<byte>{compressed});
          REQUIRE(sr.has_value());

          std::vector<byte> decompressed(sample.size());
          auto dr = polliwog::swell(
              f,
              std::span<const byte>{compressed.data(), static_cast<std::size_t>(
                                                           sr->bytes_written)},
              std::span<byte>{decompressed});
          REQUIRE(dr.has_value());
          REQUIRE(dr->bytes_written == sample.size());
          REQUIRE(std::memcmp(sample.data(), decompressed.data(),
                              sample.size()) == 0);
        }
      }
    }
  }
}
