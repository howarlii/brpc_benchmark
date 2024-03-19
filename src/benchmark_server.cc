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

// A server to receive EchoRequest and send back EchoResponse.

#include <brpc/server.h>
#include <brpc/stream.h>
#include <butil/logging.h>
#include <butil/string_printf.h>
#include <gflags/gflags.h>

#include <utility>

#include "config.h"
#include "proto/echo.pb.h"
#include "util.h"

class StreamReceiver : public brpc::StreamInputHandler {
public:
  explicit StreamReceiver(example::EchoRequest request)
      : request_(std::move(request)) {
    data_.reserve(request_.streaming_size());
  }

  int on_received_messages(brpc::StreamId id, butil::IOBuf *const messages[],
                           size_t size) final {

    for (size_t i = 0; i < size; ++i, seq_++) {
      data_.append(messages[i]->to_string());
    }
    // LOG(INFO) << "Received from Stream=" << id << ": " << os.str();
    return 0;
  }

  void on_idle_timeout(brpc::StreamId id) final {
    LOG(INFO) << "Stream=" << id << " has no data transmission for a while";
  }

  void on_closed(brpc::StreamId id) final {
    std::unique_ptr<StreamReceiver> self_guard(this);
    CHECK_EQ(data_.size(), request_.streaming_size()) << fmt::format(
        "QWQ request data size not match!  real len: {}, expect length:{}",
        data_.size(), request_.streaming_size());
    if (std::hash<std::string>{}(data_) != request_.hash()) {
      LOG(INFO) << fmt::format(
          "QWQ request data hash not match!  data len: {} hash: {}",
          request_.streaming_size(), request_.hash());
    }
  }

private:
  std::string data_;
  int seq_{0};
  // brpc::StreamId stream_id_{brpc::INVALID_STREAM_ID};
  const example::EchoRequest request_;
};

// Your implementation of example::EchoService
class EchoServiceImpl : public example::EchoService {
public:
  explicit EchoServiceImpl(BenchmarkConfig config)
      : config_(std::move(config)) {}

  void Echo(google::protobuf::RpcController *cntl_base,
            const example::EchoRequest *request,
            example::EchoResponse *response,
            google::protobuf::Closure *done) override {
    brpc::ClosureGuard done_guard(done);
    auto *cntl = static_cast<brpc::Controller *>(cntl_base);

    // ==== check request
    if (request->has_proto_bytes_size()) {
      CHECK_EQ(std::hash<std::string>{}(request->data()), request->hash())
          << fmt::format(
                 "QWQ request data hash not match!  data len: {} hash: {}",
                 request->data().size(), request->hash());
    }

    if (request->has_streaming_size()) {
      brpc::StreamOptions stream_options;
      auto receiver = new StreamReceiver(*request);
      stream_options.handler = receiver;

      brpc::StreamId stream_id{brpc::INVALID_STREAM_ID};
      if (brpc::StreamAccept(&stream_id, *cntl, &stream_options) != 0) {
        cntl->SetFailed("Fail to accept stream");
        return;
      }
    }

    if (request->has_attachment_size()) {
      // LOG(INFO) << fmt::format("revicec attachment size: {}",
      // cntl->request_attachment().size());
      CHECK_EQ(std::hash<std::string>{}(cntl->request_attachment().to_string()),
               request->hash())
          << fmt::format("QWQ request attachment size: {}",
                         cntl->request_attachment().size());
    }

    // Fill in respone
    response->set_hash(123);
  }

  void AskEcho(google::protobuf::RpcController *cntl_base,
               const example::EchoRequest *request,
               example::EchoResponse *response,
               google::protobuf::Closure *done) override {
    brpc::ClosureGuard done_guard(done);
    auto *cntl = static_cast<brpc::Controller *>(cntl_base);

    // ===== send respone
    if (request->has_proto_bytes_size()) {
      auto size = request->proto_bytes_size();
      response->set_proto_bytes_size(size);
      auto data = response->mutable_data();
      generateRandomString(*data, size);
      response->set_hash(std::hash<std::string>{}(*data));
    } else if (request->has_attachment_size()) {
      auto size = request->attachment_size();
      response->set_attachment_size(size);
      std::string data;
      generateRandomString(data, size);
      cntl->response_attachment().append(std::move(data));
      response->set_hash(std::hash<std::string>{}(data));
    } else {
      CHECK(false) << "unsupport request type";
    }
  }

private:
  BenchmarkConfig config_;
};

class Server {
public:
  explicit Server(BenchmarkConfig config = BenchmarkConfig())
      : config_(std::move(config)), echo_service_impl_(config_) {}

  void init() {
    if (server_.AddService(&echo_service_impl_,
                           brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
      LOG(ERROR) << "Fail to add service";
      exit(-1);
    }

    // Start the server_.
    brpc::ServerOptions options;

    if (server_.Start(config_.brpc_port, &options) != 0) {
      LOG(ERROR) << "Fail to start EchoServer";
      exit(-1);
    }
  }

  void join() { server_.RunUntilAskedToQuit(); }

private:
  BenchmarkConfig config_;
  // Generally you only need one Server.
  brpc::Server server_;

  // Instance of your service.
  EchoServiceImpl echo_service_impl_;
};

int main(int argc, char *argv[]) {
  auto config = parseCommandLine(argc, argv);

  Server s1(config);
  s1.init();

  s1.join();

  return 0;
}