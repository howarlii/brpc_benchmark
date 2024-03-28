#pragma once

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <string>

struct BenchmarkConfig {
  int parallelism = 4;        // Number of threads to send requests
  int benchmark_time = 2000;  // Client request number
  int req_size = 10'240;      // Size of attachment
  int resp_size = 10'240;     // Size of attachment

  // ==== attachment settings ====
  bool use_attachment = false;  // Whether to use attachment

  // ===== proto settings =====
  bool use_proto_bytes = false;

  // ===== streaming settings =====
  bool use_single_streaming = false;    // Streaming, create a streaming every time
  bool use_continue_streaming = false;  // Continue streaming, a thread will create a streaming at
                                        // begining and use the that stream to send requests
  int continue_stream_messages_in_batch = 128;
  int continue_stream_max_buf_size = 2097152;

  int single_stream_single_msg_size = 8192;

  // ===== channel settings =====
  bool use_parallel_channel = false;  // Whether to use parallel channel
  int parallel_channel_num = 2;       // Number of sub channels, avaliable when use parallel channel

  // ===== thread settings =====
  bool use_bthread = false;  // false to use pthread to send requests

  // ===== brpc settings =====
  std::string connection_type = "";  // Connection type. Available values: single, pooled, short

  std::string rpc_protocol = "baidu_std";  // Protocol type. Defined in src/brpc/options.proto
  // std::string rpc_protocol = "h2:grpc";

  std::string server_addr = "127.0.0.1:8102";  // IP Address of server
  // std::string server_addr = "0.0.0.0:8102";  // IP Address of server
  int brpc_port = 8102;  // TCP Port of server

  std::string load_balancer = "";  // The algorithm for load balancing

  int rpc_timeout_ms = 1'000'000;  // RPC timeout in milliseconds

  int rpc_max_retry = 3;  // Max retries(not including the first RPC)
};

static const BenchmarkConfig default_config;

DEFINE_int32(parallelism, default_config.parallelism, "Number of threads to send requests");
DEFINE_int32(benchmark_time, default_config.benchmark_time, "Client request number");
DEFINE_int32(req_size, default_config.req_size, "Size of request payload");
DEFINE_int32(resp_size, default_config.resp_size, "Size of respone payload");

// ==== attachment settings ====
DEFINE_bool(use_attachment, default_config.use_attachment, "Whether to use attachment");

// ===== proto settings =====
DEFINE_bool(use_proto_bytes, default_config.use_proto_bytes, "Whether to use proto bytes");

// ===== streaming settings =====
DEFINE_bool(use_streaming, default_config.use_single_streaming, "Whether to use streaming");
DEFINE_bool(use_continue_streaming, default_config.use_continue_streaming, "Whether to use continue streaming");
DEFINE_int32(continue_stream_messages_in_batch, default_config.continue_stream_messages_in_batch,
             "BRPC streaming option");
DEFINE_int32(continue_stream_max_buf_size, default_config.continue_stream_max_buf_size, "BRPC streaming option");
DEFINE_int32(single_stream_single_msg_size, default_config.single_stream_single_msg_size, "Streaming message size");

// ==== rpc protocol settings ====
DEFINE_string(rpc_protocol, default_config.rpc_protocol, "Protocol type. Defined in src/brpc/options.proto");

// ==== server settings ====
DEFINE_string(server_addr, default_config.server_addr, "IP Address of server");
DEFINE_int32(brpc_port, default_config.brpc_port, "TCP Port of server");

// write a function to parse the command line arguments
inline BenchmarkConfig parseCommandLine(int argc, char **argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  BenchmarkConfig config;
  config.parallelism = FLAGS_parallelism;
  config.benchmark_time = FLAGS_benchmark_time;
  config.req_size = FLAGS_req_size;
  config.resp_size = FLAGS_resp_size;

  // ==== attachment settings ====
  config.use_attachment = FLAGS_use_attachment;

  // ===== proto settings =====
  config.use_proto_bytes = FLAGS_use_proto_bytes;

  // ===== streaming settings =====
  config.use_single_streaming = FLAGS_use_streaming;
  config.use_continue_streaming = FLAGS_use_continue_streaming;
  config.continue_stream_messages_in_batch = FLAGS_continue_stream_messages_in_batch;
  config.continue_stream_max_buf_size = FLAGS_continue_stream_max_buf_size;
  config.single_stream_single_msg_size = FLAGS_single_stream_single_msg_size;

  // ===== rpc protocol settings =====
  config.rpc_protocol = FLAGS_rpc_protocol;

  // ==== server settings ====
  config.server_addr = FLAGS_server_addr;
  config.brpc_port = FLAGS_brpc_port;

  return config;
}
