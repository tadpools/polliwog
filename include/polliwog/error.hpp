// polliwog/error.hpp - the one error taxonomy. errors are values;
// nothing in the public api throws.
// Stability: Stable .
//
// taxonomy frozen as of 2026.3.0 (tadpole). new kinds are additive
// only; removing or renaming a kind is a milestone event, never a
// patch. the list below is exhaustive and the switch in kind_name
// enforces it at compile time.
//
// every fallible call in polliwog returns std::expected<T, error>.
// callers switch on kind, never parse strings; the note carries the
// backend's message verbatim when one is forwarded, and lowercase
// plain text when it is ours.
//
// who reaches for this header: anyone holding an expected from any
// polliwog call. who almost never does: code that only squeezes
// into a big-enough buffer and checks the bool first.
//
// failure modes, by kind:
//   stream_corrupt     bad bytes: bad header, bad checksum,
//                      truncation. pos names where the stream died.
//   buffer_too_small   the caller's output span. check the bound
//                      before retrying; nothing was written past it.
//   level_unsupported  a level outside the backend's range.
//   format_mismatch    swelling zlib bytes through a zstd sweller.
//   backend_failure    the backend returned an error this library
//                      forwards honestly; its code text is in note.
//   out_of_memory      an allocation inside the backend failed.
//   internal_contract_violation  our bug, never a bad input. the
//                      note asks for a report; please file one.

#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace polliwog {

enum class kind {
  stream_corrupt,
  buffer_too_small,
  level_unsupported,
  format_mismatch,
  backend_failure,
  out_of_memory,
  internal_contract_violation,
};

// value to human text; exhaustive over kind with no default case, so
// the compiler flags any missed enumerator (gcc/clang via -Wswitch,
// msvc via /we4062 in the project's warning gate)
std::string_view kind_name(kind k);

// which backend produced an error, or none when the library itself
// is the speaker
enum class backend_id { none, zlib, zlib_ng, zstd, lz4 };

// exhaustive over backend_id, same rule as kind_name
std::string_view backend_id_name(backend_id b);

struct error {
  kind k{};
  backend_id b{backend_id::none};
  std::uint64_t pos{0}; // byte position where the stream failed, 0
                        // when meaningless (architecture.md)
  std::string note;
};

inline std::string_view kind_name(kind k) {
  switch (k) {
  case kind::stream_corrupt:
    return "stream_corrupt";
  case kind::buffer_too_small:
    return "buffer_too_small";
  case kind::level_unsupported:
    return "level_unsupported";
  case kind::format_mismatch:
    return "format_mismatch";
  case kind::backend_failure:
    return "backend_failure";
  case kind::out_of_memory:
    return "out_of_memory";
  case kind::internal_contract_violation:
    return "internal_contract_violation";
  }
  // unreachable while the switch stays exhaustive; the missing-case
  // warning is the enforcement, this line only satisfies control
  // flow
  return "";
}

inline std::string_view backend_id_name(backend_id b) {
  switch (b) {
  case backend_id::none:
    return "none";
  case backend_id::zlib:
    return "zlib";
  case backend_id::zlib_ng:
    return "zlib_ng";
  case backend_id::zstd:
    return "zstd";
  case backend_id::lz4:
    return "lz4";
  }
  return "";
}

} // namespace polliwog
