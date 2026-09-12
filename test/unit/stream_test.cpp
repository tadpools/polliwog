// streaming round-trip, flush, and error paths.

#include "polliwog/stream.hpp"

#include "polliwog/squeeze.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <numeric>
#include <vector>

using byte = std::byte;

namespace {

std::vector<byte> make_seq(std::size_t n) {
  std::vector<byte> v(n);
  for (std::size_t i = 0; i < n; ++i)
    v[i] = static_cast<byte>(i & 0xff);
  return v;
}

} // namespace

TEST_CASE("streaming round-trip in chunks") {
  auto const src = make_seq(8192);
  constexpr std::size_t chunk = 1024;

  polliwog::compressor c(polliwog::format::zlib,
                         polliwog::zlib_level::default_level);

  std::vector<byte> compressed;
  compressed.reserve(src.size());

  // compress in chunks
  std::array<byte, chunk * 2> cbuf{};
  for (std::size_t off = 0; off < src.size(); off += chunk) {
    auto n = std::min(chunk, src.size() - off);
    auto r = c.push(std::span<const byte>{src.data() + off, n},
                    std::span<byte>{cbuf});
    REQUIRE(r.has_value());
    compressed.insert(compressed.end(), cbuf.begin(),
                      cbuf.begin() +
                          static_cast<std::ptrdiff_t>(r->bytes_produced));
  }

  // finish
  auto fin = c.finish(std::span<byte>{cbuf});
  REQUIRE(fin.has_value());
  REQUIRE(fin->done);
  compressed.insert(compressed.end(), cbuf.begin(),
                    cbuf.begin() +
                        static_cast<std::ptrdiff_t>(fin->bytes_produced));

  // decompress
  polliwog::decompressor d(polliwog::format::zlib);
  std::vector<byte> decompressed;
  decompressed.reserve(src.size());
  std::array<byte, chunk * 2> dbuf{};

  std::size_t coff = 0;
  while (coff < compressed.size()) {
    auto n = std::min(chunk, compressed.size() - coff);
    auto r = d.push(std::span<const byte>{compressed.data() + coff, n},
                    std::span<byte>{dbuf});
    REQUIRE(r.has_value());
    decompressed.insert(decompressed.end(), dbuf.begin(),
                        dbuf.begin() +
                            static_cast<std::ptrdiff_t>(r->bytes_produced));
    coff += static_cast<std::size_t>(r->bytes_consumed);
    if (r->done)
      break;
  }

  REQUIRE(decompressed.size() == src.size());
  REQUIRE(std::equal(src.begin(), src.end(), decompressed.begin()));
}

TEST_CASE("streaming flush emits sync marker then round-trips") {
  auto const src = make_seq(4096);

  polliwog::compressor c(polliwog::format::zlib,
                         polliwog::zlib_level::default_level);

  // push half
  std::array<byte, 8192> cbuf{};
  auto r1 = c.push(std::span<const byte>{src.data(), src.size() / 2},
                   std::span<byte>{cbuf});
  REQUIRE(r1.has_value());
  std::vector<byte> compressed;
  compressed.insert(compressed.end(), cbuf.begin(),
                    cbuf.begin() +
                        static_cast<std::ptrdiff_t>(r1->bytes_produced));

  // flush
  auto fl = c.flush(std::span<byte>{cbuf});
  REQUIRE(fl.has_value());
  auto foff = compressed.size();
  compressed.insert(compressed.end(), cbuf.begin(),
                    cbuf.begin() +
                        static_cast<std::ptrdiff_t>(fl->bytes_produced));

  // the sync marker 0x00 0x00 0xff 0xff must appear in the flush
  // output (rfc 1951 section 2.4)
  bool found_marker = false;
  for (std::size_t i = foff;
       i + 3 < compressed.size() && i < foff + fl->bytes_produced; ++i) {
    if (static_cast<unsigned char>(compressed[i]) == 0x00 &&
        static_cast<unsigned char>(compressed[i + 1]) == 0x00 &&
        static_cast<unsigned char>(compressed[i + 2]) == 0xff &&
        static_cast<unsigned char>(compressed[i + 3]) == 0xff) {
      found_marker = true;
      break;
    }
  }
  REQUIRE(found_marker);

  // push rest and finish
  auto r2 = c.push(std::span<const byte>{src.data() + src.size() / 2,
                                         src.size() - src.size() / 2},
                   std::span<byte>{cbuf});
  REQUIRE(r2.has_value());
  compressed.insert(compressed.end(), cbuf.begin(),
                    cbuf.begin() +
                        static_cast<std::ptrdiff_t>(r2->bytes_produced));
  auto fin = c.finish(std::span<byte>{cbuf});
  REQUIRE(fin.has_value());
  compressed.insert(compressed.end(), cbuf.begin(),
                    cbuf.begin() +
                        static_cast<std::ptrdiff_t>(fin->bytes_produced));

  // decompress the whole thing
  polliwog::decompressor d(polliwog::format::zlib);
  std::vector<byte> decompressed;
  std::array<byte, 8192> dbuf{};
  std::size_t coff = 0;
  while (coff < compressed.size()) {
    auto n = std::min(static_cast<std::size_t>(4096), compressed.size() - coff);
    auto dr = d.push(std::span<const byte>{compressed.data() + coff, n},
                     std::span<byte>{dbuf});
    REQUIRE(dr.has_value());
    decompressed.insert(decompressed.end(), dbuf.begin(),
                        dbuf.begin() +
                            static_cast<std::ptrdiff_t>(dr->bytes_produced));
    coff += static_cast<std::size_t>(dr->bytes_consumed);
    if (dr->done)
      break;
  }

  REQUIRE(decompressed.size() == src.size());
  REQUIRE(std::equal(src.begin(), src.end(), decompressed.begin()));
}

TEST_CASE("streaming decompressor buffer_too_small") {
  auto const src = make_seq(4096);

  // compress normally
  std::size_t cap = polliwog::bound(
      polliwog::format::zlib, polliwog::zlib_level::default_level, src.size());
  std::vector<byte> compressed(cap);
  auto sr = polliwog::squeeze(
      polliwog::format::zlib, polliwog::zlib_level::default_level,
      std::span<const byte>{src}, std::span<byte>{compressed});
  REQUIRE(sr.has_value());

  // decompress with a tiny output buffer; the decompressor makes
  // partial progress and reports done=false
  polliwog::decompressor d(polliwog::format::zlib);
  std::array<byte, 8> tiny{};
  auto r =
      d.push(std::span<const byte>(compressed.data(),
                                   static_cast<std::size_t>(sr->bytes_written)),
             std::span<byte>(tiny));
  // push_more returns partial progress, not an error, when the
  // output fills before the stream ends
  REQUIRE(r.has_value());
  REQUIRE_FALSE(r->done);
  REQUIRE(r->bytes_produced <= 8);
}

TEST_CASE("streaming decompressor stream_corrupt on garbage") {
  std::array<byte, 32> garbage{};
  for (std::size_t i = 0; i < garbage.size(); ++i)
    garbage[i] = static_cast<byte>(0xdd);

  std::array<byte, 256> out{};
  polliwog::decompressor d(polliwog::format::zlib);
  auto r = d.push(std::span<const byte>{garbage}, std::span<byte>{out});
  REQUIRE_FALSE(r.has_value());
  REQUIRE(r.error().k == polliwog::kind::stream_corrupt);
}
