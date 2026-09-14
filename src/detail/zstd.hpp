// detail/zstd.hpp - raii handles over ZSTD_CCtx / ZSTD_DCtx.
// Stability: internal (detail).
//
// wraps the zstd stable API only (v1.5.7, versions.md). no
// experimental or static-linking-only functions. the handles are
// move-only raii types; the destructor calls the corresponding free
// function. single-threaded by contract (one handle, one thread).
//
// see also: detail/backend.hpp, polliwog/format.hpp

#pragma once

#include "detail/backend.hpp"

#include <cstddef>
#include <cstdint>
#include <string_view>

struct ZSTD_CCtx_s;
struct ZSTD_DCtx_s;

namespace polliwog::detail {

// compression handle: owns a ZSTD_CCtx via ZSTD_freeCCtx
class zstd_squeezer {
public:
  zstd_squeezer(format f, zstd_level lvl);
  zstd_squeezer(zstd_squeezer &&o) noexcept;
  zstd_squeezer &operator=(zstd_squeezer &&o) noexcept;
  ~zstd_squeezer();

  zstd_squeezer(const zstd_squeezer &) = delete;
  zstd_squeezer &operator=(const zstd_squeezer &) = delete;

  // compress more input without finishing the stream (streaming use)
  result push_more(std::span<const std::byte> in, std::span<std::byte> out);

  // compress and finish (one-shot use)
  result push(std::span<const std::byte> in, std::span<std::byte> out);

  result flush(std::span<std::byte> out);
  result finish(std::span<std::byte> out);

private:
  ZSTD_CCtx_s *ctx_{nullptr};
  format fmt_;
  int level_{3}; // ZSTD_CLEVEL_DEFAULT; stored for one-shot ZSTD_compress
};

// decompression handle: owns a ZSTD_DCtx via ZSTD_freeDCtx
class zstd_sweller {
public:
  explicit zstd_sweller(format f);
  zstd_sweller(zstd_sweller &&o) noexcept;
  zstd_sweller &operator=(zstd_sweller &&o) noexcept;
  ~zstd_sweller();

  zstd_sweller(const zstd_sweller &) = delete;
  zstd_sweller &operator=(const zstd_sweller &) = delete;

  // decompress more input without finishing (streaming use)
  result push_more(std::span<const std::byte> in, std::span<std::byte> out);

  // decompress and finish (one-shot use)
  result push(std::span<const std::byte> in, std::span<std::byte> out);

private:
  ZSTD_DCtx_s *ctx_{nullptr};
  format fmt_;
};

// the backend type: stateless, carries the types and factories
struct zstd_backend {
  static constexpr backend_id id{backend_id::zstd};

  static std::span<const format> formats();

  using level = zstd_level;
  using handle = zstd_squeezer;

  static handle make_squeezer(format f, level lvl);
  static zstd_sweller make_sweller(format f);
  static std::size_t bound(level lvl, std::uint64_t in_size);

  static result push(handle &h, std::span<const std::byte> in,
                     std::span<std::byte> out);
  static result flush(handle &h, std::span<std::byte> out);
  static result finish(handle &h, std::span<std::byte> out);
};

} // namespace polliwog::detail
