cmake_minimum_required(VERSION 3.13)

project(cgame_proxymod
  VERSION 1.3.1 # <mayor>.<minor>.<patch>
)

option(BINARY_NAME "Name of the cgame_proxymod binary." "cgame")

configure_file(version.h.in version.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR
   CMAKE_CXX_COMPILE_ID STREQUAL "GNU")
  add_compile_options(-fdiagnostics-color=always)
elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang" OR
       CMAKE_CXX_COMPILE_ID STREQUAL "Clang")
  add_compile_options(-fcolor-diagnostics)
endif()

add_subdirectory(src)

enable_testing()
set(CMAKE_CXX_STANDARD 17)           # set C++ standard for googletest
set(INSTALL_GTEST OFF CACHE BOOL "") # disable installation of googletest
add_subdirectory(googletest)
add_subdirectory(test)
