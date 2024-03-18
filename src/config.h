#pragma once

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <string>

struct BenchmarkConfig {
  int parallelism = 4;           // Number of threads to send requests
  int client_request_num = 1000; // Client request number

  // ==== attachment settings ====
  bool use_attachment = false;        // Whether to use attachment
  int attachment_req_size = 100'000;  // Size of attachment
  int attachment_resp_size = 100'000; // Size of attachment

  // ===== proto settings =====
  bool use_proto_bytes = false;
  int proto_req_data_size_byte =
      100'000; // Carry so many byte data along with requests
  int proto_resp_data_size_byte =
      100'000; // Carry so many byte data along with respone

  // ===== streaming settings =====
  bool use_streaming = false;          // Whether to use streaming
  int stream_client_msg_size = 10'000; // message size for streaming
  int stream_client_msg_num = 10;      // message number for streaming

  // ===== channel settings =====
  bool use_parallel_channel = false; // Whether to use parallel channel
  int parallel_channel_num =
      2; // Number of sub channels, avaliable when use parallel channel

  // ===== thread settings =====
  bool use_bthread = false; // false to use pthread to send requests

  // ===== brpc settings =====
  std::string connection_type =
      ""; // Connection type. Available values: single, pooled, short

  std::string rpc_protocol =
      "baidu_std"; // Protocol type. Defined in src/brpc/options.proto
  // std::string rpc_protocol = "h2:grpc";

  std::string server_addr = "127.0.0.1:8102"; // IP Address of server
  // std::string server_addr = "0.0.0.0:8102";  // IP Address of server
  int brpc_port = 8102; // TCP Port of server

  std::string load_balancer = ""; // The algorithm for load balancing

  int rpc_timeout_ms = 1'000'000; // RPC timeout in milliseconds

  int rpc_max_retry = 3; // Max retries(not including the first RPC)
} default_config;

DEFINE_int32(parallelism, default_config.parallelism,
             "Number of threads to send requests");
DEFINE_int32(client_request_num, default_config.client_request_num,
             "Client request number");

// ==== attachment settings ====
DEFINE_bool(use_attachment, default_config.use_attachment,
            "Whether to use attachment");
DEFINE_int32(attachment_req_size, default_config.attachment_req_size,
             "Size of attachment");
DEFINE_int32(attachment_resp_size, default_config.attachment_resp_size,
             "Size of attachment");

// ===== proto settings =====
DEFINE_bool(use_proto_bytes, default_config.use_proto_bytes,
            "Whether to use proto bytes");
DEFINE_int32(proto_req_data_size_byte, default_config.proto_req_data_size_byte,
             "Carry so many byte data along with requests");
DEFINE_int32(proto_resp_data_size_byte,
             default_config.proto_resp_data_size_byte,
             "Carry so many byte data along with response");

// ===== streaming settings =====
DEFINE_bool(use_streaming, default_config.use_streaming,
            "Whether to use streaming");
DEFINE_int32(stream_client_msg_size, default_config.stream_client_msg_size,
             "Message size for streaming");
DEFINE_int32(stream_client_msg_num, default_config.stream_client_msg_num,
             "Message number for streaming");

// ==== rpc protocol settings ====
DEFINE_string(rpc_protocol, default_config.rpc_protocol,
              "Protocol type. Defined in src/brpc/options.proto");

// ==== server settings ====
DEFINE_string(server_addr, default_config.server_addr, "IP Address of server");
DEFINE_int32(brpc_port, default_config.brpc_port, "TCP Port of server");

// write a function to parse the command line arguments
inline BenchmarkConfig parseCommandLine(int argc, char **argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  BenchmarkConfig config;
  config.parallelism = FLAGS_parallelism;
  config.client_request_num = FLAGS_client_request_num;

  // ==== attachment settings ====
  config.use_attachment = FLAGS_use_attachment;
  config.attachment_req_size = FLAGS_attachment_req_size;
  config.attachment_resp_size = FLAGS_attachment_resp_size;

  // ===== proto settings =====
  config.use_proto_bytes = FLAGS_use_proto_bytes;
  config.proto_req_data_size_byte = FLAGS_proto_req_data_size_byte;
  config.proto_resp_data_size_byte = FLAGS_proto_resp_data_size_byte;

  // ===== streaming settings =====
  config.use_streaming = FLAGS_use_streaming;
  config.stream_client_msg_size = FLAGS_stream_client_msg_size;
  config.stream_client_msg_num = FLAGS_stream_client_msg_num;

  // ===== rpc protocol settings =====
  config.rpc_protocol = FLAGS_rpc_protocol;

  // ==== server settings ====
  config.server_addr = FLAGS_server_addr;
  config.brpc_port = FLAGS_brpc_port;

  return config;
}
