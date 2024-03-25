#!/bin/bash

# PORT=6667
SERVER_IP=127.0.0.1
TEST_DURATION=4
OUTPUT_FILE="result/iperf3_res.csv"
thread_nums=(1 2 4 6 8 10 12 14 16)

iperf3 -s -D

# Here we test by parallelism
throughput_arr=()
rtt_arr=()
x_axis=()
echo "x_axis,lantencys,speed" >"$OUTPUT_FILE"
for thread_num in "${thread_nums[@]}"; do
    result=$(iperf3 -c $SERVER_IP -P $thread_num -t $TEST_DURATION --json)
    # echo $result
    throughput=$(echo "$result" | jq -r '.end.sum_received.bits_per_second' | awk '{ printf "%.3f", $1/8/1024/1024 }')
    rtt=$(echo "$result" | jq -r '.end.streams[0].sender.mean_rtt' | awk '{ printf "%.3f", $1/1000 }')

    x_axis+=("$thread_num,")
    rtt_arr+=("$rtt,")
    throughput_arr+=("$throughput,")
    echo "$thread_num,$rtt,$throughput" >>"$OUTPUT_FILE"
    sleep 1
done

echo "# throughput_arr:"
echo "x_axis[\"parallel\"] = [${x_axis[@]}]"
echo "throughputs = [${throughput_arr[@]}]"
echo ""

# Here we test by payload size
# start_size="10240"
# end_size="4194304"
# num_tests=20
# increment=$(echo "scale=0; ($end_size - $start_size) / $num_tests + 1" | bc)

# results=()
# x_axis=()
# for ((i=0; i<num_tests; i++)); do
#     current_size=$(echo "scale=0; $start_size + $increment * ($i - 1)" | bc)
#     throughput=$(test_throughput ${current_size}b 1)
#     x_axis+=("${current_size}b,")
#     results+=("$throughput,")
#     sleep 1
# done

# pkill iperf3

# # 输出结果为Python数组
# echo "# Results:"
# echo "x_axis["req-size"] = [${x_axis[@]}]"
# echo "throughputs = [${results[@]}]"
