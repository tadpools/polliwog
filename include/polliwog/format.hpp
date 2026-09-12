// polliwog/format.hpp - the wire formats and the typed levels.
// Stability: Stable (hatch).
//
// formats are named for the wire container, never for the backend
// brand alone: a gzip stream is gzip, produced by the zlib backend.
// output is the backend's wire format, byte for byte - there is no
// polliwog container.
//
// who reaches for this header: anyone calling squeeze/swell or
// building a streaming pipeline, since the format selects the code
// path. who almost never does: callers who hold results and only
// read errors.
//
// the enum lists every wire format the library's roadmap serves;
// a format whose backend is not compiled in fails at the call with
// format_mismatch rather than pretending the option exists. levels
// are typed per backend so a zlib level can never reach a zstd
// stream.

#pragma once

#include <string_view>

namespace polliwog {

enum class format {
  zlib,        // rfc 1950
  gzip,        // rfc 1952
  raw_deflate, // rfc 1951, no container
  zstd,        // the zstd frame format (tadpole milestone)
  lz4_frame,   // the lz4 frame format (froglet milestone)
};

// value to human text; exhaustive over format with no default case,
// same rule as kind_name in error.hpp
std::string_view format_name(format f);

// zlib's compression levels. the numbers are pinned, not inherited:
// zlib.h (zlib 1.3.2) documents Z_BEST_SPEED as 1,
// Z_BEST_COMPRESSION as 9, and Z_DEFAULT_COMPRESSION as -1,
// "currently equivalent to level 6". polliwog pins 6 explicitly so
// the determinism contract does not depend on how the backend
// resolves its own default today.
//
// the named anchors are the blessed surface. the other zlib levels
// (0 through 9) stay reachable by explicit init, zlib_level{4};
// a value outside the backend's range fails with level_unsupported.
enum class zlib_level : int {
  fastest = 1,       // Z_BEST_SPEED
  default_level = 6, // Z_DEFAULT_COMPRESSION's documented equivalent
  best = 9,          // Z_BEST_COMPRESSION
};

inline std::string_view format_name(format f) {
  switch (f) {
  case format::zlib:
    return "zlib";
  case format::gzip:
    return "gzip";
  case format::raw_deflate:
    return "raw_deflate";
  case format::zstd:
    return "zstd";
  case format::lz4_frame:
    return "lz4_frame";
  }
  return "";
}

} // namespace polliwog
