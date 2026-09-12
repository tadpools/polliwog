// polliwog/stream.cpp - streaming types, thin wrappers over the
// backend.

#include "polliwog/stream.hpp"

#include "detail/zlib.hpp"

namespace polliwog {

// -- compressor -----------------------------------------------------------

struct compressor::impl {
  detail::zlib_squeezer handle;
  impl(format f, zlib_level lvl) : handle(f, lvl) {}
};

compressor::compressor(format f, zlib_level lvl)
    : p_(std::make_unique<impl>(f, lvl)) {}

compressor::~compressor() = default;

compressor::compressor(compressor &&) noexcept = default;
compressor &compressor::operator=(compressor &&) noexcept = default;

std::expected<stream_result, error>
compressor::push(std::span<const std::byte> in, std::span<std::byte> out) {
  auto r = p_->handle.push_more(in, out);
  if (!r)
    return std::unexpected(r.error());
  return stream_result{r->bytes_in, r->bytes_out, false};
}

std::expected<stream_result, error>
compressor::flush(std::span<std::byte> out) {
  auto r = p_->handle.flush(out);
  if (!r)
    return std::unexpected(r.error());
  return stream_result{0, r->bytes_out, false};
}

std::expected<stream_result, error>
compressor::finish(std::span<std::byte> out) {
  auto r = p_->handle.finish(out);
  if (!r)
    return std::unexpected(r.error());
  return stream_result{r->bytes_in, r->bytes_out, true};
}

// -- decompressor ---------------------------------------------------------

struct decompressor::impl {
  detail::zlib_sweller handle;
  explicit impl(format f) : handle(f) {}
};

decompressor::decompressor(format f) : p_(std::make_unique<impl>(f)) {}

decompressor::~decompressor() = default;

decompressor::decompressor(decompressor &&) noexcept = default;
decompressor &decompressor::operator=(decompressor &&) noexcept = default;

std::expected<stream_result, error>
decompressor::push(std::span<const std::byte> in, std::span<std::byte> out) {
  auto r = p_->handle.push_more(in, out);
  if (!r)
    return std::unexpected(r.error());
  return stream_result{r->bytes_in, r->bytes_out, r->done};
}

} // namespace polliwog
