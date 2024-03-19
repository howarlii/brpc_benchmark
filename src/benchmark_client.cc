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

#include <atomic>
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <thread>

#include "brpc_client_sync.h"
#include "config.h"
#include "proto/echo.pb.h"
#include "util.h"

class ClientBenchmarker {
 public:
  explicit ClientBenchmarker(BenchmarkConfig config = BenchmarkConfig()) : config_(std::move(config)) {}

  void init() {
    brpc::ChannelOptions channel_options;
    channel_options.protocol = config_.rpc_protocol;
    if (config_.use_streaming) {
      CHECK_EQ(config_.rpc_protocol, "baidu_std") << "RPC protocol must be baidu_std when enable streaming";
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
        if (sub_channel->Init(config_.server_addr.c_str(), config_.load_balancer.c_str(), &channel_options) != 0) {
          LOG(ERROR) << "Fail to initialize sub_channel[" << i << "]";
          exit(-1);
        }
        if (p_channel_.AddChannel(sub_channel, brpc::OWNS_CHANNEL, nullptr, nullptr) != 0) {
          LOG(ERROR) << "Fail to AddChannel, i=" << i;
          exit(-1);
        }
      }
      for (int i = 0; i < num_channel_; ++i) {
        recorder_.sub_channel_latency.emplace_back(std::make_unique<bvar::LatencyRecorder>());
        recorder_.sub_channel_latency.back()->expose(fmt::format("client_sub_{}", i));
      }
    } else {
      num_channel_ = 1;
      // LOG(INFO) << "use single channel, channel_num=" << num_channel_;
      channel_ = std::make_unique<brpc::Channel>();
      if (channel_->Init(config_.server_addr.c_str(), config_.load_balancer.c_str(), &channel_options) != 0) {
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
    auto benchmark_time = std::chrono::milliseconds(config_.benchmark_time);

    if (!config_.use_bthread) {
      for (auto i = 0; i < config_.parallelism; ++i) {
        clients_.emplace_back(std::make_unique<Client>(&config_, &recorder_));
        auto c = clients_.back().get();

        c->registeDoneCallBack([this]() { running_cnt_--; });
        running_cnt_++;
        if (is_req_bench) {
          jthreads_.emplace_back(
              [stub = get_stub(), c, benchmark_time]() mutable { c->runReqBench(std::move(stub), benchmark_time); });
        } else {
          jthreads_.emplace_back(
              [stub = get_stub(), c, benchmark_time]() mutable { c->runRespBench(std::move(stub), benchmark_time); });
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

  auto getSentBPS() const {
    double bytes{0};
    for (const auto &c : clients_) {
      bytes += c->getSentBPS();
    }
    return bytes;
  }
  auto getRecievedBPS() const {
    double bytes{0};
    for (const auto &c : clients_) {
      bytes += c->getRecievedBPS();
    }
    return bytes;
  }

  auto &getRecorder() const { return recorder_; }

  auto &getConfig() const { return config_; }

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

void BenchmarkThisConfigForParallel(const std::string &target_text, BenchmarkConfig config, int max_parallel = 64) {
  std::vector<int> parallelisms;
  std::vector<double> lantencys;
  std::vector<double> speeds;

  LOG(INFO) << fmt::format("{}  payload size {}", target_text, config.req_size);
  for (auto parallel = 1; parallel <= max_parallel && !brpc::IsAskedToQuit(); parallel <<= 1) {
    config.parallelism = parallel;

    ClientBenchmarker tester(config);
    tester.init();
    tester.start();

    while (tester.getRunningSender() && !brpc::IsAskedToQuit()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    tester.join();

    auto &recorder = tester.getRecorder();
    auto latency_ms = recorder.latency_recorder.latency_percentile(.99) / 1e3;
    int64_t qps = recorder.latency_recorder.qps();

    auto sent_mbps = tester.getSentBPS() / (1 << 20);
    // auto recieved_mbps = tester.getRecievedBPS() / (1 << 20);

    LOG(INFO) << fmt::format(
        "   parallelism {:2d}:   lantency: {:4.2f},  qps: {:4d} "
        "speed: {:4.2f}MB/S ",
        parallel, latency_ms, qps, sent_mbps);

    parallelisms.emplace_back(parallel);
    lantencys.emplace_back(latency_ms);
    speeds.emplace_back(sent_mbps);
  }
  LOG(INFO);

  {
    std::string prefix;
    std::string suffix;
    if (config.req_size >= (1 << 20)) {
      prefix = fmt::format("{}_", target_text);
      suffix = fmt::format("[\"{}m\"]", config.req_size >> 20);
    } else if (config.req_size >= (1 << 10)) {
      prefix = fmt::format("{}_", target_text);
      suffix = fmt::format("[\"{}k\"]", config.req_size >> 10);
    } else {
      prefix = fmt::format("{}_", target_text);
      suffix = fmt::format("[\"{}m\"]", config.req_size);
    }

    std::ofstream outfile("result.txt", std::ios::app);
    CHECK(outfile.is_open());

    outfile << fmt::format("# {}  payload size {}\n", target_text, config.req_size);

    // outfile << fmt::format("{}parallelisms{} = ", prefix, suffix);
    // StreamOutPythonArray(outfile, parallelisms);
    outfile << fmt::format("{}lantencys{} = ", prefix, suffix);
    StreamOutPythonArray(outfile, lantencys);
    outfile << fmt::format("{}speed{} = ", prefix, suffix);
    StreamOutPythonArray(outfile, speeds);
    outfile << std::endl;

    // Close the file
    outfile.close();
  }
}

int main(int argc, char *argv[]) {
  auto config = parseCommandLine(argc, argv);

  config.use_attachment = true;
  BenchmarkThisConfigForParallel("attachment", config);
  config.use_attachment = false;

  config.use_proto_bytes = true;
  BenchmarkThisConfigForParallel("proto", config);
  config.use_proto_bytes = false;

  config.use_streaming = true;
  BenchmarkThisConfigForParallel("streaming", config);
  config.use_streaming = false;

  return 0;
}
