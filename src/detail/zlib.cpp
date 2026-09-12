// detail/zlib.cpp - the zlib backend. raii handles over z_stream,
// the _z entry points preferred (zlib 1.3.2, research verified).

#include "detail/zlib.hpp"

#include <zlib.h>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <limits>
#include <utility>

namespace polliwog::detail {

int window_bits_for(format f) {
  switch (f) {
  case format::zlib:
    return 15;
  case format::gzip:
    return 15 + 16;
  case format::raw_deflate:
    return -15;
  default:
    return 15;
  }
}

// -- zlib_squeezer --------------------------------------------------------

zlib_squeezer::zlib_squeezer(format f, zlib_level lvl)
    : stream_(new z_stream_s{}) {
  stream_->zalloc = Z_NULL;
  stream_->zfree = Z_NULL;
  stream_->opaque = Z_NULL;

  int wbits = window_bits_for(f);
  int ret = deflateInit2(stream_.get(), static_cast<int>(lvl), Z_DEFLATED,
                         wbits, 8, Z_DEFAULT_STRATEGY);
  assert(ret == Z_OK);
  (void)ret;
}

zlib_squeezer::zlib_squeezer(zlib_squeezer &&o) noexcept = default;
zlib_squeezer &zlib_squeezer::operator=(zlib_squeezer &&o) noexcept = default;

zlib_squeezer::~zlib_squeezer() {
  if (stream_)
    deflateEnd(stream_.get());
}

result zlib_squeezer::push(std::span<const std::byte> in,
                           std::span<std::byte> out) {
  stream_->next_in =
      reinterpret_cast<Bytef *>(const_cast<std::byte *>(in.data()));
  stream_->avail_in = static_cast<uInt>(in.size());
  stream_->next_out = reinterpret_cast<Bytef *>(out.data());
  stream_->avail_out = static_cast<uInt>(out.size());

  std::uint64_t before_in = stream_->total_in;
  std::uint64_t before_out = stream_->total_out;

  int ret = deflate(stream_.get(), Z_FINISH);
  if (ret != Z_STREAM_END && ret != Z_OK) {
    return std::unexpected(
        error{kind::backend_failure, backend_id::zlib, 0, "deflate"});
  }

  std::uint64_t consumed = stream_->total_in - before_in;
  std::uint64_t produced = stream_->total_out - before_out;
  bool done = (ret == Z_STREAM_END);
  return progress{consumed, produced, done};
}

result zlib_squeezer::flush(std::span<std::byte> out) {
  stream_->next_in = Z_NULL;
  stream_->avail_in = 0;
  stream_->next_out = reinterpret_cast<Bytef *>(out.data());
  stream_->avail_out = static_cast<uInt>(out.size());

  std::uint64_t before_out = stream_->total_out;

  int ret = deflate(stream_.get(), Z_SYNC_FLUSH);
  if (ret != Z_OK && ret != Z_BUF_ERROR) {
    return std::unexpected(
        error{kind::backend_failure, backend_id::zlib, 0, "deflate flush"});
  }

  std::uint64_t produced = stream_->total_out - before_out;
  return progress{0, produced, false};
}

result zlib_squeezer::finish(std::span<std::byte> out) { return push({}, out); }

// -- zlib_sweller ---------------------------------------------------------

zlib_sweller::zlib_sweller(format f) : stream_(new z_stream_s{}) {
  stream_->zalloc = Z_NULL;
  stream_->zfree = Z_NULL;
  stream_->opaque = Z_NULL;
  stream_->next_in = Z_NULL;
  stream_->avail_in = 0;

  int wbits = window_bits_for(f);
  int ret = inflateInit2(stream_.get(), wbits);
  assert(ret == Z_OK);
  (void)ret;
}

zlib_sweller::zlib_sweller(zlib_sweller &&o) noexcept = default;
zlib_sweller &zlib_sweller::operator=(zlib_sweller &&o) noexcept = default;

zlib_sweller::~zlib_sweller() {
  if (stream_)
    inflateEnd(stream_.get());
}

result zlib_sweller::push(std::span<const std::byte> in,
                          std::span<std::byte> out) {
  stream_->next_in =
      reinterpret_cast<Bytef *>(const_cast<std::byte *>(in.data()));
  stream_->avail_in = static_cast<uInt>(in.size());
  stream_->next_out = reinterpret_cast<Bytef *>(out.data());
  stream_->avail_out = static_cast<uInt>(out.size());

  std::uint64_t before_in = stream_->total_in;
  std::uint64_t before_out = stream_->total_out;

  int ret = inflate(stream_.get(), Z_FINISH);
  std::uint64_t consumed = stream_->total_in - before_in;
  std::uint64_t produced = stream_->total_out - before_out;

  if (ret == Z_STREAM_END) {
    // stream ended; leftover input means extra data (concatenated
    // gzip members or trailing garbage), per D10 policy
    if (consumed < in.size())
      return std::unexpected(
          error{kind::stream_corrupt, backend_id::zlib,
                static_cast<std::uint64_t>(stream_->total_in),
                "extra data after end of stream"});
    return progress{consumed, produced, true};
  }

  if (ret == Z_OK || ret == Z_BUF_ERROR) {
    // stream not done; output filled before the stream ended
    if (consumed < in.size())
      return std::unexpected(
          error{kind::buffer_too_small, backend_id::zlib, 0, ""});
    return progress{consumed, produced, false};
  }

  return std::unexpected(error{kind::stream_corrupt, backend_id::zlib,
                               static_cast<std::uint64_t>(stream_->total_in),
                               "inflate"});
}

// -- zlib_backend ---------------------------------------------------------

static constexpr format zlib_formats[] = {format::zlib, format::gzip,
                                          format::raw_deflate};

std::span<const format> zlib_backend::formats() { return {zlib_formats, 3}; }

zlib_squeezer zlib_backend::make_squeezer(format f, level lvl) {
  return zlib_squeezer{f, lvl};
}

zlib_sweller zlib_backend::make_sweller(format f) { return zlib_sweller{f}; }

std::size_t zlib_backend::bound(level, std::uint64_t in_size) {
  // compressBound is an honest upper bound for all three containers;
  // gzip and raw deflate produce less overhead than the zlib wrapper
  // it accounts for
  uLong r = compressBound(static_cast<uLong>(in_size));
  return static_cast<long>(r) == Z_MEM_ERROR
             ? std::numeric_limits<std::size_t>::max()
             : static_cast<std::size_t>(r);
}

result zlib_backend::push(handle &h, std::span<const std::byte> in,
                          std::span<std::byte> out) {
  return h.push(in, out);
}

result zlib_backend::flush(handle &h, std::span<std::byte> out) {
  return h.flush(out);
}

result zlib_backend::finish(handle &h, std::span<std::byte> out) {
  return h.finish(out);
}

} // namespace polliwog::detail
