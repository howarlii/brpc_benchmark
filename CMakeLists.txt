cmake_minimum_required(VERSION 3.20.0)

#! https://gitlab.kitware.com/cmake/community/wikis/FAQ#how-do-i-use-a-different-compiler
# must be set before project
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_C_COMPILER clang)

project(
  Brpc_benchmark
  VERSION 0.0.1
  LANGUAGES CXX
)

# ----------------------------------------------------------------------
# CMake modules
include(CMakeDependentOption)
include(CTest) # adds BUILD_TESTING option, include after project() but before add_subdirectory()

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
include(utils)

# ----------------------------------------------------------------------
# Cached options

option(ENABLE_CCACHE "Enable ccache for compilation" ON)
option(DEBUG_WITH_ASAN "Enable address sanitizer in debug compile mode" ON)

# ----------------------------------------------------------------------
# Setup global compile options
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # For clang-tidy
set(CMAKE_FIND_PACKAGE_RESOLVE_SYMLINKS TRUE)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
# ~~~
# -Wno-mismatched-tags for velox mismatching class & struct declaration
# -fsanitize=address to enable Address Sanitizer for debug

set(CMAKE_CXX_FLAGS_DEBUG
    "-g -Wall -Wno-mismatched-tags -Wno-deprecated-declarations -Wno-deprecated-this-capture -Werror"
)
if(DEBUG_WITH_ASAN)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
endif()

# mode
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -march=native -DNDEBUG")

# ----------------------------------------------------------------------
# Use ccache
if(ENABLE_CCACHE
   AND NOT CMAKE_C_COMPILER_LAUNCHER
   AND NOT CMAKE_CXX_COMPILER_LAUNCHER
)
  find_program(CCACHE_FOUND ccache)
  if(CCACHE_FOUND)
    message(STATUS "Using ccache: ${CCACHE_FOUND}")
    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_FOUND})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_FOUND})
    # keep comments as they might matter to the compiler
    set(ENV{CCACHE_COMMENTS} "1")
  endif()
endif()

# ----------------------------------------------------------------------
# Setup dependency install path. The path is determined by deps/vars.sh (avoid hard-code twice)
execute_process(
  COMMAND bash -c "source ${PROJECT_SOURCE_DIR}/deps/vars.sh && echo -n $DEPS_INSTALL_DIR"
  OUTPUT_VARIABLE DEPS_INSTALL
  RESULT_VARIABLE DEPS_INSTALL_CMD_STATUS
)
if(DEPS_INSTALL_CMD_STATUS EQUAL "1")
  message(FATAL_ERROR "Unable to get DEPS_INSTALL_DIR!")
endif()
message(STATUS "Setting DEPS_INSTALL to ${DEPS_INSTALL}")
list(APPEND CMAKE_PREFIX_PATH "${DEPS_INSTALL}")
list(APPEND CMAKE_MODULE_PATH "${DEPS_INSTALL}")

set(Boost_USE_STATIC_RUNTIME ON)

set(GFLAGS_USE_TARGET_NAMESPACE TRUE)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_BINARY_DIR}) # for generated proto files
include_directories(${CMAKE_BINARY_DIR}/src) # for generated proto files

# TODO(xxx): Maybe we should remove below two lines to force our target explicitly refence third party libraries?
include_directories(SYSTEM ${DEPS_INSTALL}/include)
link_directories(${DEPS_INSTALL}/lib)

# ----------------------------------------------------------------------
# Make targets

# ~~~
# "make cmake-format" and "make check-cmake-format" targets
# pip install cmakelang
# ~~~
find_program(CMAKE_FORMAT_BIN NAMES cmake-format)
if(NOT CMAKE_FORMAT_BIN)
  message(STATUS "Could not find cmake-format")
else()
  set(CMAKE_FORMAT_CONF_FILE "${PROJECT_SOURCE_DIR}/.cmake-format.json")
  message(STATUS "Found cmake-format: ${CMAKE_FORMAT_BIN}")

  # find CMakeLists.txt files
  file(
    GLOB_RECURSE
    _cmake_file_sources
    src/CMakeLists.txt
    proto/CMakeLists.txt
    ./cmake/*.cmake
  )
  list(APPEND _cmake_file_sources ${PROJECT_SOURCE_DIR}/CMakeLists.txt)

  add_custom_target(
    cmake-format
    ${CMAKE_FORMAT_BIN}
    -i # in-place formatting
    -c=${CMAKE_FORMAT_CONF_FILE}
    ${_cmake_file_sources}
    COMMENT "Runs cmake-format and format CMake files in place."
  )

  add_custom_target(
    check-cmake-format
    ${CMAKE_FORMAT_BIN}
    --check
    -c=${CMAKE_FORMAT_CONF_FILE}
    ${_cmake_file_sources}
    COMMENT "Runs cmake-format and exits with status code 1 if changes are needed."
  )
endif()

# ----------------------------------------------------------------------
# Dependency packages

find_package(PkgConfig REQUIRED)
#! make pkg find static dependencies, e.g. libs.private in <lib>.pc file
#! this only works by pkg-config
list(APPEND PKG_CONFIG_EXECUTABLE "--static")

# openssl
pkg_check_modules(
  openssl
  REQUIRED
  IMPORTED_TARGET
  openssl
)

# brpc : opennssl
pkg_check_modules(
  brpc
  REQUIRED
  IMPORTED_TARGET
  brpc
)
target_link_libraries(PkgConfig::brpc INTERFACE PkgConfig::openssl)

# TODO(ycli): Currently re2 is implicitly imported as re2:re2 by GRPC, try import explicitly pkg_check_modules( re2
# REQUIRED IMPORTED_TARGET re2 )

# gflags, make cmake able to find gflags::gflags_static
# ~~~
# set(GFLAGS_NOTHREADS FALSE)
# find_package(gflags CONFIG REQUIRED)
# ~~~

# glog : gflags
find_package(glog CONFIG REQUIRED)

# protobuf
find_package(protobuf CONFIG REQUIRED)

# fmt
find_package(Fmt CONFIG REQUIRED)

if(BUILD_TESTING)
  include(GoogleTest)
  find_package(GTest CONFIG REQUIRED)
endif()

# oneTBB
find_package(TBB CONFIG REQUIRED)

add_subdirectory(proto)
add_subdirectory(src)
