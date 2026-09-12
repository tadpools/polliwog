// polliwog/squeeze.cpp - one-shot layer, dispatches through the
// backend selected by the format enum.

#include "polliwog/squeeze.hpp"

#include "detail/zlib.hpp"

namespace polliwog {

std::size_t bound(format f, zlib_level lvl, std::uint64_t in_size) {
  switch (f) {
  case format::zlib:
  case format::gzip:
  case format::raw_deflate:
    return detail::zlib_backend::bound(lvl, in_size);
  default:
    return 0;
  }
}

std::expected<squeeze_result, error> squeeze(format f, zlib_level lvl,
                                             std::span<const std::byte> in,
                                             std::span<std::byte> out) {
  std::size_t cap = bound(f, lvl, in.size());
  if (cap == 0)
    return std::unexpected(
        error{kind::format_mismatch, backend_id::none, 0, ""});
  if (out.size() < cap)
    return std::unexpected(
        error{kind::buffer_too_small, backend_id::none, 0, ""});

  auto squeezer = detail::zlib_backend::make_squeezer(f, lvl);
  auto r = squeezer.push(in, out);
  if (!r)
    return std::unexpected(r.error());

  // gzip determinism: zlib writes mtime=0 (hardcoded) but the OS
  // byte is a platform constant (OS_CODE). patch it to 0xff
  // (unknown) so the output is identical on every platform.
  if (f == format::gzip && r->bytes_out >= 10)
    out[9] = std::byte{0xff};

  return squeeze_result{r->bytes_out};
}

std::expected<squeeze_result, error>
swell(format f, std::span<const std::byte> in, std::span<std::byte> out) {
  switch (f) {
  case format::zlib:
  case format::gzip:
  case format::raw_deflate:
    break;
  default:
    return std::unexpected(
        error{kind::format_mismatch, backend_id::none, 0, ""});
  }

  auto sweller = detail::zlib_backend::make_sweller(f);
  auto r = sweller.push(in, out);
  if (!r)
    return std::unexpected(r.error());
  return squeeze_result{r->bytes_out};
}

} // namespace polliwog
