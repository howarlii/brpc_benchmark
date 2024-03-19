#include <fmt/core.h>
#include <glog/logging.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <execution>
#include <memory>
#include <thread>

#include "config.h"
#include "proto/echo.pb.h"
#include "util.h"

class Client {
  struct ResponeInfo {
    bool has_sent{false};
    example::EchoResponse resp;
    std::vector<std::string> data;
  };

public:
  explicit Client(const BenchmarkConfig *config, PreformanceRecorder *recorder)
      : config_(config), recorder_(recorder) {}

  class OnDone : public google::protobuf::Closure {
  public:
    explicit OnDone(Client *c) : client(c) { ++client->running_rpc_count_; }
    ~OnDone() override { --client->running_rpc_count_; }

    void Run() override {
      std::unique_ptr<OnDone> self_guard(this);

      if (cntl.Failed()) {
        LOG(INFO) << "QWQ  rpc failed! " << cntl.ErrorText()
                  << " latency=" << cntl.latency_us();
        client->recorder_->error_count << 1;
        // We can't connect to the server, sleep a while. Notice that this
        // is a specific sleeping to prevent this thread from spinning too
        // fast. You should continue the business logic in a production
        // server rather than sleeping.
        return;
      }

      client->recorder_->latency_recorder << cntl.latency_us();
      for (int i = 0; i < cntl.sub_count(); ++i) {
        if (cntl.sub(i) && !cntl.sub(i)->Failed()) {
          *(client->recorder_->sub_channel_latency[i])
              << cntl.sub(i)->latency_us();
        }
      }

      if (!stream_datas.empty()) {
        sendStream();
      }

      if (resp_info->resp.has_attachment_size()) {
        resp_info->data.emplace_back(cntl.response_attachment().to_string());
        CHECK_EQ(resp_info->data.back().size(),
                 resp_info->resp.attachment_size())
            << fmt::format("QWQ respone attachment size not match! size: {}",
                           resp_info->data.back().size());
      }
    }

    void sendStream() {
      for (const auto &msg : stream_datas) {
        CHECK_EQ(0, brpc::StreamWrite(stream_id, msg));
      }
    }

    std::vector<butil::IOBuf> stream_datas;
    brpc::StreamId stream_id;
    Client *client;
    ResponeInfo *resp_info;
    brpc::Controller cntl;
  };

  void registeDoneCallBack(std::function<void()> func) {
    done_call_back_func_ = std::move(func);
  }

  void runReqBench(std::unique_ptr<example::EchoService_Stub> stub,
                   int reqs_num) {
    resp_infos_.resize(reqs_num);
    for (int seq_id = 0; seq_id < reqs_num && !brpc::IsAskedToQuit() && !g_stop;
         seq_id++) {
      // We will receive response synchronously, safe to put variables
      // on stack.
      example::EchoRequest request;
      uint64_t tot_bytes = 16;
      request.set_seq(seq_id);

      auto done_ptr = new OnDone(this);
      done_ptr->resp_info = &resp_infos_[seq_id];

      if (config_->use_streaming && config_->stream_client_msg_num > 0) {
        if (brpc::StreamCreate(&done_ptr->stream_id, done_ptr->cntl, nullptr) !=
            0) {
          LOG(ERROR) << "Fail to create stream";
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
          continue;
        }
        std::string data;
        for (auto msg_idx = 0; msg_idx < config_->stream_client_msg_num;
             msg_idx++) {
          generateRandomString(data, config_->stream_client_msg_size);
          done_ptr->stream_datas.emplace_back();
          done_ptr->stream_datas.back().append(std::move(data));
          request.mutable_hash()->Add(std::hash<std::string>{}(data));
        }
        request.set_streaming_size(config_->stream_client_msg_num *
                                   config_->stream_client_msg_size);
        tot_bytes +=
            config_->stream_client_msg_num * config_->stream_client_msg_size;
      }

      if (config_->use_proto_bytes && config_->proto_req_data_size_byte > 0) {
        request.set_proto_bytes_size(config_->proto_req_data_size_byte);
        auto data = request.mutable_data();
        generateRandomString(*data, config_->proto_req_data_size_byte);
        request.mutable_hash()->Add(std::hash<std::string>{}(*data));
        tot_bytes += config_->proto_req_data_size_byte;
      }

      if (config_->use_attachment && config_->attachment_req_size > 0) {
        request.set_attachment_size(config_->attachment_req_size);
        std::string data;
        generateRandomString(data, config_->attachment_req_size);
        request.mutable_hash()->Add(std::hash<std::string>{}(data));
        // request.mutable_hash()->Add(1); // error test
        done_ptr->cntl.request_attachment().append(std::move(data));
        tot_bytes += config_->attachment_req_size;
      }

      // Because `done'(last parameter) is NULL, this function waits until
      // the response comes back or error occurs(including timedout).
      done_ptr->resp_info->has_sent = true;
      stub->Echo(&done_ptr->cntl, &request, &done_ptr->resp_info->resp,
                 done_ptr);
      sent_bytes_ += tot_bytes;
    }

    while (running_rpc_count_) {
    }
    done_call_back_func_();

    checkResp();
  }

  void runRespBench(std::unique_ptr<example::EchoService_Stub> stub,
                    int reqs_num) {
    resp_infos_.resize(reqs_num);
    for (int seq_id = 0; seq_id < reqs_num && !brpc::IsAskedToQuit() && !g_stop;
         seq_id++) {
      // We will receive response synchronously, safe to put variables
      // on stack.
      example::EchoRequest request;
      uint64_t tot_bytes = 16;
      request.set_seq(seq_id);

      auto done_ptr = new OnDone(this);
      done_ptr->resp_info = &resp_infos_[seq_id];

      if (config_->use_streaming && config_->stream_client_msg_num > 0) {
        CHECK(false) << "Not support";
      }

      if (config_->use_proto_bytes && config_->proto_req_data_size_byte > 0) {
        request.set_proto_bytes_size(config_->proto_req_data_size_byte);
      }

      if (config_->use_attachment && config_->attachment_req_size > 0) {
        request.set_attachment_size(config_->attachment_req_size);
      }

      // Because `done'(last parameter) is NULL, this function waits until
      // the response comes back or error occurs(including timedout).
      done_ptr->resp_info->has_sent = true;
      stub->AskEcho(&done_ptr->cntl, &request, &done_ptr->resp_info->resp,
                    done_ptr);
      sent_bytes_ += tot_bytes;
    }

    while (running_rpc_count_) {
    }
    done_call_back_func_();

    checkResp();
  }

  uint64_t getSentBytes() const { return sent_bytes_; }
  uint64_t getRecievedBytes() const { return recieved_bytes_; }

private:
  void checkResp() {
    std::for_each(
        std::execution::par_unseq, resp_infos_.begin(), resp_infos_.end(),
        [&](const auto &resp_info) {
          if (!resp_info.has_sent)
            return;
          uint64_t tot_bytes = 16;
          auto response = resp_info.resp;

          if (!resp_info.data.empty()) {
            CHECK_EQ(resp_info.data.size(), response.hash_size())
                << fmt::format("QWQ response hash not match! data len: {} "
                               "hash_size: {}",
                               resp_info.data.size(), response.hash_size());
            for (int i = 0; i < resp_info.data.size(); i++) {
              tot_bytes += resp_info.data[i].size() + 8;
              if (std::hash<std::string>{}(resp_info.data[i]) !=
                  response.hash()[i]) {
                LOG(INFO) << fmt::format(
                    "QWQ response hash not match!  data len: {} hash: {}",
                    resp_info.data[i].size(), response.hash()[i]);
                exit(-1);
              }
            }
          } else if (response.has_proto_bytes_size()) {
            auto &data = response.data();
            tot_bytes += data.size();
            CHECK_EQ(response.data().size(), response.proto_bytes_size());
            CHECK_EQ(response.hash_size(), 1);
            if (std::hash<std::string>{}(data) != response.hash()[0]) {
              LOG(INFO) << fmt::format(
                  "QWQ response hash not match!  data len: {} hash: {}",
                  data.size(), response.hash()[0]);
              exit(-1);
            }
            recieved_bytes_ += tot_bytes;
          }
        });
  }

  std::atomic<int> running_rpc_count_{0};

  const BenchmarkConfig *config_;
  PreformanceRecorder *recorder_;

  std::vector<ResponeInfo> resp_infos_;
  std::function<void()> done_call_back_func_;

  std::atomic<uint64_t> sent_bytes_{0};
  std::atomic<uint64_t> recieved_bytes_{0};
};