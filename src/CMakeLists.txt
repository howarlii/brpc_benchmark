# TODO(xxx): Split into test objects
# ----------------- unit tests -----------------
add_executable(brpc_test_server benchmark_server.cc)
add_executable(brpc_test_client benchmark_client.cc)
target_link_libraries(
  brpc_test_server
  PRIVATE pb_obj
          fmt::fmt
          glog::glog
          PkgConfig::brpc
)
target_link_libraries(
  brpc_test_client
  PRIVATE pb_obj
          fmt::fmt
          glog::glog
          PkgConfig::brpc
          tbb
)
