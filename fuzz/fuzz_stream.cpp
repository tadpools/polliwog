// fuzz_stream.cpp - libfuzzer target for streaming compressor and
// decompressor. feeds raw bytes through push/pull/finish in random
// chunk sizes. catches crashes andubsan findings. Stability: internal.

#include "polliwog/stream.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  if (size == 0)
    return 0;

  std::span<const std::byte> in{reinterpret_cast<const std::byte *>(data),
                                size};

  // output buffer: generous to avoid buffer_too_small dominating
  std::byte out[65536]{};

  // compress through the streaming compressor
  polliwog::compressor comp(polliwog::format::zlib,
                            polliwog::zlib_level::default_level);

  std::size_t pos = 0;
  while (pos < size) {
    // feed in random-sized chunks (1 to 64 bytes, or rest of input)
    std::size_t chunk = (data[pos] & 0x3f) + 1;
    if (chunk > size - pos)
      chunk = size - pos;

    auto r = comp.push(std::span<const std::byte>{in.data() + pos, chunk},
                       std::span<std::byte>{out});
    if (!r)
      return 0;
    pos += r->bytes_in;
  }

  // finish the stream
  auto fr = comp.finish(std::span<std::byte>{out});
  if (!fr.has_value())
    return 0;

  // now decompress what we just compressed
  std::span<const std::byte> compressed{
      out, static_cast<std::size_t>(fr->bytes_out)};
  std::byte decompressed[65536]{};

  polliwog::decompressor dec(polliwog::format::zlib);
  std::size_t cpos = 0;
  while (cpos < compressed.size()) {
    std::size_t chunk = (data[cpos % size] & 0x3f) + 1;
    if (chunk > compressed.size() - cpos)
      chunk = compressed.size() - cpos;

    auto r =
        dec.push(std::span<const std::byte>{compressed.data() + cpos, chunk},
                 std::span<std::byte>{decompressed});
    if (!r)
      return 0;
    if (r->done)
      break;
    cpos += r->bytes_in;
  }

  return 0;
}
