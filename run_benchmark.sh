#!/bin/bash

delays=(0 1 10)

./build/src/brpc_test_server -max_body_size=2147483647 -socket_max_unwritten_bytes=2147483647 -h2_client_stream_window_size=2147483647 -h2_client_connection_window_size=2147483647 &

if [ $? -ne 0 ]; then
    echo "Error: Last command failed with a non-zero exit status. Exiting..."
    exit 1
fi

sleep 1

for delay in "${delays[@]}"; do
    tc qdisc add dev lo root netem delay ${delay}ms
    tc qdisc show

    ./script/iperf3_test.sh

    ./build/src/brpc_test_client -test_attachment=false -test_proto=false -req_size=268435456 -benchmark_time=4000 -for_req_size=true -parallelism=1 -max_body_size=2147483647 -socket_max_unwritten_bytes=2147483647
    ./build/src/brpc_test_client -test_attachment=false -test_proto=false -req_size=268435456 -benchmark_time=4000 -for_req_size=true -parallelism=2 -max_body_size=2147483647 -socket_max_unwritten_bytes=2147483647
    ./build/src/brpc_test_client -test_attachment=false -test_proto=false -req_size=268435456 -benchmark_time=4000 -for_req_size=true -parallelism=4 -max_body_size=2147483647 -socket_max_unwritten_bytes=2147483647
    ./build/src/brpc_test_client -test_attachment=false -test_proto=false -req_size=268435456 -benchmark_time=4000 -for_req_size=true -parallelism=6 -max_body_size=2147483647 -socket_max_unwritten_bytes=2147483647
    ./build/src/brpc_test_client -test_attachment=false -test_proto=false -req_size=268435456 -benchmark_time=4000 -for_req_size=true -parallelism=8 -max_body_size=2147483647 -socket_max_unwritten_bytes=2147483647

    # ./build/src/brpc_test_client -req_size=268435456 -test_attachment=false -test_cstreaming=false -benchmark_time=4000 -for_req_size=true -parallelism=1 -h2_client_stream_window_size=2147483647 -h2_client_connection_window_size=2147483647 -max_body_size=2147483647 -socket_max_unwritten_bytes=2147483647
    ./build/src/brpc_test_client -req_size=268435456 -rpc_protocol=h2:grpc -test_attachment=false -test_cstreaming=false -benchmark_time=4000 -for_req_size=true -parallelism=1 -h2_client_stream_window_size=2147483647 -h2_client_connection_window_size=2147483647 -max_body_size=2147483647 -socket_max_unwritten_bytes=2147483647
    ./build/src/brpc_test_client -req_size=268435456 -rpc_protocol=hulu_pbrpc -test_attachment=false -test_cstreaming=false -benchmark_time=4000 -for_req_size=true -parallelism=1 -h2_client_stream_window_size=2147483647 -h2_client_connection_window_size=2147483647 -max_body_size=2147483647 -socket_max_unwritten_bytes=2147483647

    ./build/src/brpc_test_client -continue_stream_messages_in_batch=1 -continue_stream_max_buf_size=2097152 -test_attachment=false -test_proto=false -req_size=268435456 -benchmark_time=4000 -for_req_size=true -parallelism=1 -max_body_size=2147483647 -socket_max_unwritten_bytes=2147483647
    ./build/src/brpc_test_client -continue_stream_messages_in_batch=1024 -continue_stream_max_buf_size=2097152 -test_attachment=false -test_proto=false -req_size=268435456 -benchmark_time=4000 -for_req_size=true -parallelism=1 -max_body_size=2147483647 -socket_max_unwritten_bytes=2147483647

    ./build/src/brpc_test_client -continue_stream_messages_in_batch=1 -continue_stream_max_buf_size=33554432 -test_attachment=false -test_proto=false -req_size=268435456 -benchmark_time=4000 -for_req_size=true -parallelism=1 -max_body_size=2147483647 -socket_max_unwritten_bytes=2147483647
    ./build/src/brpc_test_client -continue_stream_messages_in_batch=128 -continue_stream_max_buf_size=33554432 -test_attachment=false -test_proto=false -req_size=268435456 -benchmark_time=4000 -for_req_size=true -parallelism=1 -max_body_size=2147483647 -socket_max_unwritten_bytes=2147483647
    ./build/src/brpc_test_client -continue_stream_messages_in_batch=1024 -continue_stream_max_buf_size=33554432 -test_attachment=false -test_proto=false -req_size=268435456 -benchmark_time=4000 -for_req_size=true -parallelism=1 -max_body_size=2147483647 -socket_max_unwritten_bytes=2147483647

    ./build/src/brpc_test_client -continue_stream_messages_in_batch=128 -continue_stream_max_buf_size=8388608 -test_attachment=false -test_proto=false -req_size=268435456 -benchmark_time=4000 -for_req_size=true -parallelism=1 -max_body_size=2147483647 -socket_max_unwritten_bytes=2147483647
    ./build/src/brpc_test_client -continue_stream_messages_in_batch=128 -continue_stream_max_buf_size=16777216 -test_attachment=false -test_proto=false -req_size=268435456 -benchmark_time=4000 -for_req_size=true -parallelism=1 -max_body_size=2147483647 -socket_max_unwritten_bytes=2147483647
    ./build/src/brpc_test_client -continue_stream_messages_in_batch=128 -continue_stream_max_buf_size=33554432 -test_attachment=false -test_proto=false -req_size=268435456 -benchmark_time=4000 -for_req_size=true -parallelism=1 -max_body_size=2147483647 -socket_max_unwritten_bytes=2147483647
    ./build/src/brpc_test_client -continue_stream_messages_in_batch=128 -continue_stream_max_buf_size=67108864 -test_attachment=false -test_proto=false -req_size=268435456 -benchmark_time=4000 -for_req_size=true -parallelism=1 -max_body_size=2147483647 -socket_max_unwritten_bytes=2147483647

    # ./build/src/brpc_test_client -continue_stream_messages_in_batch=128 -continue_stream_max_buf_size=33554432 -test_attachment=false -test_proto=false -req_size=268435456 -benchmark_time=4000 -for_req_size=true -parallelism=1 -max_body_size=2147483647 -socket_max_unwritten_bytes=2147483647
    ./build/src/brpc_test_client -continue_stream_messages_in_batch=128 -continue_stream_max_buf_size=33554432 -test_attachment=false -test_proto=false -req_size=268435456 -benchmark_time=4000 -for_req_size=true -parallelism=2 -max_body_size=2147483647 -socket_max_unwritten_bytes=2147483647
    ./build/src/brpc_test_client -continue_stream_messages_in_batch=128 -continue_stream_max_buf_size=33554432 -test_attachment=false -test_proto=false -req_size=268435456 -benchmark_time=4000 -for_req_size=true -parallelism=4 -max_body_size=2147483647 -socket_max_unwritten_bytes=2147483647

    mv result/*.csv result/${delay}ms/

    tc qdisc del dev lo root netem delay ${delay}ms
done

# clean resource
pkill brpc_test

tc qdisc show
