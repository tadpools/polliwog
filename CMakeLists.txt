cmake_minimum_required(VERSION 3.28)

project(polliwog
  VERSION 2026.3.0
  DESCRIPTION "one squeeze/swell interface over zlib, zstd, and lz4"
  HOMEPAGE_URL "https://github.com/tadpools/polliwog"
  LANGUAGES CXX)

option(POLLIWOG_WITH_ZLIB "zlib backend" ON)
option(POLLIWOG_WITH_ZLIB_NG "swap to zlib-ng, symbol-mangled" OFF)
option(POLLIWOG_WITH_ZSTD "zstd backend" OFF)
option(POLLIWOG_WITH_LZ4 "lz4 backend (2026.4.0)" OFF)
option(POLLIWOG_WARNINGS "treat warnings on our code as errors" ON)
option(POLLIWOG_SANITIZERS "asan + ubsan on our targets" OFF)
option(POLLIWOG_CXX26 "opt-in c++26 experiments; never required" OFF)

add_library(polliwog INTERFACE)
add_library(polliwog::polliwog ALIAS polliwog)

target_compile_features(polliwog INTERFACE cxx_std_23)

target_include_directories(polliwog INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

function(polliwog_target_warnings tgt)
  if(POLLIWOG_WARNINGS)
    if(MSVC)
      target_compile_options(${tgt} PRIVATE /W4 /permissive- /we4062 /WX)
    else()
      target_compile_options(${tgt} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
  endif()
endfunction()

include(CheckIncludeFileCXX)
include(FetchContent)

# -- collect backend sources and link targets ------------------------------

set(_POLLIWOG_BACKEND_SRCS
  src/squeeze.cpp
  src/stream.cpp)
set(_POLLIWOG_BACKEND_LIBS "")
set(_POLLIWOG_BACKEND_INCS ${CMAKE_CURRENT_SOURCE_DIR}/src)

# -- zlib backend ----------------------------------------------------------

if(POLLIWOG_WITH_ZLIB)
  list(APPEND _POLLIWOG_BACKEND_SRCS src/detail/zlib.cpp)

  if(POLLIWOG_WITH_ZLIB_NG)
    FetchContent_Declare(zlib-ng
      SYSTEM
      GIT_REPOSITORY https://github.com/zlib-ng/zlib-ng.git
      GIT_TAG 2.3.3
      GIT_SHALLOW TRUE)
    set(ZLIB_COMPAT ON CACHE BOOL "" FORCE)
    set(ZLIB_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
    set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(zlib-ng)
    list(APPEND _POLLIWOG_BACKEND_LIBS zlib-ng)
  else()
    find_package(ZLIB QUIET)
    if(ZLIB_FOUND)
      check_include_file_cxx(zlib-ng.h ZLIBNG_HEADER)
      if(NOT ZLIBNG_HEADER)
        list(APPEND _POLLIWOG_BACKEND_LIBS ZLIB::ZLIB)
        set(_POLLIWOG_ZLIB_FOUND TRUE)
      endif()
    endif()

    if(NOT _POLLIWOG_ZLIB_FOUND)
      set(ZLIB_BUILD_TESTING OFF CACHE BOOL "" FORCE)
      set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
      FetchContent_Declare(zlib
        SYSTEM
        GIT_REPOSITORY https://github.com/madler/zlib.git
        GIT_TAG v1.3.2
        GIT_SHALLOW TRUE)
      FetchContent_MakeAvailable(zlib)
      list(APPEND _POLLIWOG_BACKEND_LIBS zlibstatic)
    endif()
  endif()
endif()

# -- zstd backend ----------------------------------------------------------

if(POLLIWOG_WITH_ZSTD)
  list(APPEND _POLLIWOG_BACKEND_SRCS src/detail/zstd.cpp)

  FetchContent_Declare(zstd
    SOURCE_SUBDIR build/cmake
    SYSTEM
    GIT_REPOSITORY https://github.com/facebook/zstd.git
    GIT_TAG v1.5.7
    GIT_SHALLOW TRUE)
  set(ZSTD_BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
  set(ZSTD_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(ZSTD_BUILD_SHARED OFF CACHE BOOL "" FORCE)
  set(ZSTD_BUILD_STATIC ON CACHE BOOL "" FORCE)
  set(ZSTD_BUILD_LIBCXX OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(zstd)

  # the zstd cmake target is libzstd_static; ensure its include
  # directory is available to our sources
  if(TARGET libzstd_static)
    list(APPEND _POLLIWOG_BACKEND_LIBS libzstd_static)
  else()
    # fallback: link the zstd static library directly
    list(APPEND _POLLIWOG_BACKEND_LIBS libzstd)
  endif()
  list(APPEND _POLLIWOG_BACKEND_INCS ${zstd_SOURCE_DIR}/lib)
endif()

# -- build the backend static library -------------------------------------

if(_POLLIWOG_BACKEND_SRCS)
  add_library(polliwog_backend STATIC ${_POLLIWOG_BACKEND_SRCS})
  target_link_libraries(polliwog_backend PRIVATE ${_POLLIWOG_BACKEND_LIBS})
  target_link_libraries(polliwog_backend PUBLIC polliwog)
  target_include_directories(polliwog_backend PRIVATE ${_POLLIWOG_BACKEND_INCS})
  polliwog_target_warnings(polliwog_backend)
  set_target_properties(polliwog_backend PROPERTIES
    POSITION_INDEPENDENT_CODE ON)
  target_link_libraries(polliwog INTERFACE polliwog_backend)

  if(POLLIWOG_WITH_ZLIB)
    target_compile_definitions(polliwog_backend PUBLIC POLLIWOG_HAS_ZLIB)
  endif()
  if(POLLIWOG_WITH_ZSTD)
    target_compile_definitions(polliwog_backend PUBLIC POLLIWOG_HAS_ZSTD)
  endif()

  if(MSVC)
    target_compile_options(polliwog_backend PRIVATE /external:W0)
  endif()
endif()

# -- benchmarks (dev-only) ------------------------------------------------

option(POLLIWOG_BENCH "build benchmark targets (dev-only)" OFF)

enable_testing()
add_subdirectory(test)

if(POLLIWOG_BENCH)
  add_subdirectory(bench)
endif()

# -- fuzz targets (dev-only, clang required) -------------------------------

option(POLLIWOG_FUZZ "build libfuzzer targets (dev-only)" OFF)

if(POLLIWOG_FUZZ)
  add_executable(fuzz_squeeze fuzz/fuzz_squeeze.cpp)
  target_link_libraries(fuzz_squeeze PRIVATE polliwog_backend)
  target_compile_options(fuzz_squeeze PRIVATE -fsanitize=fuzzer,address,undefined
    -fno-omit-frame-pointer)
  target_link_options(fuzz_squeeze PRIVATE -fsanitize=fuzzer,address,undefined)

  add_executable(fuzz_stream fuzz/fuzz_stream.cpp)
  target_link_libraries(fuzz_stream PRIVATE polliwog_backend)
  target_compile_options(fuzz_stream PRIVATE -fsanitize=fuzzer,address,undefined
    -fno-omit-frame-pointer)
  target_link_options(fuzz_stream PRIVATE -fsanitize=fuzzer,address,undefined)
endif()
