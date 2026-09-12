// polliwog/stream.hpp - streaming compressor and decompressor.
// Stability: Growing (hatch).
//
// the streaming types wrap the same backend the one-shot layer uses,
// at a different height: multiple push calls accumulate state, flush
// emits a sync marker, finish ends the stream. a beginner's
// one-shot call and a streaming pipeline are built from the same
// parts (philosophy, belief 2).
//
// who reaches for this header: code that compresses or decompresses
// in chunks - a file reader, a network pipe, a brood job. who
// almost never does: code that only squeezes a single buffer (use
// squeeze.hpp).
//
// each streaming object allocates twice: once for the pimpl, once
// for the backend's internal state (z_stream). the pimpl keeps
// internal headers out of this public surface.
//
// flush emits a zlib sync marker (0x00 0x00 0xff 0xff) so the
// receiver can decompress what arrived so far without waiting for
// finish. only sync_flush is supported (D9) until a proven need
// for other flush modes lands with its own fixtures.
//
// see also: polliwog/squeeze.hpp, detail/backend.hpp

#pragma once

#include <cstdint>
#include <expected>
#include <memory>
#include <span>

#include "polliwog/error.hpp"
#include "polliwog/format.hpp"

namespace polliwog {

struct stream_result {
  std::uint64_t bytes_consumed{0};
  std::uint64_t bytes_produced{0};
  bool done{false};
};

class compressor {
public:
  compressor(format f, zlib_level lvl);
  ~compressor();

  compressor(compressor &&) noexcept;
  compressor &operator=(compressor &&) noexcept;

  compressor(const compressor &) = delete;
  compressor &operator=(const compressor &) = delete;

  std::expected<stream_result, error> push(std::span<const std::byte> in,
                                           std::span<std::byte> out);

  std::expected<stream_result, error> flush(std::span<std::byte> out);

  std::expected<stream_result, error> finish(std::span<std::byte> out);

private:
  struct impl;
  std::unique_ptr<impl> p_;
};

class decompressor {
public:
  explicit decompressor(format f);
  ~decompressor();

  decompressor(decompressor &&) noexcept;
  decompressor &operator=(decompressor &&) noexcept;

  decompressor(const decompressor &) = delete;
  decompressor &operator=(const decompressor &) = delete;

  std::expected<stream_result, error> push(std::span<const std::byte> in,
                                           std::span<std::byte> out);

private:
  struct impl;
  std::unique_ptr<impl> p_;
};

} // namespace polliwog
