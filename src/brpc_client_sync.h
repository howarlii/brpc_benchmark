#include <brpc/stream.h>
#include <fmt/core.h>
#include <glog/logging.h>
#include <functional>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <execution>
#include <memory>
#include <stdexcept>
#include <thread>

#include "config.h"
#include "proto/echo.pb.h"
#include "util.h"

class Client {
  static constexpr auto kSendInterval = std::chrono::milliseconds(200);
  static constexpr auto kDefaultBenchmarkTime = std::chrono::milliseconds(1000);
  static constexpr auto kBaseProroSize = 12;

  struct ResponeInfo {
    bool has_sent{false};
    example::EchoResponse resp;
    std::vector<std::string> data;
  };

 public:
  explicit Client(const BenchmarkConfig *config, PreformanceRecorder *recorder)
      : config_(config), recorder_(recorder), req_size_(config_->req_size) {}

  void registeDoneCallBack(std::function<void()> func) { done_call_back_func_ = std::move(func); }

  void runReqBench(std::unique_ptr<example::EchoService_Stub> stub,
                   std::chrono::milliseconds benchmark_time = kDefaultBenchmarkTime) {
    sent_bytes_ = 0;
    stub_ = std::move(stub);
    generate_data(req_size_);

    auto start = std::chrono::steady_clock::now();
    for (int seq_id = 0; (std::chrono::steady_clock::now() - start) < benchmark_time && !brpc::IsAskedToQuit();
         seq_id++) {
      const auto &[data, data_hash] = send_datas_[seq_id % send_datas_.size()];
      brpc::Controller cntl;
      brpc::StreamId stream_id;

      example::EchoRequest request;
      request.set_hash(data_hash);

      if (config_->use_streaming) {
        if (brpc::StreamCreate(&stream_id, cntl, nullptr) != 0) {
          LOG(ERROR) << "Fail to create stream";
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
          continue;
        }
        request.set_streaming_size(req_size_);
        sent_bytes_ += req_size_;
      }

      if (config_->use_proto_bytes) {
        request.set_proto_bytes_size(req_size_);
        request.set_data(data);
        sent_bytes_ += req_size_;
      }

      if (config_->use_attachment) {
        request.set_attachment_size(req_size_);
        cntl.request_attachment().append(data);
        sent_bytes_ += req_size_;
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(1));

      example::EchoResponse resp;
      stub_->Echo(&cntl, &request, &resp, nullptr);
      sent_bytes_ += kBaseProroSize;

      if (cntl.Failed()) {
        LOG(INFO) << "QWQ  rpc failed! " << cntl.ErrorText() << " latency=" << cntl.latency_us();
        recorder_->error_count << 1;
        // We can't connect to the server, sleep a while. Notice that this
        // is a specific sleeping to prevent this thread from spinning too
        // fast. You should continue the business logic in a production
        // server rather than sleeping.
        return;
      }

      recorder_->latency_recorder << cntl.latency_us();
      for (int i = 0; i < cntl.sub_count(); ++i) {
        if (cntl.sub(i) && !cntl.sub(i)->Failed()) {
          *(recorder_->sub_channel_latency[i]) << cntl.sub(i)->latency_us();
        }
      }

      if (config_->use_streaming) {
        butil::IOBuf stream_data;
        for (auto tot_size = 0; tot_size < req_size_; tot_size += config_->stream_client_msg_size) {
          auto size = std::min(config_->stream_client_msg_size, req_size_ - tot_size);

          stream_data.append(data.data() + tot_size, size);
          CHECK_EQ(0, brpc::StreamWrite(stream_id, stream_data));
          stream_data.clear();
        }
        brpc::StreamClose(stream_id);
      }
    }
    real_benchmark_time_S_ =
        (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start)).count() / 1e6;

    done_call_back_func_();
  }

  void runRespBench(std::unique_ptr<example::EchoService_Stub> stub,
                    std::chrono::milliseconds benchmark_time = kDefaultBenchmarkTime) {
    std::runtime_error("Not implemented");
  }

  uint64_t getSentBytes() const { return sent_bytes_; }
  uint64_t getRecievedBytes() const { return recieved_bytes_; }

  double getSentBPS() const { return sent_bytes_ / real_benchmark_time_S_; }
  double getRecievedBPS() const { return recieved_bytes_ / real_benchmark_time_S_; }

 private:
  void generate_data(int data_size, int num_data = 10) {
    send_datas_.resize(num_data);
    for (auto &[data, hash] : send_datas_) {
      generateRandomString(data, data_size);
      hash = std::hash<std::string>()(data);
    }
  }

  const BenchmarkConfig *config_;
  PreformanceRecorder *recorder_;
  std::unique_ptr<example::EchoService_Stub> stub_;

  const int req_size_;
  std::vector<std::pair<std::string, uint64_t>> send_datas_;

  std::function<void()> done_call_back_func_;

  uint64_t sent_bytes_{0};
  uint64_t recieved_bytes_{0};
  double real_benchmark_time_S_;
};