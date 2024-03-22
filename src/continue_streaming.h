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

class ContinueStreamingSender {
  static constexpr auto kBaseProroSize = 12;

 public:
  explicit ContinueStreamingSender(example::EchoService_Stub *stub, int msg_size) : stub_(stub), msg_size_(msg_size) {}

  auto getSentBytes() const { return sent_bytes_; }

  auto getRealBenchTime() const { return real_benchmark_time_s_; }

  void run_continue_streaming(std::chrono::milliseconds benchmark_time) {
    std::string data;
    generateRandomString(data, msg_size_);
    auto data_hash = std::hash<std::string>()(data);

    brpc::Controller cntl;
    brpc::StreamId stream_id;
    example::EchoRequest request;
    request.set_hash(data_hash);
    request.set_continue_streaming_size(msg_size_);

    brpc::StreamOptions stream_options;
    auto handler = std::make_unique<StreamHandler>(&recv_seq_id_);
    stream_options.handler = handler.get();
    stream_options.messages_in_batch = 1;
    // stream_options.max_buf_size = 1 << 30;
    if (brpc::StreamCreate(&stream_id, cntl, &stream_options) != 0) {
      LOG(WARNING) << "Fail to create stream";
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      return;
    }

    example::EchoResponse resp;
    stub_->Echo(&cntl, &request, &resp, nullptr);
    sent_bytes_ += kBaseProroSize;

    if (cntl.Failed()) {
      LOG(WARNING) << "QWQ continue streaming rpc failed! " << cntl.ErrorText() << " latency=" << cntl.latency_us();
      // We can't connect to the server, sleep a while. Notice that this
      // is a specific sleeping to prevent this thread from spinning too
      // fast. You should continue the business logic in a production
      // server rather than sleeping.
      return;
    }

    butil::IOBuf stream_data;
    stream_data.append(data);
    auto start = std::chrono::steady_clock::now();

    for (; (std::chrono::steady_clock::now() - start) < benchmark_time && !brpc::IsAskedToQuit();) {
      while (auto ret = brpc::StreamWrite(stream_id, stream_data)) {
        if (ret == EAGAIN) {
          CHECK(brpc::StreamWait(stream_id, nullptr) == 0);
          continue;
        } else {
          CHECK_EQ(0, 1) << "Fail to write stream";
        }
      }
      CHECK(stream_data.size() == msg_size_);
    }
    while (recv_seq_id_ == 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    sent_bytes_ += msg_size_ * recv_seq_id_;
    real_benchmark_time_s_ =
        (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start)).count() / 1e6;
    brpc::StreamClose(stream_id);
    handler->is_closed.wait(false);
  }

 private:
  class StreamHandler : public brpc::StreamInputHandler {
   public:
    explicit StreamHandler(std::atomic<int> *seq_id) : seq_id_(seq_id) {}

    int on_received_messages(brpc::StreamId id, butil::IOBuf *const messages[], size_t size) final {
      // LOG(INFO) << "Received from Stream=" << id << ": seq: " << seq_id_ << " size: " << size;
      auto last_msg = messages[size - 1];
      if (last_msg->size() != sizeof(int)) return 1;
      int seq_id = *reinterpret_cast<const int *>(last_msg->to_string().data());
      CHECK_GT(seq_id, *seq_id_);
      *seq_id_ = seq_id;
      return 0;
    }

    void on_idle_timeout(brpc::StreamId id) final {
      LOG(INFO) << "Stream=" << id << " has no data transmission for a while";
    }

    void on_closed(brpc::StreamId id) final {
      is_closed = true;
      is_closed.notify_one();
    }

    std::atomic<bool> is_closed{false};

   private:
    std::atomic<int> *seq_id_;
  };

  example::EchoService_Stub *stub_;

  const int msg_size_;
  std::atomic<int> recv_seq_id_{-1};

  uint64_t sent_bytes_{0};
  double real_benchmark_time_s_;
};

class ContinueStreamRecvHandler : public brpc::StreamInputHandler {
 public:
  explicit ContinueStreamRecvHandler(example::EchoRequest request) : request_(std::move(request)) {}

  int on_received_messages(brpc::StreamId id, butil::IOBuf *const messages[], size_t size) final {
    for (size_t i = 0; i < size; ++i) {
      auto data = messages[i]->to_string();
      if (data.size() != request_.continue_streaming_size()) {
        LOG(WARNING) << fmt::format("QWQ request data size not match!  real len: {}, expect length:{}", data.size(),
                                    request_.continue_streaming_size());
      }
      if (std::hash<std::string>{}(data) != request_.hash()) {
        LOG(WARNING) << fmt::format("QWQ request data hash not match!  data len: {} hash: {}",
                                    request_.continue_streaming_size(), request_.hash());
      }
      seq_id_++;
    }

    // LOG(INFO) << "Received from Stream=" << id << ": seq: " << seq_id_ << " size: " << size;
    butil::IOBuf stream_data;
    stream_data.append(reinterpret_cast<const char *>(&seq_id_), sizeof(int));
    brpc::StreamWrite(id, stream_data);

    return 0;
  }

  void on_idle_timeout(brpc::StreamId id) final {
    LOG(INFO) << "Stream=" << id << " has no data transmission for a while";
  }

  void on_closed(brpc::StreamId id) final { std::unique_ptr<ContinueStreamRecvHandler> self_guard(this); }

 private:
  const example::EchoRequest request_;

  int seq_id_{-1};
};
