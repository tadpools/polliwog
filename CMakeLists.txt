cmake_minimum_required(VERSION 3.28)

project(polliwog
  VERSION 2026.2.0
  DESCRIPTION "one squeeze/swell interface over zlib, zstd, and lz4"
  HOMEPAGE_URL "https://github.com/tadpools/polliwog"
  LANGUAGES CXX)

option(POLLIWOG_WITH_ZLIB "zlib backend" ON)
option(POLLIWOG_WITH_ZLIB_NG "swap to zlib-ng, symbol-mangled" OFF)
option(POLLIWOG_WITH_ZSTD "zstd backend (tadpole milestone)" OFF)
option(POLLIWOG_WITH_LZ4 "lz4 backend (froglet milestone)" OFF)
option(POLLIWOG_WARNINGS "treat warnings on our code as errors" ON)
option(POLLIWOG_SANITIZERS "asan + ubsan on our targets" OFF)
option(POLLIWOG_CXX26 "opt-in c++26 experiments; never required" OFF)

add_library(polliwog INTERFACE)
add_library(polliwog::polliwog ALIAS polliwog)

target_compile_features(polliwog INTERFACE cxx_std_23)

target_include_directories(polliwog INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

# the warning gate: our code only, consumers never inherit it
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

if(POLLIWOG_WITH_ZLIB)
  include(FetchContent)

  # prefer the system zlib when it exists and is not actually zlib-ng
  # (zlib-ng defines ZLIBNG_VERSION; the real zlib does not)
  find_package(ZLIB QUIET)
  if(ZLIB_FOUND)
    check_include_file_cxx(zlib-ng.h ZLIBNG_HEADER)
    if(NOT ZLIBNG_HEADER AND NOT POLLIWOG_WITH_ZLIB_NG)
      add_library(polliwog_backend STATIC
        src/detail/zlib.cpp
        src/squeeze.cpp)
      target_link_libraries(polliwog_backend PRIVATE ZLIB::ZLIB)
      target_link_libraries(polliwog_backend PUBLIC polliwog)
      target_include_directories(polliwog_backend PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src)
      polliwog_target_warnings(polliwog_backend)
      set_target_properties(polliwog_backend PROPERTIES
        POSITION_INDEPENDENT_CODE ON)
      target_link_libraries(polliwog INTERFACE polliwog_backend)
      set(_POLLIWOG_ZLIB_FOUND TRUE)
    endif()
  endif()

  if(NOT _POLLIWOG_ZLIB_FOUND)
    set(ZLIB_BUILD_TESTING OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(zlib
      SYSTEM
      GIT_REPOSITORY https://github.com/madler/zlib.git
      GIT_TAG v1.3.2
      GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(zlib)

    add_library(polliwog_backend STATIC
      src/detail/zlib.cpp
      src/squeeze.cpp)
    target_link_libraries(polliwog_backend PRIVATE zlibstatic)
    target_link_libraries(polliwog_backend PUBLIC polliwog)
    target_include_directories(polliwog_backend PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/src
      ${zlib_SOURCE_DIR}
      ${zlib_BINARY_DIR})
    polliwog_target_warnings(polliwog_backend)
    set_target_properties(polliwog_backend PROPERTIES
      POSITION_INDEPENDENT_CODE ON)
    target_link_libraries(polliwog INTERFACE polliwog_backend)
  endif()

  if(MSVC)
    # /external:W0 suppresses warnings from the system zlib headers
    # under the warning gate; fetchcontent's SYSTEM keyword and msvc
    # disagree sometimes
    target_compile_options(polliwog_backend PRIVATE /external:W0)
  endif()
endif()

if(POLLIWOG_WITH_ZLIB_NG AND NOT _POLLIWOG_ZLIB_FOUND)
  include(FetchContent)
  FetchContent_Declare(zlib-ng
    SYSTEM
    GIT_REPOSITORY https://github.com/zlib-ng/zlib-ng.git
    GIT_TAG 2.3.3
    GIT_SHALLOW TRUE)
  set(ZLIB_COMPAT ON CACHE BOOL "" FORCE)
  set(ZLIB_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(zlib-ng)
  # when zlib-ng compat mode is on it exports as zlib; handled above
endif()

enable_testing()
add_subdirectory(test)
