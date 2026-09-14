// polliwog/squeeze.cpp - one-shot layer, dispatches through the
// backend selected by the format enum.

#include "polliwog/squeeze.hpp"

#ifdef POLLIWOG_HAS_ZLIB
#include "detail/zlib.hpp"
#endif
#ifdef POLLIWOG_HAS_ZSTD
#include "detail/zstd.hpp"
#endif

namespace polliwog {

std::size_t bound(format f, zlib_level lvl, std::uint64_t in_size) {
  switch (f) {
  case format::zlib:
  case format::gzip:
  case format::raw_deflate:
#ifdef POLLIWOG_HAS_ZLIB
    return detail::zlib_backend::bound(lvl, in_size);
#else
    (void)lvl;
    (void)in_size;
    return 0;
#endif
  case format::zstd:
#ifdef POLLIWOG_HAS_ZSTD
    return detail::zstd_backend::bound(zstd_level::default_level, in_size);
#else
    (void)in_size;
    return 0;
#endif
  default:
    return 0;
  }
}

std::expected<squeeze_result, error> squeeze(format f, zlib_level lvl,
                                             std::span<const std::byte> in,
                                             std::span<std::byte> out) {
  switch (f) {
  case format::zlib:
  case format::gzip:
  case format::raw_deflate: {
#ifdef POLLIWOG_HAS_ZLIB
    auto const v = static_cast<int>(lvl);
    if (v < 0 || v > 9)
      return std::unexpected(
          error{kind::level_unsupported, backend_id::zlib, 0, ""});

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
#else
    (void)lvl;
    (void)in;
    (void)out;
    return std::unexpected(
        error{kind::format_mismatch, backend_id::none, 0, ""});
#endif
  }
  case format::zstd: {
#ifdef POLLIWOG_HAS_ZSTD
    std::size_t cap = bound(f, zlib_level::default_level, in.size());
    if (cap == 0)
      return std::unexpected(
          error{kind::format_mismatch, backend_id::none, 0, ""});
    if (out.size() < cap)
      return std::unexpected(
          error{kind::buffer_too_small, backend_id::none, 0, ""});

    auto squeezer = detail::zstd_backend::make_squeezer(
        f, detail::zstd_backend::level{static_cast<int>(lvl)});
    auto r = squeezer.push(in, out);
    if (!r)
      return std::unexpected(r.error());
    return squeeze_result{r->bytes_out};
#else
    (void)lvl;
    (void)in;
    (void)out;
    return std::unexpected(
        error{kind::format_mismatch, backend_id::none, 0, ""});
#endif
  }
  default:
    (void)lvl;
    (void)in;
    (void)out;
    return std::unexpected(
        error{kind::format_mismatch, backend_id::none, 0, ""});
  }
}

std::expected<squeeze_result, error>
swell(format f, std::span<const std::byte> in, std::span<std::byte> out) {
  switch (f) {
  case format::zlib:
  case format::gzip:
  case format::raw_deflate: {
#ifdef POLLIWOG_HAS_ZLIB
    auto sweller = detail::zlib_backend::make_sweller(f);
    auto r = sweller.push(in, out);
    if (!r)
      return std::unexpected(r.error());
    return squeeze_result{r->bytes_out};
#else
    (void)in;
    (void)out;
    return std::unexpected(
        error{kind::format_mismatch, backend_id::none, 0, ""});
#endif
  }
  case format::zstd: {
#ifdef POLLIWOG_HAS_ZSTD
    auto sweller = detail::zstd_backend::make_sweller(f);
    auto r = sweller.push(in, out);
    if (!r)
      return std::unexpected(r.error());
    return squeeze_result{r->bytes_out};
#else
    (void)in;
    (void)out;
    return std::unexpected(
        error{kind::format_mismatch, backend_id::none, 0, ""});
#endif
  }
  default:
    (void)in;
    (void)out;
    return std::unexpected(
        error{kind::format_mismatch, backend_id::none, 0, ""});
  }
}

} // namespace polliwog
