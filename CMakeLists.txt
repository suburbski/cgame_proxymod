cmake_minimum_required(VERSION 3.5)

project(cgame_proxymod
  VERSION 0.0.2.0 # <mayor>.<compat>.<minor>.<build>
  LANGUAGES C)

option(FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." FALSE)
if(${FORCE_COLORED_OUTPUT})
  if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options(-fdiagnostics-color=always)
  elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options(-fcolor-diagnostics)
  endif()
endif()

if(MSVC)
  add_compile_options(/std:c99)
  add_compile_options(/Wall)
else()
  add_compile_options(-std=c99)
  add_compile_options(-Wall -Wextra -pedantic-errors)
  add_compile_options(-Wdouble-promotion -Wstrict-prototypes -Wshadow)
endif()

option(BINARY_NAME "Name of the cgame_proxymod binary." "cgame")

add_subdirectory(src)
