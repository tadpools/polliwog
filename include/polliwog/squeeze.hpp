// polliwog/squeeze.hpp - one-shot compress and decompress.
// Stability: Growing .
//
// squeeze(format, level, in, out): compress in into out, return
// bytes written on success. bound() gives the honest upper size for
// the output span; if out is smaller than that, buffer_too_small
// fires before any bytes are written.
//
// swell(format, in, out): decompress in into out. the caller
// provides out large enough to hold the original; if it does not fit,
// buffer_too_small names the gap. the input must be the exact
// compressed stream (squeeze's bytes_written worth).
//
// the one-shot layer is a pure function: no globals, no statics, no
// hidden state. concurrent calls on distinct buffers are safe by
// contract.
//
// see also: detail/backend.hpp, polliwog/format.hpp

#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>

#include "polliwog/error.hpp"
#include "polliwog/format.hpp"

namespace polliwog {

// the result of a one-shot squeeze
struct squeeze_result {
  std::uint64_t bytes_written{0};
};

// honest upper bound on the compressed size. safe for all containers
// (zlib, gzip, raw deflate); for formats without a backend yet,
// returns 0 and squeeze will fail with format_mismatch.
std::size_t bound(format f, zlib_level lvl, std::uint64_t in_size);

// compress in into out. out must be at least bound(f, lvl,
// in.size()) bytes. returns bytes_written on success.
std::expected<squeeze_result, error> squeeze(format f, zlib_level lvl,
                                             std::span<const std::byte> in,
                                             std::span<std::byte> out);

// decompress in into out. out must hold the original data. in is
// the exact compressed stream from squeeze (in.size() ==
// squeeze's bytes_written).
std::expected<squeeze_result, error>
swell(format f, std::span<const std::byte> in, std::span<std::byte> out);

} // namespace polliwog
