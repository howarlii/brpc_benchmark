./build/src/brpc_test_server -max_body_size=2147483647 -socket_max_unwritten_bytes=2147483647 &

sleep 1

function benchmark_brpc {
    ./script/iperf3_test.sh

    # ./build/src/brpc_test_client -req_size=268435456 -benchmark_time=4000 -for_req_size=true -parallelism=1 -max_body_size=2147483647 -socket_max_unwritten_bytes=2147483647
    # ./build/src/brpc_test_client -req_size=268435456 -benchmark_time=4000 -for_req_size=true -parallelism=2 -max_body_size=2147483647 -socket_max_unwritten_bytes=2147483647
    # ./build/src/brpc_test_client -req_size=268435456 -benchmark_time=4000 -for_req_size=true -parallelism=4 -max_body_size=2147483647 -socket_max_unwritten_bytes=2147483647

    # ./build/src/brpc_test_client -req_size=268435456 -benchmark_time=4000 -for_streaming_size=true -parallelism=1 -max_body_size=2147483647 -socket_max_unwritten_bytes=2147483647
}

# Benchmark under 0ms delay
tc qdisc show

benchmark_brpc
mv result/*.csv result/0ms/

# Benchmark under 1ms delay
tc qdisc add dev lo root netem delay 1ms
tc qdisc show

benchmark_brpc
mv result/*.csv result/1ms/

tc qdisc del dev lo root netem delay 1ms

# Benchmark under 10ms delay
tc qdisc add dev lo root netem delay 10ms
tc qdisc show

benchmark_brpc
mv result/*.csv result/10ms/

tc qdisc del dev lo root netem delay 10ms

# clean resource
pkill brpc_test

tc qdisc show
