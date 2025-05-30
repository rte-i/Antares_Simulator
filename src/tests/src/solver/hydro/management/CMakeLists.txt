include(${CMAKE_SOURCE_DIR}/tests/macros.cmake)

# Hydro input checker
add_boost_test(test-hydro-inputs-checker
  SRC test-hydro-inputs-checker.cpp
  LIBS
  Antares::study
  antares-solver-hydro
  test_utils_unit)

