cmake_minimum_required(VERSION 3.13)

project(cgame_proxymod
  VERSION 1.4.0 # <mayor>.<minor>.<patch>
)

set(BINARY_NAME cgame CACHE STRING "Name of the cgame_proxymod binary")

# set cgame version:
#   release     X.X.X
#   pre-release X.X.X+HEAD_SHA1
#   others      0.0.0 or HEAD_SHA1
find_package(Git QUIET)
if(GIT_FOUND)
  # re-configure if the Git index file is touched
  set_property(DIRECTORY APPEND
    PROPERTY CMAKE_CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/.git/index)

  execute_process(COMMAND
    ${GIT_EXECUTABLE} rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE HEAD_SHA1
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  set(TAG_NAME v${PROJECT_VERSION})
  execute_process(COMMAND
    ${GIT_EXECUTABLE} rev-list --abbrev-commit -n 1 tags/${TAG_NAME}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE TAG_SHA1
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  if(NOT TAG_SHA1)
    message(WARNING "No annotated tag ${TAG_NAME} found.")
    set(CGAME_VERSION ${HEAD_SHA1})
  else()
    execute_process(COMMAND
      ${GIT_EXECUTABLE} merge-base --is-ancestor ${TAG_SHA1} ${HEAD_SHA1}
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      RESULT_VARIABLE ANCESTOR_EXIT_CODE
      ERROR_QUIET)

    if(NOT ANCESTOR_EXIT_CODE EQUAL "0")
      message(WARNING "Annotated tag ${TAG_NAME} [${TAG_SHA1}] is not an ancestor of HEAD [${HEAD_SHA1}].")
      set(CGAME_VERSION ${HEAD_SHA1})
    elseif(NOT HEAD_SHA1 STREQUAL TAG_SHA1) 
      string(CONCAT CGAME_VERSION ${PROJECT_VERSION}+${HEAD_SHA1}) # pre-release
    else() 
      set(CGAME_VERSION ${PROJECT_VERSION}) # release
    endif()
  endif()
else()
  set(CGAME_VERSION "0.0.0")
endif()

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

option(ENABLE_TESTING "Enable the tests" ON)
if(ENABLE_TESTING)
  enable_testing()
  set(CMAKE_CXX_STANDARD 17)           # set C++ standard for googletest
  set(INSTALL_GTEST OFF CACHE BOOL "") # disable installation of googletest
  add_subdirectory(googletest)
  add_subdirectory(test)
endif()
