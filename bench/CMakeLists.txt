# the benchmark tree. google benchmark is dev-only and never leaks
# into the installed target. pinned to v1.9.5 (versions.md,
# research-log row 11).

include(FetchContent)

FetchContent_Declare(benchmark
  SYSTEM
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.9.5
  GIT_SHALLOW TRUE)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(benchmark)

add_executable(polliwog-bench
  squeeze_bench.cpp)
target_link_libraries(polliwog-bench PRIVATE polliwog::polliwog benchmark::benchmark_main)
polliwog_target_warnings(polliwog-bench)
target_compile_definitions(polliwog-bench PRIVATE
  BENCH_POND_CORPUS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../test/pond/corpus/large")
