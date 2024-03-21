#!/bin/bash

set -e

### Install Base Requirements
# NOTE(pgao): remove apt install, make the build script os independent.
#apt install -y libssl-dev

export DEPS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

if [ ! -f ${DEPS_DIR}/vars.sh ]; then
    echo "vars.sh is missing".
    exit 1
fi
# shellcheck source=.
. ${DEPS_DIR}/vars.sh

if [[ ! -f ${DEPS_DIR}/download_deps.sh ]]; then
    echo "Download deps script is missing".
    exit 1
fi

# Download
${DEPS_DIR}/download_deps.sh

if [[ ! -f ${DEPS_DIR}/clone_deps.sh ]]; then
    echo "Clone deps script is missing".
    exit 1
fi

# Clone
${DEPS_DIR}/clone_deps.sh

echo "====="
echo "Deps are installed in DEPS_INSTALL_DIR: ${DEPS_INSTALL_DIR}"

# setup toolchain
export CC=clang
export CXX=clang++
export PARALLEL=12
CMAKE_GENERATOR=Ninja

# TODO(xxx): Automatically check cpu architecture
# Temporary code to compile folly using AVX instruction
source ${DEPS_SOURCE_DIR}/${VELOX_SOURCE}/scripts/setup-helper-functions.sh
CPU_TARGET="unknown"
COMPILER_FLAGS=\"$(get_cxx_flags $CPU_TARGET)\"

COMMON_CMAKE_FLAGS="-DCMAKE_INSTALL_PREFIX=${DEPS_INSTALL_DIR} \
                    -DCMAKE_PREFIX_PATH=${DEPS_INSTALL_DIR} \
                    -DCMAKE_BUILD_TYPE=Release \
                    -DCMAKE_C_COMPILER=$CC \
                    -DCMAKE_CXX_COMPILER=$CXX \
                    -DBUILD_SHARED_LIBS=OFF \
                    -DCMAKE_CXX_STANDARD=20 \
                    -DBUILD_TESTING=OFF \
                    -DCMAKE_REQUIRED_INCLUDES=${DEPS_INSTALL_DIR}/include \
                    -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
                    -DCMAKE_CXX_FLAGS=$COMPILER_FLAGS"

# prepare build
mkdir -p ${DEPS_INSTALL_DIR}


check_if_source_exist() {
    if [ -z $1 ]; then
        echo "dir should specified to check if exist."
        exit 1
    fi

    if [ ! -d ${DEPS_SOURCE_DIR}/$1 ];then
        echo "${DEPS_SOURCE_DIR}/$1 does not exist."
        exit 1
    fi
    echo "===== begin build $1"
}


########
######## Install All Deps
########

# abseil_cpp
build_abseil_cpp() {
    check_if_source_exist ${ABSEIL_CPP_SOURCE}
    cd ${DEPS_SOURCE_DIR}/${ABSEIL_CPP_SOURCE}

    mkdir -p ${BUILD_DIR}
    cd ${BUILD_DIR}
    bash -c "cmake ${COMMON_CMAKE_FLAGS} .."
    make -j ${PARALLEL} install
}

# backward_cpp
build_backward_cpp() {
    check_if_source_exist ${BACKWARD_CPP_SOURCE}
    cd ${DEPS_SOURCE_DIR}/${BACKWARD_CPP_SOURCE}
    
    mkdir -p ${BUILD_DIR}
    cd ${BUILD_DIR}
    bash -c "cmake ${COMMON_CMAKE_FLAGS} .."
    make -j ${PARALLEL} install
}

# protobuf
build_protobuf() {
    check_if_source_exist ${PROTOBUF_SOURCE}
    cd ${DEPS_SOURCE_DIR}/${PROTOBUF_SOURCE}

    mkdir -p ${BUILD_DIR}
    cd ${BUILD_DIR}
    bash -c "cmake .. ${COMMON_CMAKE_FLAGS} \
        -Dprotobuf_BUILD_TESTS=OFF \
        -Dprotobuf_ABSL_PROVIDER=\"package\" \
    "
    make -j ${PARALLEL} install
}

# boost
build_boost() {
    check_if_source_exist ${BOOST_SOURCE}
    cd ${DEPS_SOURCE_DIR}/${BOOST_SOURCE}

    ./bootstrap.sh --prefix=${DEPS_INSTALL_DIR} --with-toolset=clang
    ./b2 link=static runtime-link=static -j $PARALLEL \
    --without-mpi --without-graph --without-graph_parallel \
    --without-python cxxflags="-std=c++11 -g -fPIC \
    -I${DEPS_INCLUDE_DIR} -L${DEPS_LIB_DIR}" install
}

# double conversion
build_double_conversion() {
    check_if_source_exist ${DOUBLE_CONVERSION_SOURCE}
    cd ${DEPS_SOURCE_DIR}/${DOUBLE_CONVERSION_SOURCE}
    mkdir -p ${BUILD_DIR}
    cd ${BUILD_DIR}
    bash -c "cmake $COMMON_CMAKE_FLAGS .."
    make -j ${PARALLEL} install
}

# fmt
build_fmtlib() {
    check_if_source_exist ${FMTLIB_SOURCE}
    cd ${DEPS_SOURCE_DIR}/${FMTLIB_SOURCE}
    mkdir -p ${BUILD_DIR}
    cd ${BUILD_DIR}
    bash -c "cmake $COMMON_CMAKE_FLAGS -DFMT_TEST=OFF .."
    make -j ${PARALLEL} install
}

# spdlog
# build_spdlog() {
#     check_if_source_exist ${SPDLOG_SOURCE}
#     cd ${DEPS_SOURCE_DIR}/${SPDLOG_SOURCE}
#     mkdir -p ${BUILD_DIR}
#     cd ${BUILD_DIR}
#     bash -c "cmake ${COMMON_CMAKE_FLAGS} -DSPDLOG_USE_STD_FORMAT=ON.."
#     make -j ${PARALLEL} install
# }

# gtest
build_gtest() {
    check_if_source_exist ${GTEST_SOURCE}
    cd ${DEPS_SOURCE_DIR}/${GTEST_SOURCE}
    mkdir -p ${BUILD_DIR}
    cd ${BUILD_DIR}
    bash -c "cmake $COMMON_CMAKE_FLAGS .."
    make -j ${PARALLEL} install
}

# glog
build_glog() {
    check_if_source_exist ${GLOG_SOURCE}
    cd ${DEPS_SOURCE_DIR}/${GLOG_SOURCE}
    mkdir -p ${BUILD_DIR}
    cd ${BUILD_DIR}

    # Glog library will depend on libunwind if libunwind is installed,
    # brpc use find_path in camke to use glog so that brpc will not link libunwind.
    # Use dynamic glog library to solve this problem
    bash -c "cmake $COMMON_CMAKE_FLAGS \
        -DBUILD_STATIC_LIBS=OFF \
        -DBUILD_SHARED_LIBS=ON \
        .."
    make -j ${PARALLEL} install
}

# gflags
build_gflags() {
    check_if_source_exist ${GFLAGS_SOURCE}
    cd ${DEPS_SOURCE_DIR}/${GFLAGS_SOURCE}
    mkdir -p ${BUILD_DIR}
    cd ${BUILD_DIR}

    # Velox depends on gflags shared library
    bash -c "cmake $COMMON_CMAKE_FLAGS -DBUILD_STATIC_LIBS=ON -DINSTALL_STATIC_LIBS=ON \
          -DBUILD_SHARED_LIBS=ON -DINSTALL_SHARED_LIBS=ON .."
    make -j ${PARALLEL} install
}

build_libevent() {
    check_if_source_exist ${LIBEVENT_SOURCE}
    cd ${DEPS_SOURCE_DIR}/${LIBEVENT_SOURCE}
    mkdir -p ${BUILD_DIR}
    cd ${BUILD_DIR}
    bash -c "cmake $COMMON_CMAKE_FLAGS  \
        -DEVENT__DISABLE_TESTS=ON  \
        -DEVENT__DISABLE_BENCHMARK=ON \
        -DEVENT__DISABLE_SAMPLES=ON  \
        -DEVENT__DISABLE_REGRESS=ON  \
        -DEVENT__LIBRARY_TYPE=STATIC \
        .."
    make -j ${PARALLEL} install
}

# folly
build_folly() {
    check_if_source_exist ${FOLLY_SOURCE}
    cd ${DEPS_SOURCE_DIR}/${FOLLY_SOURCE}
    mkdir -p ${BUILD_DIR}
    cd ${BUILD_DIR}
    bash -c "cmake $COMMON_CMAKE_FLAGS \
        -DGFLAGS_USE_TARGET_NAMESPACE=TRUE \
        -DBoost_USE_STATIC_RUNTIME=ON \
        -DBOOST_LINK_STATIC=ON \
        -DBUILD_TESTS=OFF \
        -DGFLAGS_NOTHREADS=OFF \
        -DFOLLY_HAVE_INT128_T=ON \
        -DCXX_STD="c++20" \
        .."
    make -j ${PARALLEL} install
}

# leveldb
build_leveldb() {
    check_if_source_exist ${LEVELDB_SOURCE}
    cd ${DEPS_SOURCE_DIR}/${LEVELDB_SOURCE}
    mkdir -p ${BUILD_DIR}
    cd ${BUILD_DIR}

    bash -c "cmake ${COMMON_CMAKE_FLAGS} \
	  -DLEVELDB_BUILD_TESTS=OFF \
	  -DLEVELDB_BUILD_BENCHMARKS=OFF \
	  .."
    make -j ${PARALLEL} install
}

# brpc
build_brpc() {
    check_if_source_exist ${BRPC_SOURCE}
    cd ${DEPS_SOURCE_DIR}/${BRPC_SOURCE}
    mkdir -p ${BUILD_DIR}
    cd ${BUILD_DIR}

    bash -c "cmake ${COMMON_CMAKE_FLAGS} \
      -DWITH_GLOG=ON \
      -DWITH_SNAPPY=ON \
	  .."
    make -j ${PARALLEL} install
}

# arrow
build_arrow() {
    check_if_source_exist ${ARROW_SOURCE}
    cd ${DEPS_SOURCE_DIR}/${ARROW_SOURCE}/cpp
    mkdir -p ${BUILD_DIR}
    cd ${BUILD_DIR}

    # note: AWS-centos has openssl as shared lib, please set this option on to make it build.
    bash -c "cmake -G${CMAKE_GENERATOR} ${COMMON_CMAKE_FLAGS} \
        -DBUILD_WARNING_LEVEL=PRODUCTION \
        -DARROW_ALTIVEC=OFF \
        -DARROW_DEPENDENCY_USE_SHARED=OFF \
        -DARROW_USE_CCACHE=ON -DARROW_BOOST_USE_SHARED=OFF -DARROW_BUILD_SHARED=OFF \
        -DARROW_BUILD_STATIC=ON -DARROW_COMPUTE=ON -DARROW_CSV=ON -DARROW_IPC=ON -DARROW_JEMALLOC=OFF  \
        -DARROW_JSON=OFF -DARROW_PARQUET=ON -DARROW_SIMD_LEVEL=NONE -DARROW_RUNTIME_SIMD_LEVEL=NONE  \
        -DARROW_WITH_BROTLI=OFF \
        -DARROW_WITH_LZ4=ON -Dlz4_SOURCE=BUNDLED -DARROW_WITH_PROTOBUF=OFF -DARROW_WITH_RAPIDJSON=OFF \
        -DARROW_WITH_SNAPPY=ON -DSnappy_SOURCE=BUNDLED -DARROW_WITH_ZLIB=ON -DZLIB_SOURCE=BUNDLED \
        -DARROW_WITH_ZSTD=ON -Dzstd_SOURCE=BUNDLED -DThrift_SOURCE=BUNDLED -DBOOST_ROOT=${DEPS_INSTALL_DIR}  \
        -DARROW_WITH_RE2=OFF \
        -DARROW_WITH_UTF8PROC=OFF -DARROW_BUILD_BENCHMARKS=OFF -DARROW_BUILD_EXAMPLES=OFF  \
        -DARROW_BUILD_INTEGRATION=OFF \
        -DARROW_CSV=ON \
        -DARROW_FILESYSTEM=ON \
        -DARROW_GCS=ON \
        -DARROW_HDFS=ON \
        -DARROW_PARQUET=ON \
        -DARROW_S3=ON \
        -DARROW_BUILD_UTILITIES=ON -DARROW_BUILD_TESTS=OFF -DARROW_ENABLE_TIMING_TESTS=OFF  \
        -DARROW_FUZZING=OFF \
	-DARROW_OPENSSL_USE_SHARED=ON \
        .."
    cmake --build . --config Release -- -j $PARALLEL
    cmake --install .
}

# tbb
build_tbb() {
    check_if_source_exist ${TBB_SOURCE}
    cd ${DEPS_SOURCE_DIR}/${TBB_SOURCE}
    mkdir -p ${BUILD_DIR}
    cd ${BUILD_DIR}
    bash -c "cmake ${COMMON_CMAKE_FLAGS} -DTBB_STRICT=OFF .."
    cmake --build . --config Release -- -j $PARALLEL
    cmake --install .
}


#HdrHistogram
build_hdrHistogram() {
    check_if_source_exist ${HDRHISTOGRAM_SOURCE}
    cd ${DEPS_SOURCE_DIR}/${HDRHISTOGRAM_SOURCE}
    mkdir -p ${BUILD_DIR}
    cd ${BUILD_DIR}
    bash -c "cmake ${COMMON_CMAKE_FLAGS} \
     -DHDR_HISTOGRAM_BUILD_SHARED=OFF \
     -DHDR_HISTOGRAM_BUILD_PROGRAMS=OFF \
     .."
    make -j ${PARALLEL} install
}    

#Velox
build_velox() {
    check_if_source_exist ${VELOX_SOURCE}
    cd ${DEPS_SOURCE_DIR}/${VELOX_SOURCE}
    #bash -c "./scripts/setup-ubuntu.sh"
}

build_grpc(){
    check_if_source_exist ${GRPC_SOURCE}
    cd ${DEPS_SOURCE_DIR}/${GRPC_SOURCE}
    git submodule init
    git submodule update
    mkdir -p "cmake/${BUILD_DIR}"
    cd "cmake/${BUILD_DIR}"
    bash -c "cmake ${COMMON_CMAKE_FLAGS} \
     -DgRPC_ZLIB_PROVIDER=\"package\" \
     -DgRPC_RE2_PROVIDER=\"package\" \
     -DgRPC_SSL_PROVIDER=\"package\" \
     -DgRPC_PROTOBUF_PROVIDER=\"package\" \
     -DgRPC_ABSL_PROVIDER=\"package\" \
     -DgRPC_BUILD_TESTS=OFF \
     ../.."
    make -j ${PARALLEL} install
}

# etcd-cpp-apiv3
build_etcd_cpp_api() {
    check_if_source_exist ${ETCD_CPP_API_SOURCE}
    cd ${DEPS_SOURCE_DIR}/${ETCD_CPP_API_SOURCE}
    mkdir -p ${BUILD_DIR}
    cd ${BUILD_DIR}
    bash -c "cmake ${COMMON_CMAKE_FLAGS}  \
     -DBUILD_ETCD_CORE_ONLY=ON \
     -DETCD_W_STRICT=OFF \
     -DBUILD_SHARED_LIBS=ON \
     -DETCD_CMAKE_CXX_STANDARD=20 \
    .."
    make -j ${PARALLEL} install
}

build_abseil_cpp
build_backward_cpp
build_protobuf
build_boost
build_double_conversion
build_fmtlib
# build_spdlog
build_gtest
build_gflags
build_glog
build_libevent
build_folly
build_leveldb
build_arrow
build_tbb
build_hdrHistogram
build_velox
build_grpc
build_brpc
build_etcd_cpp_api
echo "Finished to build all dependencies"
