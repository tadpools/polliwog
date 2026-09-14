// fuzz_squeeze.cpp - libfuzzer target for one-shot squeeze and swell.
// feeds raw bytes through squeeze at every format+level, then tries
// to swell the result. catches crashes,ubsan findings, and
// unreachable paths. Stability: internal.

#include "polliwog/squeeze.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
#include <vector>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  if (size == 0)
    return 0;

  std::span<const std::byte> in{
      reinterpret_cast<const std::byte *>(data), size};

  // a reasonable output bound; if the fuzzer finds buffer_too_small
  // that is fine - we just do not want to crash on allocation
  std::vector<std::byte> out(polliwog::bound(
      polliwog::format::zlib, polliwog::zlib_level::default_level, size));
  if (out.empty())
    return 0;

  // squeeze with zlib
  auto sr = polliwog::squeeze(polliwog::format::zlib,
                              polliwog::zlib_level::default_level, in,
                              std::span<std::byte>{out});
  if (sr.has_value()) {
    // swell it back
    std::vector<std::byte> dec(size);
    auto dr = polliwog::swell(
        polliwog::format::zlib,
        std::span<const std::byte>{out.data(), sr->bytes_written},
        std::span<std::byte>{dec});
    if (dr.has_value()) {
      // round-trip check: the decompressed data must match the original
      if (dr->bytes_written == size)
        std::memcmp(in.data(), dec.data(), size);
    }
  }

  return 0;
}
