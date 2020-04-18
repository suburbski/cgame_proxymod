cmake_minimum_required(VERSION 3.8)

project(cgame_proxymod
  VERSION 1.3.0 # <mayor>.<minor>.<patch>
)

option(BINARY_NAME "Name of the cgame_proxymod binary." "cgame")

configure_file(version.h.in version.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

option(FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." FALSE)
if(${FORCE_COLORED_OUTPUT})
  if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR
     CMAKE_CXX_COMPILE_ID STREQUAL "GNU")
    add_compile_options(-fdiagnostics-color=always)
  elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang" OR
         CMAKE_CXX_COMPILE_ID STREQUAL "Clang")
    add_compile_options(-fcolor-diagnostics)
  endif()
endif()

add_subdirectory(src)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(-std=c++17)

  enable_testing()
  add_subdirectory(googletest)
  add_subdirectory(test)
endif()
