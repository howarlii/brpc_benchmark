./build/src/brpc_test_client -req_size=10240 -benchmark_time=2000 -for_parallelism=true -max_body_size=1073741824 -socket_max_unwritten_bytes=1073741824
./build/src/brpc_test_client -req_size=102400 -benchmark_time=2000 -for_parallelism=true -max_body_size=1073741824 -socket_max_unwritten_bytes=1073741824
./build/src/brpc_test_client -req_size=1048576 -benchmark_time=2000 -for_parallelism=true -max_body_size=1073741824 -socket_max_unwritten_bytes=1073741824
./build/src/brpc_test_client -req_size=10485760 -benchmark_time=2000 -for_parallelism=true -max_body_size=536870912 -socket_max_unwritten_bytes=1073741824

./build/src/brpc_test_client -req_size=10240 -benchmark_time=4000 -for_req_size=true -parallelism=1 -max_body_size=1073741824 -socket_max_unwritten_bytes=1073741824
