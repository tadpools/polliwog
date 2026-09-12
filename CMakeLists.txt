cmake_minimum_required(VERSION 3.28)

project(polliwog
  VERSION 2026.1.0
  DESCRIPTION "compression truth for c++: one squeeze/swell surface over zlib, zstd, and lz4"
  HOMEPAGE_URL "https://github.com/tadpools/polliwog"
  LANGUAGES CXX)

# the option set documented in the readme; this file and that table
# move together
option(POLLIWOG_WITH_ZLIB "zlib backend; the only one for now" ON)
option(POLLIWOG_WITH_ZLIB_NG "swap the zlib shim for zlib-ng, symbol-mangled" OFF)
option(POLLIWOG_WITH_ZSTD "zstd backend (tadpole milestone)" OFF)
option(POLLIWOG_WITH_LZ4 "lz4 backend (froglet milestone)" OFF)
option(POLLIWOG_WARNINGS "treat warnings on our code as errors" ON)
option(POLLIWOG_SANITIZERS "asan + ubsan on our targets" OFF)
option(POLLIWOG_CXX26 "opt-in c++26 experiments; never required" OFF)

# spawn: the target is an empty interface. it grows sources at the
# hatch milestone and stays the one entry point throughout.
add_library(polliwog INTERFACE)
add_library(polliwog::polliwog ALIAS polliwog)

target_compile_features(polliwog INTERFACE cxx_std_23)

target_include_directories(polliwog INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

# the warning gate applies to our code only; consumers never inherit
# it. every target of ours calls this once.
function(polliwog_target_warnings tgt)
  if(POLLIWOG_WARNINGS)
    if(MSVC)
      # /we4062 makes a switch missing an enumerator a hard error, so
      # the exhaustive kind_name/format_name switches are compiler
      # enforced on msvc the way -Wswitch does it for gcc/clang
      target_compile_options(${tgt} PRIVATE /W4 /permissive- /we4062 /WX)
    else()
      target_compile_options(${tgt} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
  endif()
endfunction()

enable_testing()
add_subdirectory(test)
