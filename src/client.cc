// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

// A client sending requests to server in parallel by multiple threads.

#include <brpc/parallel_channel.h>
#include <brpc/server.h>
#include <bthread/bthread.h>
#include <butil/macros.h>
#include <butil/time.h>
#include <fmt/core.h>
#include <gflags/gflags.h>
#include <glog/logging.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

#include "brpc_client.h"
#include "config.h"
#include "proto/echo.pb.h"
#include "util.h"

class ClientBenchmarker {
public:
  explicit ClientBenchmarker(BenchmarkConfig config = BenchmarkConfig())
      : config_(std::move(config)) {}

  void init() {
    brpc::ChannelOptions channel_options;
    channel_options.protocol = config_.rpc_protocol;
    if (config_.use_streaming) {
      CHECK_EQ(config_.rpc_protocol, "baidu_std")
          << "RPC protocol must be baidu_std when enable streaming";
    }
    channel_options.max_retry = config_.rpc_max_retry;
    channel_options.timeout_ms = config_.rpc_timeout_ms;

    if (config_.use_parallel_channel) {
      num_channel_ = config_.parallel_channel_num;
      // LOG(INFO) << "use parallel channel, channel_num=" << num_channel_;
      brpc::ParallelChannelOptions pchan_options;
      pchan_options.timeout_ms = config_.rpc_timeout_ms;
      if (p_channel_.Init(&pchan_options) != 0) {
        LOG(ERROR) << "Fail to init ParallelChannel";
        exit(-1);
      }

      for (int i = 0; i < num_channel_; ++i) {
        auto sub_channel = new brpc::Channel;
        // Initialize the channel, nullptr means using default options.
        // options, see `brpc/channel.h'.
        if (sub_channel->Init(config_.server_addr.c_str(),
                              config_.load_balancer.c_str(),
                              &channel_options) != 0) {
          LOG(ERROR) << "Fail to initialize sub_channel[" << i << "]";
          exit(-1);
        }
        if (p_channel_.AddChannel(sub_channel, brpc::OWNS_CHANNEL, nullptr,
                                  nullptr) != 0) {
          LOG(ERROR) << "Fail to AddChannel, i=" << i;
          exit(-1);
        }
      }
      for (int i = 0; i < num_channel_; ++i) {
        recorder_.sub_channel_latency.emplace_back(
            std::make_unique<bvar::LatencyRecorder>());
        recorder_.sub_channel_latency.back()->expose(
            fmt::format("client_sub_{}", i));
      }
    } else {
      num_channel_ = 1;
      // LOG(INFO) << "use single channel, channel_num=" << num_channel_;
      channel_ = std::make_unique<brpc::Channel>();
      if (channel_->Init(config_.server_addr.c_str(),
                         config_.load_balancer.c_str(),
                         &channel_options) != 0) {
        LOG(ERROR) << "Fail to initialize channel";
        exit(-1);
      }
    }
  }

  void start(bool is_req_bench = true) {
    auto get_stub = [&]() {
      if (config_.use_parallel_channel) {
        return std::make_unique<example::EchoService_Stub>(&p_channel_);
      } else {
        return std::make_unique<example::EchoService_Stub>(channel_.get());
      }
    };

    if (!config_.use_bthread) {
      for (auto i = 0; i < config_.parallelism; ++i) {
        clients_.emplace_back(std::make_unique<Client>(&config_, &recorder_));
        auto c = clients_.back().get();

        c->registeDoneCallBack([this]() { running_cnt_--; });
        running_cnt_++;
        if (is_req_bench) {
          jthreads_.emplace_back([stub = get_stub(), c, this]() mutable {
            c->runReqBench(std::move(stub), config_.client_request_num);
          });
        } else {
          jthreads_.emplace_back([stub = get_stub(), c, this]() mutable {
            c->runRespBench(std::move(stub), config_.client_request_num);
          });
        }
      }
    } else {
      LOG(INFO) << "QWQ not implemented";
      exit(-1);
    }
  }

  void output() { recorder_.output(); }

  auto getSentBytes() const {
    uint64_t bytes{0};
    for (const auto &c : clients_) {
      bytes += c->getSentBytes();
    }
    return bytes;
  }
  auto getRecievedBytes() const {
    uint64_t bytes{0};
    for (const auto &c : clients_) {
      bytes += c->getRecievedBytes();
    }
    return bytes;
  }
  auto &getConfig() const { return config_; }

  auto &getRecorder() { return recorder_.getRecorder(); }

  int getRunningSender() const { return running_cnt_; }

  void join() { jthreads_.clear(); }

private:
  BenchmarkConfig config_;

  int num_channel_;
  PreformanceRecorder recorder_;
  std::unique_ptr<brpc::Channel> channel_;
  brpc::ParallelChannel p_channel_;

  std::atomic<int> running_cnt_{0};

  std::vector<bthread_t> bids_;
  std::vector<std::jthread> jthreads_;
  std::vector<std::unique_ptr<Client>> clients_;
};

void testParallel(BenchmarkConfig config, int max_parallel = 32) {
  double up_payload_size = 16;
  double down_payload_size = 16;

  if (config.use_attachment) {
    up_payload_size += config.attachment_req_size;
    down_payload_size += config.attachment_resp_size;
  }
  if (config.use_proto_bytes) {
    up_payload_size += config.proto_req_data_size_byte;
    down_payload_size += config.proto_resp_data_size_byte;
  }
  if (config.use_streaming) {
    up_payload_size +=
        (config.stream_client_msg_size + 8ll) * config.stream_client_msg_num -
        8;
  }

  up_payload_size /= 1 << 20;
  down_payload_size /= 1 << 20;

  LOG(INFO) << fmt::format(
      "=> client_request_num: {} payload size   up:{:.2f}MB,  down: {:.2f}MB",
      config.client_request_num, up_payload_size, down_payload_size);
  for (auto parallel = 1; parallel <= max_parallel && !brpc::IsAskedToQuit();
       parallel <<= 1) {
    config.parallelism = parallel;
    g_stop = false;
    ClientBenchmarker tester(config);
    tester.init();

    tester.start(false);

    auto run_second = 2;
    std::this_thread::sleep_for(std::chrono::seconds(run_second));

    auto &recorder = tester.getRecorder();
    double sent_mb = tester.getSentBytes() * 1.0 / (1 << 20) / run_second;
    int64_t qps = recorder.qps(1);

    if (tester.getRunningSender() != config.parallelism) {
      LOG(INFO) << "WARNING! some sender might finished. running_sender: "
                << tester.getRunningSender()
                << "  parallelism: " << config.parallelism;
    }
    g_stop = true;

    while (tester.getRunningSender() && !brpc::IsAskedToQuit()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    tester.join();

    double recieved_mb =
        tester.getRecievedBytes() * 1.0 / (1 << 20) / run_second;
    LOG(INFO) << fmt::format(
        "   parallelism {}:   qps: {}, up/down_speed: {:.2f}MB/S {:.2f}MB/S",
        parallel, qps, sent_mb, recieved_mb);
  }
  LOG(INFO);
}

int main(int argc, char *argv[]) {
  auto config = parseCommandLine(argc, argv);

  LOG(INFO) << "use attachment:";
  config.use_attachment = true;
  testParallel(config);
  config.use_attachment = false;

  LOG(INFO) << "use proto:";
  config.use_proto_bytes = true;
  testParallel(config);
  config.use_proto_bytes = false;

  LOG(INFO) << "use streaming:";
  config.use_streaming = true;
  testParallel(config);
  config.use_streaming = false;

  return 0;
}
