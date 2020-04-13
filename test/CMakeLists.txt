cmake_minimum_required(VERSION 3.8)

add_executable(UnitTest
  main.cpp
)

target_include_directories(UnitTest
  PRIVATE ../googletest/googletest/include
)

target_compile_definitions(UnitTest PRIVATE USING_SHARED)

if(MSVC)
  target_compile_options(UnitTest PRIVATE /W4)
else()
  target_compile_options(UnitTest PRIVATE
    -Wall
    -Wextra
    -pedantic-errors
    -Wshadow
    -Wold-style-cast
    $<$<C_COMPILER_ID:Clang>:-Wunreachable-code-return>
    $<$<C_COMPILER_ID:GNU>:-Wunreachable-code>
  )
endif()

target_link_libraries(UnitTest
  PRIVATE gtest_main
  PRIVATE ${BINARY_NAME}
)

include(GoogleTest)
gtest_discover_tests(UnitTest)
