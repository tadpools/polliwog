// detail/zstd.cpp - the zstd backend. raii handles over ZSTD_CCtx /
// ZSTD_DCtx, stable API only (v1.5.7, versions.md).

#include "detail/zstd.hpp"

#include <zstd.h>

#include <algorithm>
#include <cassert>
#include <limits>
#include <utility>

namespace polliwog::detail {

// -- zstd_squeezer --------------------------------------------------------

zstd_squeezer::zstd_squeezer(format f, zstd_level lvl)
    : fmt_(f), level_(static_cast<int>(lvl)) {
  ctx_ = ZSTD_createCCtx();
  assert(ctx_ != nullptr);

  // set the compression level via the advanced API. level 0 means
  // "use default" (ZSTD_CLEVEL_DEFAULT=3). this parameter is sticky
  // for streaming via ZSTD_compressStream2.
  auto ret =
      ZSTD_CCtx_setParameter(ctx_, ZSTD_c_compressionLevel, level_);
  assert(!ZSTD_isError(ret));
  (void)ret;
}

zstd_squeezer::zstd_squeezer(zstd_squeezer &&o) noexcept
    : ctx_(o.ctx_), fmt_(o.fmt_), level_(o.level_) {
  o.ctx_ = nullptr;
}

zstd_squeezer &zstd_squeezer::operator=(zstd_squeezer &&o) noexcept {
  if (this != &o) {
    if (ctx_)
      ZSTD_freeCCtx(ctx_);
    ctx_ = o.ctx_;
    fmt_ = o.fmt_;
    level_ = o.level_;
    o.ctx_ = nullptr;
  }
  return *this;
}

zstd_squeezer::~zstd_squeezer() {
  if (ctx_)
    ZSTD_freeCCtx(ctx_);
}

result zstd_squeezer::push(std::span<const std::byte> in,
                           std::span<std::byte> out) {
  // one-shot compress using the simple API. ZSTD_compress creates a
  // complete frame (header + content + epilogue) in one call. the
  // level is passed directly here rather than through the CCtx, since
  // ZSTD_compress does not use the CCtx's parameters.
  auto ret = ZSTD_compress(out.data(), out.size(), in.data(), in.size(),
                           level_);

  if (ZSTD_isError(ret)) {
    return std::unexpected(error{kind::backend_failure, backend_id::zstd, 0,
                                 ZSTD_getErrorName(ret)});
  }

  return progress{in.size(), ret, true};
}

result zstd_squeezer::push_more(std::span<const std::byte> in,
                                std::span<std::byte> out) {
  ZSTD_inBuffer input{in.data(), in.size(), 0};
  ZSTD_outBuffer output{out.data(), out.size(), 0};

  auto ret = ZSTD_compressStream2(ctx_, &output, &input, ZSTD_e_continue);

  if (ZSTD_isError(ret)) {
    return std::unexpected(error{kind::backend_failure, backend_id::zstd, 0,
                                 ZSTD_getErrorName(ret)});
  }

  return progress{input.pos, output.pos, false};
}

result zstd_squeezer::flush(std::span<std::byte> out) {
  ZSTD_inBuffer input{nullptr, 0, 0};
  ZSTD_outBuffer output{out.data(), out.size(), 0};

  auto ret = ZSTD_compressStream2(ctx_, &output, &input, ZSTD_e_flush);

  if (ZSTD_isError(ret)) {
    return std::unexpected(error{kind::backend_failure, backend_id::zstd, 0,
                                 ZSTD_getErrorName(ret)});
  }

  return progress{0, output.pos, false};
}

result zstd_squeezer::finish(std::span<std::byte> out) {
  ZSTD_inBuffer input{nullptr, 0, 0};
  ZSTD_outBuffer output{out.data(), out.size(), 0};

  // ZSTD_e_end flushes remaining data and closes the frame.
  // must call until ret == 0 (all data flushed).
  size_t remaining;
  do {
    remaining = ZSTD_compressStream2(ctx_, &output, &input, ZSTD_e_end);
    if (ZSTD_isError(remaining)) {
      return std::unexpected(error{kind::backend_failure, backend_id::zstd, 0,
                                   ZSTD_getErrorName(remaining)});
    }
    // if output buffer is full and there's still data, report progress
    // and let the caller call again
    if (remaining > 0 && output.pos == output.size) {
      return progress{0, output.pos, false};
    }
  } while (remaining > 0);

  return progress{0, output.pos, true};
}

// -- zstd_sweller ---------------------------------------------------------

zstd_sweller::zstd_sweller(format f) : fmt_(f) {
  ctx_ = ZSTD_createDCtx();
  assert(ctx_ != nullptr);
}

zstd_sweller::zstd_sweller(zstd_sweller &&o) noexcept
    : ctx_(o.ctx_), fmt_(o.fmt_) {
  o.ctx_ = nullptr;
}

zstd_sweller &zstd_sweller::operator=(zstd_sweller &&o) noexcept {
  if (this != &o) {
    if (ctx_)
      ZSTD_freeDCtx(ctx_);
    ctx_ = o.ctx_;
    fmt_ = o.fmt_;
    o.ctx_ = nullptr;
  }
  return *this;
}

zstd_sweller::~zstd_sweller() {
  if (ctx_)
    ZSTD_freeDCtx(ctx_);
}

result zstd_sweller::push(std::span<const std::byte> in,
                          std::span<std::byte> out) {
  // one-shot decompress
  auto ret =
      ZSTD_decompress(out.data(), out.size(), in.data(), in.size());

  if (ZSTD_isError(ret)) {
    return std::unexpected(error{kind::stream_corrupt, backend_id::zstd, 0,
                                 ZSTD_getErrorName(ret)});
  }

  return progress{in.size(), ret, true};
}

result zstd_sweller::push_more(std::span<const std::byte> in,
                               std::span<std::byte> out) {
  ZSTD_inBuffer input{in.data(), in.size(), 0};
  ZSTD_outBuffer output{out.data(), out.size(), 0};

  auto ret = ZSTD_decompressStream(ctx_, &output, &input);

  if (ZSTD_isError(ret)) {
    return std::unexpected(error{kind::stream_corrupt, backend_id::zstd, 0,
                                 ZSTD_getErrorName(ret)});
  }

  // ret > 0 means there is more frame data to consume
  // ret == 0 means frame is fully decoded
  bool done = (ret == 0);

  // check for extra data after frame end (single-frame policy, same
  // as gzip D10)
  if (done && input.pos < input.size) {
    return std::unexpected(error{kind::stream_corrupt, backend_id::zstd,
                                 static_cast<std::uint64_t>(input.pos),
                                 "extra data after end of stream"});
  }

  return progress{input.pos, output.pos, done};
}

// -- zstd_backend ---------------------------------------------------------

static constexpr format zstd_formats[] = {format::zstd};

std::span<const format> zstd_backend::formats() { return {zstd_formats, 1}; }

zstd_squeezer zstd_backend::make_squeezer(format f, level lvl) {
  return zstd_squeezer{f, lvl};
}

zstd_sweller zstd_backend::make_sweller(format f) { return zstd_sweller{f}; }

std::size_t zstd_backend::bound(level, std::uint64_t in_size) {
  size_t r = ZSTD_compressBound(in_size);
  return ZSTD_isError(r) ? std::numeric_limits<std::size_t>::max()
                          : static_cast<std::size_t>(r);
}

result zstd_backend::push(handle &h, std::span<const std::byte> in,
                          std::span<std::byte> out) {
  return h.push(in, out);
}

result zstd_backend::flush(handle &h, std::span<std::byte> out) {
  return h.flush(out);
}

result zstd_backend::finish(handle &h, std::span<std::byte> out) {
  return h.finish(out);
}

} // namespace polliwog::detail
