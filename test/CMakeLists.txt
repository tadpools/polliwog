# the test tree. catch2 is dev-only and never leaks into the installed
# target (philosophy.md, belief 5).

include(FetchContent)

# pinned to v3.16.0, verified against the upstream release. SYSTEM
# keeps catch2 headers outside our warning gate - warnings from
# third-party headers are upstream's problem, the same rule the c
# backends get.
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

add_test(NAME smoke COMMAND polliwog-tests)
