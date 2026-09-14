// polliwog bench targets: squeeze and swell over the pond corpus,
// across formats and levels. results are printed to stdout in
// google benchmark's default format; ci keeps history for
// regression tracking (5% rule in pr review).
//
// the corpus directory is injected via compile definition from
// CMakeLists.txt. when the corpus is absent, the bench compiles
// but skips the file-based benchmarks (the synthetic benchmarks
// still run).
// Stability: Growing .

#include "polliwog/squeeze.hpp"
#include "polliwog/stream.hpp"

#include <benchmark/benchmark.h>

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <span>
#include <string>
#include <vector>

using byte = std::byte;
namespace fs = std::filesystem;

// -- corpus loading -------------------------------------------------------

namespace {

fs::path corpus_dir() {
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
  return fs::path{BENCH_POND_CORPUS_DIR};
}

std::vector<byte> load_file(fs::path const &p) {
  std::ifstream f(p, std::ios::binary | std::ios::ate);
  auto sz = f.tellg();
  f.seekg(0);
  std::vector<byte> buf(static_cast<std::size_t>(sz));
  f.read(reinterpret_cast<char *>(buf.data()), sz);
  return buf;
}

bool corpus_available() {
  auto dir = corpus_dir();
  return fs::is_directory(dir) && fs::exists(dir / "dickens");
}

// synthetic data: easily compressible (long runs of zeros)
std::vector<byte> synthetic(std::size_t n) {
  return std::vector<byte>(n, std::byte{0});
}

// synthetic data: random, incompressible
std::vector<byte> random_data(std::size_t n) {
  std::vector<byte> buf(n);
  // simple deterministic fill; not cryptographically random, but
  // incompressible enough for the ratio to approach 1.0
  for (std::size_t i = 0; i < n; ++i)
    buf[i] = static_cast<byte>((i * 2654435761u) & 0xff);
  return buf;
}

} // namespace

// -- synthetic benchmarks (always run) ------------------------------------

static void BM_squeeze_zlib_default(benchmark::State &state) {
  auto const data = synthetic(static_cast<std::size_t>(state.range(0)));
  std::size_t cap = polliwog::bound(polliwog::format::zlib,
                                    polliwog::zlib_level::default_level,
                                    data.size());
  std::vector<byte> out(cap);

  for (auto _ : state) {
    auto r = polliwog::squeeze(polliwog::format::zlib,
                               polliwog::zlib_level::default_level,
                               std::span<const byte>{data},
                               std::span<byte>{out});
    benchmark::DoNotOptimize(r);
  }
  state.SetBytesProcessed(state.iterations() * data.size());
}
BENCHMARK(BM_squeeze_zlib_default)
    ->Arg(1024)
    ->Arg(65536)
    ->Arg(1024 * 1024);

static void BM_swell_zlib_default(benchmark::State &state) {
  auto const data = synthetic(static_cast<std::size_t>(state.range(0)));
  std::size_t cap = polliwog::bound(polliwog::format::zlib,
                                    polliwog::zlib_level::default_level,
                                    data.size());
  std::vector<byte> compressed(cap);
  auto sr = polliwog::squeeze(polliwog::format::zlib,
                              polliwog::zlib_level::default_level,
                              std::span<const byte>{data},
                              std::span<byte>{compressed});
  benchmark::DoNotOptimize(sr);

  std::vector<byte> decompressed(data.size());
  for (auto _ : state) {
    auto r = polliwog::swell(
        polliwog::format::zlib,
        std::span<const byte>{compressed.data(), sr->bytes_written},
        std::span<byte>{decompressed});
    benchmark::DoNotOptimize(r);
  }
  state.SetBytesProcessed(state.iterations() * data.size());
}
BENCHMARK(BM_swell_zlib_default)
    ->Arg(1024)
    ->Arg(65536)
    ->Arg(1024 * 1024);

static void BM_squeeze_gzip_default(benchmark::State &state) {
  auto const data = synthetic(static_cast<std::size_t>(state.range(0)));
  std::size_t cap = polliwog::bound(polliwog::format::gzip,
                                    polliwog::zlib_level::default_level,
                                    data.size());
  std::vector<byte> out(cap);

  for (auto _ : state) {
    auto r = polliwog::squeeze(polliwog::format::gzip,
                               polliwog::zlib_level::default_level,
                               std::span<const byte>{data},
                               std::span<byte>{out});
    benchmark::DoNotOptimize(r);
  }
  state.SetBytesProcessed(state.iterations() * data.size());
}
BENCHMARK(BM_squeeze_gzip_default)
    ->Arg(1024)
    ->Arg(65536)
    ->Arg(1024 * 1024);

static void BM_squeeze_zlib_best(benchmark::State &state) {
  auto const data = synthetic(static_cast<std::size_t>(state.range(0)));
  std::size_t cap = polliwog::bound(polliwog::format::zlib,
                                    polliwog::zlib_level::best, data.size());
  std::vector<byte> out(cap);

  for (auto _ : state) {
    auto r = polliwog::squeeze(polliwog::format::zlib,
                               polliwog::zlib_level::best,
                               std::span<const byte>{data},
                               std::span<byte>{out});
    benchmark::DoNotOptimize(r);
  }
  state.SetBytesProcessed(state.iterations() * data.size());
}
BENCHMARK(BM_squeeze_zlib_best)
    ->Arg(1024)
    ->Arg(65536)
    ->Arg(1024 * 1024);

static void BM_squeeze_zlib_fastest(benchmark::State &state) {
  auto const data = synthetic(static_cast<std::size_t>(state.range(0)));
  std::size_t cap = polliwog::bound(polliwog::format::zlib,
                                    polliwog::zlib_level::fastest, data.size());
  std::vector<byte> out(cap);

  for (auto _ : state) {
    auto r = polliwog::squeeze(polliwog::format::zlib,
                               polliwog::zlib_level::fastest,
                               std::span<const byte>{data},
                               std::span<byte>{out});
    benchmark::DoNotOptimize(r);
  }
  state.SetBytesProcessed(state.iterations() * data.size());
}
BENCHMARK(BM_squeeze_zlib_fastest)
    ->Arg(1024)
    ->Arg(65536)
    ->Arg(1024 * 1024);

// incompressible data: the ratio should approach 1.0 (plus header overhead)
static void BM_squeeze_zlib_random(benchmark::State &state) {
  auto const data = random_data(static_cast<std::size_t>(state.range(0)));
  std::size_t cap = polliwog::bound(polliwog::format::zlib,
                                    polliwog::zlib_level::default_level,
                                    data.size());
  std::vector<byte> out(cap);

  for (auto _ : state) {
    auto r = polliwog::squeeze(polliwog::format::zlib,
                               polliwog::zlib_level::default_level,
                               std::span<const byte>{data},
                               std::span<byte>{out});
    benchmark::DoNotOptimize(r);
  }
  state.SetBytesProcessed(state.iterations() * data.size());
}
BENCHMARK(BM_squeeze_zlib_random)
    ->Arg(1024)
    ->Arg(65536)
    ->Arg(1024 * 1024);

// -- streaming benchmarks -------------------------------------------------

static void BM_streaming_zlib(benchmark::State &state) {
  auto const data = synthetic(static_cast<std::size_t>(state.range(0)));
  std::size_t cap = polliwog::bound(polliwog::format::zlib,
                                    polliwog::zlib_level::default_level,
                                    data.size());
  std::vector<byte> compressed(cap);
  std::vector<byte> decompressed(data.size());

  for (auto _ : state) {
    // compress
    polliwog::compressor c(polliwog::format::zlib,
                           polliwog::zlib_level::default_level);
    std::size_t c_off = 0;
    constexpr std::size_t chunk = 4096;
    for (std::size_t off = 0; off < data.size(); off += chunk) {
      auto n = std::min(chunk, data.size() - off);
      auto r = c.push(std::span<const byte>{data.data() + off, n},
                      std::span<byte>{compressed.data() + c_off,
                                      compressed.size() - c_off});
      benchmark::DoNotOptimize(r);
      c_off += r->bytes_produced;
    }
    auto fin = c.finish(std::span<byte>{compressed.data() + c_off,
                                        compressed.size() - c_off});
    benchmark::DoNotOptimize(fin);
    c_off += fin->bytes_produced;

    // decompress
    polliwog::decompressor d(polliwog::format::zlib);
    std::size_t d_off = 0;
    std::size_t co = 0;
    while (co < c_off) {
      auto n = std::min(chunk, c_off - co);
      auto r = d.push(
          std::span<const byte>{compressed.data() + co, n},
          std::span<byte>{decompressed.data() + d_off,
                          decompressed.size() - d_off});
      benchmark::DoNotOptimize(r);
      d_off += r->bytes_produced;
      co += r->bytes_consumed;
      if (r->done)
        break;
    }
  }
  state.SetBytesProcessed(state.iterations() * data.size());
}
BENCHMARK(BM_streaming_zlib)
    ->Arg(1024)
    ->Arg(65536)
    ->Arg(1024 * 1024);

// -- corpus benchmarks (run only when the pond corpus is present) ---------

static void BM_corpus_squeeze_zlib(benchmark::State &state) {
  if (!corpus_available()) {
    state.SkipWithError("pond corpus not available");
    return;
  }

  auto name = std::string{"dickens"};
  auto data = load_file(corpus_dir() / name);
  std::size_t cap = polliwog::bound(polliwog::format::zlib,
                                    polliwog::zlib_level::default_level,
                                    data.size());
  std::vector<byte> out(cap);

  for (auto _ : state) {
    auto r = polliwog::squeeze(polliwog::format::zlib,
                               polliwog::zlib_level::default_level,
                               std::span<const byte>{data},
                               std::span<byte>{out});
    benchmark::DoNotOptimize(r);
  }
  state.SetBytesProcessed(state.iterations() * data.size());
}
BENCHMARK(BM_corpus_squeeze_zlib);

static void BM_corpus_squeeze_gzip(benchmark::State &state) {
  if (!corpus_available()) {
    state.SkipWithError("pond corpus not available");
    return;
  }

  auto data = load_file(corpus_dir() / "dickens");
  std::size_t cap = polliwog::bound(polliwog::format::gzip,
                                    polliwog::zlib_level::default_level,
                                    data.size());
  std::vector<byte> out(cap);

  for (auto _ : state) {
    auto r = polliwog::squeeze(polliwog::format::gzip,
                               polliwog::zlib_level::default_level,
                               std::span<const byte>{data},
                               std::span<byte>{out});
    benchmark::DoNotOptimize(r);
  }
  state.SetBytesProcessed(state.iterations() * data.size());
}
BENCHMARK(BM_corpus_squeeze_gzip);

static void BM_corpus_roundtrip_zlib(benchmark::State &state) {
  if (!corpus_available()) {
    state.SkipWithError("pond corpus not available");
    return;
  }

  auto data = load_file(corpus_dir() / "dickens");
  std::size_t cap = polliwog::bound(polliwog::format::zlib,
                                    polliwog::zlib_level::default_level,
                                    data.size());
  std::vector<byte> compressed(cap);
  std::vector<byte> decompressed(data.size());

  for (auto _ : state) {
    auto sr = polliwog::squeeze(polliwog::format::zlib,
                                polliwog::zlib_level::default_level,
                                std::span<const byte>{data},
                                std::span<byte>{compressed});
    benchmark::DoNotOptimize(sr);
    auto dr = polliwog::swell(
        polliwog::format::zlib,
        std::span<const byte>{compressed.data(), sr->bytes_written},
        std::span<byte>{decompressed});
    benchmark::DoNotOptimize(dr);
  }
  state.SetBytesProcessed(state.iterations() * data.size());
}
BENCHMARK(BM_corpus_roundtrip_zlib);
