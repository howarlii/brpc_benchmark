./build/src/brpc_test_server -max_body_size=2147483647 -socket_max_unwritten_bytes=2147483647 &

sleep 1

# ./build/src/brpc_test_client -req_size=1024 -benchmark_time=2000 -for_parallelism=true -max_body_size=2147483647 -socket_max_unwritten_bytes=2147483647
# ./build/src/brpc_test_client -req_size=10240 -benchmark_time=2000 -for_parallelism=true -max_body_size=2147483647 -socket_max_unwritten_bytes=2147483647
# ./build/src/brpc_test_client -req_size=102400 -benchmark_time=2000 -for_parallelism=true -max_body_size=2147483647 -socket_max_unwritten_bytes=2147483647
# ./build/src/brpc_test_client -req_size=1048576 -benchmark_time=2000 -for_parallelism=true -max_body_size=2147483647 -socket_max_unwritten_bytes=2147483647
# ./build/src/brpc_test_client -req_size=10485760 -benchmark_time=2000 -for_parallelism=true -max_body_size=536870912 -socket_max_unwritten_bytes=2147483647

./build/src/brpc_test_client -req_size=536870912 -benchmark_time=4000 -for_req_size=true -parallelism=1 -max_body_size=2147483647 -socket_max_unwritten_bytes=2147483647

# ./build/src/brpc_test_client -req_size=1048576 -benchmark_time=4000 -for_streaming_size=true -parallelism=1 -max_body_size=2147483647 -socket_max_unwritten_bytes=2147483647

pkill brpc_test
