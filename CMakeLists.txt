
cmake_minimum_required(VERSION 3.16) # Version that comes with Ubuntu 20.04.
project(square-mod-graph)

# See http://www.stablecoder.ca/2018/10/30/full-cmake-helper-suite.html

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_COMPILER "clang++-12")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

# clang-tidy
find_program (CLANG_TIDY_EXE NAMES "clang-tidy")
if(CLANG_TIDY_EXE)
  message(STATUS "clang-tidy found!")
  set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-format-style=file;-header-filter='${CMAKE_SOURCE_DIR}/*'" CACHE STRING "" FORCE)
else()
  message(STATUS "clang-tidy NOT found!")
  set(CMAKE_CXX_CLANG_TIDY "" CACHE STRING "" FORCE) # clear it
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -g")

add_subdirectory(gsl)

