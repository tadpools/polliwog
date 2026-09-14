# the test tree. catch2 is dev-only and never leaks into the installed
# target.

include(FetchContent)

FetchContent_Declare(catch2
  SYSTEM
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.16.0)
FetchContent_MakeAvailable(catch2)

add_executable(polliwog-tests
  unit/smoke.cpp
  unit/error_test.cpp
  unit/format_test.cpp
  unit/backend_test.cpp)
target_link_libraries(polliwog-tests PRIVATE polliwog::polliwog Catch2::Catch2WithMain)
polliwog_target_warnings(polliwog-tests)

if(POLLIWOG_WITH_ZLIB)
  target_sources(polliwog-tests PRIVATE
    unit/zlib_test.cpp
    unit/gzip_fixtures.cpp
    unit/stream_test.cpp
    unit/error_paths_test.cpp
    unit/alloc_test.cpp
    unit/pond_test.cpp)
  target_compile_definitions(polliwog-tests PRIVATE
    TEST_POND_CORPUS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/pond/corpus/large")
endif()

if(POLLIWOG_WITH_ZSTD)
  target_sources(polliwog-tests PRIVATE
    unit/zstd_test.cpp)
endif()

add_test(NAME smoke COMMAND polliwog-tests)
