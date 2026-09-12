// detail/zlib.hpp - raii handles over z_stream, the _z entry points.
// Stability: internal.

#pragma once

#include "detail/backend.hpp"

#include <memory>
#include <span>
#include <string_view>

struct z_stream_s;

namespace polliwog::detail {

// window bits per container: zlib needs a header (15), gzip carries
// its own mtime=0 header (15+16), raw deflate has none (-15). the
// choice is made at handle construction so the rest of the code never
// thinks about it again.
int window_bits_for(format f);

// compression handle: move-only, owns its z_stream via deflateEnd
class zlib_squeezer {
public:
  zlib_squeezer(format f, zlib_level lvl);
  zlib_squeezer(zlib_squeezer &&o) noexcept;
  zlib_squeezer &operator=(zlib_squeezer &&o) noexcept;
  ~zlib_squeezer();

  zlib_squeezer(const zlib_squeezer &) = delete;
  zlib_squeezer &operator=(const zlib_squeezer &) = delete;

  result push(std::span<const std::byte> in, std::span<std::byte> out);
  result flush(std::span<std::byte> out);
  result finish(std::span<std::byte> out);

private:
  std::unique_ptr<z_stream_s> stream_;
};

// decompression handle: move-only, owns its z_stream via inflateEnd
class zlib_sweller {
public:
  explicit zlib_sweller(format f);
  zlib_sweller(zlib_sweller &&o) noexcept;
  zlib_sweller &operator=(zlib_sweller &&o) noexcept;
  ~zlib_sweller();

  zlib_sweller(const zlib_sweller &) = delete;
  zlib_sweller &operator=(const zlib_sweller &) = delete;

  result push(std::span<const std::byte> in, std::span<std::byte> out);

private:
  std::unique_ptr<z_stream_s> stream_;
};

// the backend type: stateless, carries the types and factories
struct zlib_backend {
  static constexpr backend_id id{backend_id::zlib};

  static std::span<const format> formats();

  using level = zlib_level;
  using handle = zlib_squeezer;

  static handle make_squeezer(format f, level lvl);
  static zlib_sweller make_sweller(format f);
  static std::size_t bound(level lvl, std::uint64_t in_size);

  static result push(handle &h, std::span<const std::byte> in,
                     std::span<std::byte> out);
  static result flush(handle &h, std::span<std::byte> out);
  static result finish(handle &h, std::span<std::byte> out);
};

} // namespace polliwog::detail
