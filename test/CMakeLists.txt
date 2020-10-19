cmake_minimum_required(VERSION 3.13)

add_executable(UnitTest
  main.cpp
)

target_compile_features(UnitTest PUBLIC cxx_std_17)

if(MSVC)
  target_compile_options(UnitTest PRIVATE
    /WX
    /W4
    /wd4996
  )
  target_link_options(UnitTest PRIVATE /WX)
else()
  target_compile_options(UnitTest PRIVATE
    -Werror
    -Wall
    -Wextra
    -pedantic-errors
    -Wshadow

    $<$<COMPILE_LANGUAGE:C>:-Wmissing-prototypes>
    $<$<COMPILE_LANGUAGE:C>:-Wstrict-prototypes>
    $<$<C_COMPILER_ID:Clang>:-Wunreachable-code-return>
    $<$<C_COMPILER_ID:GNU>:-Wunreachable-code>

    $<$<COMPILE_LANGUAGE:CXX>:-Wold-style-cast>
    $<$<CXX_COMPILER_ID:Clang>:-Wunreachable-code-return>
    $<$<CXX_COMPILER_ID:GNU>:-Wunreachable-code>
  )
endif()

target_link_libraries(UnitTest
  PRIVATE cgame_obj
  PRIVATE gtest_main
)

include(GoogleTest)
gtest_discover_tests(UnitTest)
