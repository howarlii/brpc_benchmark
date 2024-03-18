#!/bin/bash

#export DEPS_INSTALL_DIR=${DEPS_DIR}/install
export DEPS_INSTALL_DIR=/opt/install # install on /opt/install folder
export DEPS_SOURCE_DIR=${DEPS_DIR}/src
export DEPS_PATCH_DIR=${DEPS_DIR}/patches

export DEPS_INCLUDE_DIR=${DEPS_INSTALL_DIR}/include
export DEPS_LIB_DIR=${DEPS_INSTALL_DIR}/lib


#temporary build dir
export BUILD_DIR="__build"

# Abseil
ABSEIL_CPP_DOWNLOAD="https://github.com/abseil/abseil-cpp/archive/refs/tags/20230802.1.tar.gz"
ABSEIL_CPP_NAME="abseil-LTS-20230802.1.tar.gz"
ABSEIL_CPP_SOURCE="abseil-LTS-20230802.1"
ABSEIL_CPP_SHA256="987ce98f02eefbaf930d6e38ab16aa05737234d7afbab2d5c4ea7adbe50c28ed"

# backwardcpp
BACKWARD_CPP_DOWNLOAD="https://github.com/bombela/backward-cpp/archive/refs/tags/v1.6.tar.gz"
BACKWARD_CPP_NAME="backward-cpp-v1.6.tar.gz"
BACKWARD_CPP_SOURCE="backward-cpp-v1.6"
BACKWARD_CPP_SHA256="c654d0923d43f1cea23d086729673498e4741fb2457e806cfaeaea7b20c97c10"

# protobuf
PROTOBUF_DOWNLOAD="https://github.com/protocolbuffers/protobuf/archive/refs/tags/v21.12.tar.gz"
PROTOBUF_NAME="protobuf-21.12.tar.gz"
PROTOBUF_SOURCE="protobuf-21.12"
PROTOBUF_SHA256="22fdaf641b31655d4b2297f9981fa5203b2866f8332d3c6333f6b0107bb320de"

#PROTOBUF_DOWNLOAD="https://github.com/protocolbuffers/protobuf/archive/refs/tags/v24.4.tar.gz"
#PROTOBUF_NAME="protobuf-24.4.tar.gz"
#PROTOBUF_SOURCE="protobuf-24.4"
#PROTOBUF_SHA256="616bb3536ac1fff3fb1a141450fa28b875e985712170ea7f1bfe5e5fc41e2cd8"

# boost
BOOST_DOWNLOAD="https://boostorg.jfrog.io/artifactory/main/release/1.78.0/source/boost_1_78_0.tar.gz"
BOOST_NAME=boost_1_78_0.tar.gz
BOOST_SOURCE=boost_1_78_0
BOOST_SHA256="94ced8b72956591c4775ae2207a9763d3600b30d9d7446562c552f0a14a63be7"

# fmt
FMTLIB_DOWNLOAD="https://github.com/fmtlib/fmt/archive/refs/tags/8.0.1.tar.gz"
FMTLIB_NAME="fmt-8.0.1.tar.gz"
FMTLIB_SOURCE="fmt-8.0.1"
FMTLIB_SHA256="b06ca3130158c625848f3fb7418f235155a4d389b2abc3a6245fb01cb0eb1e01"

# spdlog
# SPDLOG_DOWNLOAD="https://github.com/gabime/spdlog/archive/refs/tags/v1.11.0.tar.gz"
# SPDLOG_NAME="spdlog-v1.11.0.tar.gz"
# SPDLOG_SOURCE="spdlog-v1.11.0"
# SPDLOG_SHA256="ca5cae8d6cac15dae0ec63b21d6ad3530070650f68076f3a4a862ca293a858bb"

# gtest
GTEST_DOWNLOAD="https://github.com/google/googletest/archive/refs/tags/v1.13.0.tar.gz"
GTEST_NAME="gtest-v1.13.0.tar.gz"
GTEST_SOURCE="gtest-v1.13.0"
GTEST_SHA256="ad7fdba11ea011c1d925b3289cf4af2c66a352e18d4c7264392fead75e919363"

# glog
GLOG_DOWNLOAD="https://github.com/google/glog/archive/v0.6.0.tar.gz"
GLOG_NAME="glog-0.6.0.tar.gz"
GLOG_SOURCE="glog-0.6.0"
GLOG_SHA256="8a83bf982f37bb70825df71a9709fa90ea9f4447fb3c099e1d720a439d88bad6"

# gflags
GFLAGS_DOWNLOAD="https://github.com/lych4o/gflags/archive/refs/tags/v2.2.2.tar.gz"
GFLAGS_NAME="gflags-2.2.2.tar.gz"
GFLAGS_SOURCE="gflags-2.2.2"
GFLAGS_SHA256="865a89ca0e9c78a390de2935ca8f3d1416893a767786953e5ee9d4a821573ec2"

# brpc: protobuf, gflags, leveldb
BRPC_DOWNLOAD="https://github.com/apache/brpc/archive/refs/tags/1.6.0.tar.gz"
BRPC_NAME="brpc-1.6.0.tar.gz"
BRPC_SOURCE="brpc-1.6.0"
BRPC_SHA256="d286d520ec4d317180d91ea3970494c1b8319c8867229e5c4784998c4536718f"

# TBB
TBB_DOWNLOAD="https://github.com/oneapi-src/oneTBB/archive/refs/tags/v2021.8.0.tar.gz"
TBB_NAME="oneTBB-2021.8.0.tar.gz"
TBB_SOURCE="oneTBB-2021.8.0"
TBB_SHA256="eee380323bb7ce864355ed9431f85c43955faaae9e9bce35c62b372d7ffd9f8b"


# ALL DEPS
DEPS_ARCHIVES_DOWNLOAD="ABSEIL_CPP BACKWARD_CPP PROTOBUF BOOST DOUBLE_CONVERSION FMTLIB SPDLOG GTEST GLOG GFLAGS LIBEVENT FOLLY LEVELDB BRPC TBB HDRHISTOGRAM ETCD_CPP_API"

# grpc
GRPC_REPO="https://github.com/grpc/grpc.git"
GRPC_TAG="v1.51.0"
GRPC_SOURCE="grpc-1.51.0"

DEPS_ARCHIVES_CLONE="GRPC"
