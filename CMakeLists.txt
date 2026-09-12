cmake_minimum_required(VERSION 3.28)

project(polliwog
  VERSION 2026.2.0
  DESCRIPTION "one squeeze/swell interface over zlib, zstd, and lz4"
  HOMEPAGE_URL "https://github.com/tadpools/polliwog"
  LANGUAGES CXX)

option(POLLIWOG_WITH_ZLIB "zlib backend" ON)
option(POLLIWOG_WITH_ZLIB_NG "swap to zlib-ng, symbol-mangled" OFF)
option(POLLIWOG_WITH_ZSTD "zstd backend (2026.3.0)" OFF)
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

if(POLLIWOG_WITH_ZLIB)
  include(FetchContent)

  # zlib-ng in compat mode (ZLIB_COMPAT) exports the same targets as
  # real zlib; when the option is on, it replaces zlib entirely rather
  # than coexisting alongside it
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
  else()
    # prefer the system zlib when it exists and is not actually zlib-ng
    find_package(ZLIB QUIET)
    if(ZLIB_FOUND)
      check_include_file_cxx(zlib-ng.h ZLIBNG_HEADER)
      if(NOT ZLIBNG_HEADER)
        add_library(polliwog_backend STATIC
          src/detail/zlib.cpp
          src/squeeze.cpp
          src/stream.cpp)
        target_link_libraries(polliwog_backend PRIVATE ZLIB::ZLIB)
        target_link_libraries(polliwog_backend PUBLIC polliwog)
        target_include_directories(polliwog_backend PRIVATE
          ${CMAKE_CURRENT_SOURCE_DIR}/src)
        polliwog_target_warnings(polliwog_backend)
        set_target_properties(polliwog_backend PROPERTIES
          POSITION_INDEPENDENT_CODE ON)
        target_link_libraries(polliwog INTERFACE polliwog_backend)
        set(_POLLIWOG_ZLIB_BACKEND_BUILT TRUE)
      endif()
    endif()

    if(NOT _POLLIWOG_ZLIB_BACKEND_BUILT)
      set(ZLIB_BUILD_TESTING OFF CACHE BOOL "" FORCE)
      set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
      FetchContent_Declare(zlib
        SYSTEM
        GIT_REPOSITORY https://github.com/madler/zlib.git
        GIT_TAG v1.3.2
        GIT_SHALLOW TRUE)
      FetchContent_MakeAvailable(zlib)
    endif()
  endif()

  if(NOT _POLLIWOG_ZLIB_BACKEND_BUILT)
    add_library(polliwog_backend STATIC
      src/detail/zlib.cpp
      src/squeeze.cpp
      src/stream.cpp)
    target_link_libraries(polliwog_backend PRIVATE zlibstatic)
    target_link_libraries(polliwog_backend PUBLIC polliwog)
    target_include_directories(polliwog_backend PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/src)
    polliwog_target_warnings(polliwog_backend)
    set_target_properties(polliwog_backend PROPERTIES
      POSITION_INDEPENDENT_CODE ON)
    target_link_libraries(polliwog INTERFACE polliwog_backend)
  endif()

  if(MSVC)
    target_compile_options(polliwog_backend PRIVATE /external:W0)
  endif()
endif()

enable_testing()
add_subdirectory(test)
