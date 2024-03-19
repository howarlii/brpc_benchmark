#pragma once

#include <brpc/server.h>
#include <fmt/core.h>
#include <glog/logging.h>

#include <cstdint>
#include <limits>
#include <random>

struct PreformanceRecorder {
  void output() {
    LOG(INFO) << "Sending EchoRequest at qps=" << latency_recorder.qps(1)
              << " latency=" << latency_recorder.latency(1);

    for (int i = 0; i < sub_channel_latency.size(); ++i) {
      LOG(INFO) << "sub_channel latency_" << i << "="
                << sub_channel_latency[i]->latency(1);
    }
  }

  auto &getRecorder() const { return latency_recorder; }

  bvar::LatencyRecorder latency_recorder{"client"};
  bvar::Adder<int> error_count{"client_error_count"};
  std::vector<std::unique_ptr<bvar::LatencyRecorder>> sub_channel_latency;
};

template <typename AppendableT>
void generateRandomString(AppendableT &str, int length) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<uint64_t> dis_i64(
      0, std::numeric_limits<uint64_t>::max());
  std::uniform_int_distribution<uint8_t> dis(0, 255);
  str.clear();
  str.reserve(length);
  int i = 0;
  for (; i + 8 <= length; i += 8) {
    auto v = dis_i64(gen);
    str.append(reinterpret_cast<const char *>(&v), 8);
  }
  for (; i < length; ++i) {
    auto v = dis(gen);
    str.append(reinterpret_cast<const char *>(&v), 1);
  }
  assert(str.size() == length);
};
