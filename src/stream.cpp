// polliwog/stream.cpp - streaming types, thin wrappers over the
// backend. supports zlib and zstd via compile-time backend selection.

#include "polliwog/stream.hpp"

#ifdef POLLIWOG_HAS_ZLIB
#include "detail/zlib.hpp"
#endif
#ifdef POLLIWOG_HAS_ZSTD
#include "detail/zstd.hpp"
#endif

#include <stdexcept>
#include <variant>

namespace polliwog {

// -- helpers to construct the right handle variant -------------------------

namespace {

// stub types used when a backend is not compiled in. these are
// never instantiated at runtime; they exist only so the variant
// type and visit helpers compile.
#ifndef POLLIWOG_HAS_ZLIB
struct zlib_stub_squeezer {
  zlib_stub_squeezer(format, zlib_level) {}
  bool push_more(std::span<const std::byte>, std::span<std::byte>) {
    return false;
  }
  bool flush(std::span<std::byte>) { return false; }
  bool finish(std::span<std::byte>) { return false; }
};
struct zlib_stub_sweller {
  explicit zlib_stub_sweller(format) {}
  bool push_more(std::span<const std::byte>, std::span<std::byte>) {
    return false;
  }
};
#endif

#ifndef POLLIWOG_HAS_ZSTD
struct zstd_stub_squeezer {
  zstd_stub_squeezer(format, int) {}
  bool push_more(std::span<const std::byte>, std::span<std::byte>) {
    return false;
  }
  bool flush(std::span<std::byte>) { return false; }
  bool finish(std::span<std::byte>) { return false; }
};
struct zstd_stub_sweller {
  explicit zstd_stub_sweller(format) {}
  bool push_more(std::span<const std::byte>, std::span<std::byte>) {
    return false;
  }
};
#endif

// helper: build a format_mismatch error
inline error format_mismatch_err() {
  return error{kind::format_mismatch, backend_id::none, 0, ""};
}

// visit helpers: each visitor returns the same type (std::expected<detail::result, error>)
// using a common return wrapper. we define a simple struct to hold
// push/flush/finish results.
struct push_out {
  std::size_t bytes_in{};
  std::size_t bytes_out{};
  bool done{false};
};

// for the real backends, we need to forward to their methods and
// adapt the return type. the cleanest way: each visitor adapts.
// we use a generic visitor that works with all handle types.

struct push_visitor {
  std::span<const std::byte> in;
  std::span<std::byte> out;

#ifdef POLLIWOG_HAS_ZLIB
  std::expected<push_out, error> operator()(detail::zlib_squeezer &h) {
    auto r = h.push_more(in, out);
    if (!r)
      return std::unexpected(r.error());
    return push_out{r->bytes_in, r->bytes_out, false};
  }
  std::expected<push_out, error> operator()(detail::zlib_sweller &h) {
    auto r = h.push_more(in, out);
    if (!r)
      return std::unexpected(r.error());
    return push_out{r->bytes_in, r->bytes_out, r->done};
  }
#endif
#ifdef POLLIWOG_HAS_ZSTD
  std::expected<push_out, error> operator()(detail::zstd_squeezer &h) {
    auto r = h.push_more(in, out);
    if (!r)
      return std::unexpected(r.error());
    return push_out{r->bytes_in, r->bytes_out, false};
  }
  std::expected<push_out, error> operator()(detail::zstd_sweller &h) {
    auto r = h.push_more(in, out);
    if (!r)
      return std::unexpected(r.error());
    return push_out{r->bytes_in, r->bytes_out, r->done};
  }
#endif

  // stubs: when backend not compiled in, always fail with format_mismatch
#ifndef POLLIWOG_HAS_ZLIB
  std::expected<push_out, error> operator()(zlib_stub_squeezer &) {
    return std::unexpected(format_mismatch_err());
  }
  std::expected<push_out, error> operator()(zlib_stub_sweller &) {
    return std::unexpected(format_mismatch_err());
  }
#endif
#ifndef POLLIWOG_HAS_ZSTD
  std::expected<push_out, error> operator()(zstd_stub_squeezer &) {
    return std::unexpected(format_mismatch_err());
  }
  std::expected<push_out, error> operator()(zstd_stub_sweller &) {
    return std::unexpected(format_mismatch_err());
  }
#endif
};

struct flush_visitor {
  std::span<std::byte> out;

#ifdef POLLIWOG_HAS_ZLIB
  std::expected<push_out, error> operator()(detail::zlib_squeezer &h) {
    auto r = h.flush(out);
    if (!r)
      return std::unexpected(r.error());
    return push_out{0, r->bytes_out, false};
  }
#endif
#ifdef POLLIWOG_HAS_ZSTD
  std::expected<push_out, error> operator()(detail::zstd_squeezer &h) {
    auto r = h.flush(out);
    if (!r)
      return std::unexpected(r.error());
    return push_out{0, r->bytes_out, false};
  }
#endif
#ifndef POLLIWOG_HAS_ZLIB
  std::expected<push_out, error> operator()(zlib_stub_squeezer &) {
    return std::unexpected(format_mismatch_err());
  }
#endif
#ifndef POLLIWOG_HAS_ZSTD
  std::expected<push_out, error> operator()(zstd_stub_squeezer &) {
    return std::unexpected(format_mismatch_err());
  }
#endif
};

struct finish_visitor {
  std::span<std::byte> out;

#ifdef POLLIWOG_HAS_ZLIB
  std::expected<push_out, error> operator()(detail::zlib_squeezer &h) {
    auto r = h.finish(out);
    if (!r)
      return std::unexpected(r.error());
    return push_out{0, r->bytes_out, true};
  }
#endif
#ifdef POLLIWOG_HAS_ZSTD
  std::expected<push_out, error> operator()(detail::zstd_squeezer &h) {
    auto r = h.finish(out);
    if (!r)
      return std::unexpected(r.error());
    return push_out{0, r->bytes_out, true};
  }
#endif
#ifndef POLLIWOG_HAS_ZLIB
  std::expected<push_out, error> operator()(zlib_stub_squeezer &) {
    return std::unexpected(format_mismatch_err());
  }
#endif
#ifndef POLLIWOG_HAS_ZSTD
  std::expected<push_out, error> operator()(zstd_stub_squeezer &) {
    return std::unexpected(format_mismatch_err());
  }
#endif
};

// variant aliases
using squeezer_variant =
    std::variant<
#ifdef POLLIWOG_HAS_ZLIB
      detail::zlib_squeezer,
#else
      zlib_stub_squeezer,
#endif
#ifdef POLLIWOG_HAS_ZSTD
      detail::zstd_squeezer
#else
      zstd_stub_squeezer
#endif
    >;

using sweller_variant =
    std::variant<
#ifdef POLLIWOG_HAS_ZLIB
      detail::zlib_sweller,
#else
      zlib_stub_sweller,
#endif
#ifdef POLLIWOG_HAS_ZSTD
      detail::zstd_sweller
#else
      zstd_stub_sweller
#endif
    >;

squeezer_variant make_squeezer(format f, zlib_level lvl) {
  switch (f) {
  case format::zlib:
  case format::gzip:
  case format::raw_deflate:
#ifdef POLLIWOG_HAS_ZLIB
    return detail::zlib_squeezer{f, lvl};
#else
    return zlib_stub_squeezer{f, lvl};
#endif
  case format::zstd:
#ifdef POLLIWOG_HAS_ZSTD
    return detail::zstd_squeezer{f, static_cast<zstd_level>(static_cast<int>(lvl))};
#else
    return zstd_stub_squeezer{f, static_cast<int>(lvl)};
#endif
  default:
#ifdef POLLIWOG_HAS_ZLIB
    return detail::zlib_squeezer{f, lvl};
#else
    return zlib_stub_squeezer{f, lvl};
#endif
  }
}

sweller_variant make_sweller(format f) {
  switch (f) {
  case format::zlib:
  case format::gzip:
  case format::raw_deflate:
#ifdef POLLIWOG_HAS_ZLIB
    return detail::zlib_sweller{f};
#else
    return zlib_stub_sweller{f};
#endif
  case format::zstd:
#ifdef POLLIWOG_HAS_ZSTD
    return detail::zstd_sweller{f};
#else
    return zstd_stub_sweller{f};
#endif
  default:
#ifdef POLLIWOG_HAS_ZLIB
    return detail::zlib_sweller{f};
#else
    return zlib_stub_sweller{f};
#endif
  }
}

} // namespace

// -- compressor -----------------------------------------------------------

struct compressor::impl {
  squeezer_variant handle;
  impl(format f, zlib_level lvl) : handle(make_squeezer(f, lvl)) {}
};

compressor::compressor(format f, zlib_level lvl) {
  if (f != format::zstd) {
    auto const v = static_cast<int>(lvl);
    if (v < 0 || v > 9)
      throw std::invalid_argument("level_unsupported: zlib levels are 0-9");
  }
  p_ = std::make_unique<impl>(f, lvl);
}

compressor::~compressor() = default;

compressor::compressor(compressor &&) noexcept = default;
compressor &compressor::operator=(compressor &&) noexcept = default;

std::expected<stream_result, error>
compressor::push(std::span<const std::byte> in, std::span<std::byte> out) {
  auto r = std::visit(push_visitor{in, out}, p_->handle);
  if (!r)
    return std::unexpected(r.error());
  return stream_result{r->bytes_in, r->bytes_out, false};
}

std::expected<stream_result, error>
compressor::flush(std::span<std::byte> out) {
  auto r = std::visit(flush_visitor{out}, p_->handle);
  if (!r)
    return std::unexpected(r.error());
  return stream_result{0, r->bytes_out, false};
}

std::expected<stream_result, error>
compressor::finish(std::span<std::byte> out) {
  auto r = std::visit(finish_visitor{out}, p_->handle);
  if (!r)
    return std::unexpected(r.error());
  return stream_result{r->bytes_in, r->bytes_out, true};
}

// -- decompressor ---------------------------------------------------------

struct decompressor::impl {
  sweller_variant handle;
  explicit impl(format f) : handle(make_sweller(f)) {}
};

decompressor::decompressor(format f) : p_(std::make_unique<impl>(f)) {}

decompressor::~decompressor() = default;

decompressor::decompressor(decompressor &&) noexcept = default;
decompressor &decompressor::operator=(decompressor &&) noexcept = default;

std::expected<stream_result, error>
decompressor::push(std::span<const std::byte> in, std::span<std::byte> out) {
  auto r = std::visit(push_visitor{in, out}, p_->handle);
  if (!r)
    return std::unexpected(r.error());
  return stream_result{r->bytes_in, r->bytes_out, r->done};
}

} // namespace polliwog
