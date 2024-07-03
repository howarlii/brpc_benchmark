#include <memory>

#include <brpc/channel.h>
#include <brpc/controller.h>
#include <brpc/server.h>
#include <brpc/stream.h>
#include <butil/logging.h>
#include <butil/string_printf.h>

#include "proto/echo.pb.h"

class RepeaterStreamReceiver : public brpc::StreamInputHandler {
 public:
  int on_received_messages(brpc::StreamId id, butil::IOBuf *const messages[], size_t size) final {
    butil::IOBuf buf;
    int t = 1;
    buf.append(&t, sizeof(t));
    brpc::StreamWrite(id, buf);
    LOG(INFO) << "send stream";
    return 0;
  }

  void on_idle_timeout(brpc::StreamId id) final {}

  void on_closed(brpc::StreamId id) final { std::unique_ptr<RepeaterStreamReceiver> self_guard(this); }
};

// Your implementation of example::EchoService
class StreamTestServiceImpl : public example::StreamTestService {
 public:
  void Rpc(google::protobuf::RpcController *cntl_base, const example::EmptyMsg *request, example::EmptyMsg *response,
           google::protobuf::Closure *done) final {
    brpc::ClosureGuard done_guard(done);
  }

  void StreamRpc(google::protobuf::RpcController *cntl_base, const example::EmptyMsg *request,
                 example::EmptyMsg *response, google::protobuf::Closure *done) override {
    auto *cntl = static_cast<brpc::Controller *>(cntl_base);
    brpc::ClosureGuard done_guard(done);

    brpc::StreamId stream_id;
    // accept the stream
    brpc::StreamOptions stream_options;
    stream_options.handler = new RepeaterStreamReceiver();
    if (brpc::StreamAccept(&stream_id, *cntl, &stream_options) != 0) {
      cntl->SetFailed("Failed to accept stream");
      LOG(ERROR) << "Cannot accept stream";
      return;
    }
  }
};

class RPCTestServiceImpl : public example::RPCTestService {
 public:
  void Rpc(google::protobuf::RpcController *cntl_base, const example::EmptyMsg *request, example::EmptyMsg *response,
           google::protobuf::Closure *done) final {
    brpc::ClosureGuard done_guard(done);
  }

  void StreamRpc(google::protobuf::RpcController *cntl_base, const example::EmptyMsg *request,
                 example::EmptyMsg *response, google::protobuf::Closure *done) override {
    auto *cntl = static_cast<brpc::Controller *>(cntl_base);
    brpc::ClosureGuard done_guard(done);

    brpc::StreamId stream_id;
    // accept the stream
    brpc::StreamOptions stream_options;
    stream_options.handler = new RepeaterStreamReceiver();
    if (brpc::StreamAccept(&stream_id, *cntl, &stream_options) != 0) {
      cntl->SetFailed("Failed to accept stream");
      LOG(ERROR) << "Cannot accept stream";
      return;
    }
  }
};

class StreamTester {
 public:
  static constexpr int kPort1 = 12306;
  static constexpr int kPort2 = 12307;

  StreamTester(bool use_same_server) {
    if (server_.AddService(&service_stream_, brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
      LOG(ERROR) << "Fail to add service";
      return;
    }
    if (server_.AddService(&service_rpc_, brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
      LOG(ERROR) << "Fail to add service";
      return;
    }
    brpc::ServerOptions options;
    if (server_.Start(kPort1, &options) != 0) {
      LOG(ERROR) << "Fail to start EchoServer";
      return;
    }

    if (server2_.AddService(&service_rpc_, brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
      LOG(ERROR) << "Fail to add service";
      return;
    }
    if (server2_.Start(kPort2, &options) != 0) {
      LOG(ERROR) << "Fail to start EchoServer";
      return;
    }

    initChannel(use_same_server);
  }

  void initChannel(bool use_same_server) {
    brpc::ChannelOptions channel_options;
    channel_options.protocol = "baidu_std";
    std::string addr1 = "127.0.0.1:" + std::to_string(kPort1);

    channel_stream_ = std::make_unique<brpc::Channel>();
    if (channel_stream_->Init(addr1.c_str(), "", &channel_options) != 0) {
      LOG(ERROR) << "Fail to initialize channel";
      exit(-1);
    }
    stub_stream_ = std::make_unique<example::StreamTestService_Stub>(channel_stream_.get());

    std::string addr2 = use_same_server ? addr1 : "127.0.0.1:" + std::to_string(kPort2);
    channel_rpc_ = std::make_unique<brpc::Channel>();
    if (channel_rpc_->Init(addr2.c_str(), "", &channel_options) != 0) {
      LOG(ERROR) << "Fail to initialize channel";
      exit(-1);
    }
    stub_rpc_ = std::make_unique<example::RPCTestService_Stub>(channel_rpc_.get());
  }

  void sendRpc() {
    brpc::Controller cntl;
    example::EmptyMsg req;
    example::EmptyMsg resp;

    stub_rpc_->Rpc(&cntl, &req, &resp, nullptr);
    if (cntl.Failed()) {
      LOG(ERROR) << "Fail to send rpc " << cntl.ErrorText();
    }
    LOG(INFO) << "send rpc";
  }

  void StartStreamRpc() {
    brpc::Controller cntl;
    example::EmptyMsg req;
    example::EmptyMsg resp;

    brpc::StreamOptions stream_options;
    stream_options.handler = new RepeaterStreamReceiver();
    if (brpc::StreamCreate(&stream_id_, cntl, &stream_options) != 0) {
      LOG(ERROR) << "Failed to create stream";
      exit(-1);
    }

    stub_stream_->StreamRpc(&cntl, &req, &resp, nullptr);
    if (cntl.Failed()) {
      LOG(ERROR) << "Fail to send rpc " << cntl.ErrorText();
    }

    butil::IOBuf buf;
    int t = 1;
    buf.append(&t, sizeof(t));
    brpc::StreamWrite(stream_id_, buf);
  }

  void stopStreamRpc() {
    brpc::StreamClose(stream_id_);
    exit(-1);
  }

 private:
  brpc::Server server_, server2_;
  StreamTestServiceImpl service_stream_;
  RPCTestServiceImpl service_rpc_;
  std::unique_ptr<brpc::Channel> channel_stream_, channel_rpc_;
  std::unique_ptr<example::StreamTestService_Stub> stub_stream_;
  std::unique_ptr<example::RPCTestService_Stub> stub_rpc_;
  brpc::StreamId stream_id_;
};

int main(int argc, char *argv[]) {
  // StreamTester tester(true); // FAIL
  StreamTester tester(false);

  tester.StartStreamRpc();

  for (int i = 0; i < 10; ++i) {
    tester.sendRpc();
  }

  tester.stopStreamRpc();

  return 0;
}