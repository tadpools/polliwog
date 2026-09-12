// detail/backend.hpp - the Backend concept, the whole integration
// surface for a compression engine.
// Stability: internal (detail). reachable, honest, moving faster
// than the public headers.
//
// a backend is a type, not an object: stateless, selected at compile
// time by the format enum, checked by this concept at build time. a
// new backend touches nothing above it (architecture.md).
//
// the contract, field by field:
//
//   id        which backend_id names this engine in error values
//   formats() the wire containers this engine serves; a format the
//             engine does not list must never be dispatched to it
//   level     the typed level type (zlib_level, zstd_level, ...)
//   handle    move-only raii state; the destructor is the only
//             cleanup, and the c api never leaks past this type.
//             single-threaded by contract: one handle, one thread
//             at a time, no internal locking
//   make_squeezer(format, level)
//             a handle ready to compress that container at that level
//   make_sweller(format)  a handle ready to decompress that container
//   push(handle, in, out) compress or decompress as much of in as
//             out can hold; may consume less than all of in.
//             progress reports what was consumed and produced. no
//             hidden buffering, ever
//   flush(handle, out)    sync_flush only (decision D9) until a
//             proven need lands with its own fixtures
//   finish(handle, out)   ends the stream and emits the trailer
//   bound(level, in_size) an honest upper bound on the squeezed
//             size, or max() when the backend cannot bound; the
//             one-shot layer documents its growth policy for max()
//
// every fallible expression returns std::expected<progress, error>
// and never throws; errors carry this backend's id.
//
// see also: polliwog/error.hpp, polliwog/format.hpp

#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <type_traits>
#include <utility>

#include "polliwog/error.hpp"
#include "polliwog/format.hpp"

namespace polliwog::detail {

struct progress {
  std::uint64_t bytes_in{0};
  std::uint64_t bytes_out{0};
  bool done{false};
};

// the expected return type of every fallible backend expression
using result = std::expected<progress, error>;

template <typename B>
concept backend =
    std::movable<typename B::handle> && !std::copyable<typename B::handle> &&
    requires {
      { B::id } -> std::convertible_to<const backend_id>;
      { B::formats() } -> std::convertible_to<std::span<const format>>;
    } &&
    requires(typename B::level lvl, std::uint64_t in_size) {
      {
        B::make_squeezer(format{}, lvl)
      } -> std::convertible_to<typename B::handle>;
      { B::make_sweller(format{}) } -> std::convertible_to<typename B::handle>;
      { B::bound(lvl, in_size) } -> std::convertible_to<std::size_t>;
    } &&
    requires(typename B::handle &h, std::span<const std::byte> in,
             std::span<std::byte> out) {
      { B::push(h, in, out) } -> std::same_as<result>;
      { B::flush(h, out) } -> std::same_as<result>;
      { B::finish(h, out) } -> std::same_as<result>;
    };

} // namespace polliwog::detail
