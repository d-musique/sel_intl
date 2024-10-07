# The SEL extension library
# Free software published under the MIT license.

cmake_minimum_required(VERSION 3.24)

project(sel_intl LANGUAGES CXX)
option(SEL_INTL_TESTS "Build unit tests for this project" OFF)

set(CMAKE_CXX_STANDARD 17)

add_library(sel_intl STATIC)
target_include_directories(sel_intl PUBLIC "include" PRIVATE "source")
target_sources(sel_intl PRIVATE
  "source/sel/intl.cpp"
  "source/sel/intl_catalog.cpp"
  "source/sel/intl_plural_expr.cpp")
if(WIN32)
  target_sources(sel_intl PRIVATE
    "source/sel/intl_win32.cpp")
endif()
add_library(sel::intl ALIAS sel_intl)

include(CTest)
if(BUILD_TESTING AND SEL_INTL_TESTS)
  include("cmake/get_doctest.cmake")
  get_doctest()

  add_executable(sel_intl_tests "test/intl_tests.cpp" "test/main.cpp")
  target_include_directories(sel_intl_tests PRIVATE "source")
  target_compile_definitions(sel_intl_tests PRIVATE "DOCTEST_CONFIG_USE_STD_HEADERS=1")
  target_compile_definitions(sel_intl_tests PRIVATE "DOCTEST_CONFIG_SUPER_FAST_ASSERTS=1")
  target_compile_definitions(sel_intl_tests PRIVATE "SEL_TEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/test\"")
  target_link_libraries(sel_intl_tests PRIVATE sel_intl doctest::doctest)

  include(doctest)
  doctest_discover_tests(sel_intl_tests)
endif()
