import matplotlib.pyplot as plt

parallelisms = [1, 2, 4, 8, 16, 32, 64, ]
attachment_lantencys = dict()
attachment_speed = dict()
proto_lantencys = dict()
proto_speed = dict()
streaming_lantencys = dict()
streaming_speed = dict()

# ================ paste data here begin ================

# attachment  payload size 10240
attachment_lantencys["10k"] = [43.023, 40.858, 40.705, 41.552, 59.921, 73.168, 67.206, ]
attachment_speed["10k"] = [0.441557, 0.885717, 1.75337, 3.50728, 6.99694, 13.9411, 28.0845, ]

# proto  payload size 10240
proto_lantencys["10k"] = [41.037, 41.179, 40.421, 41.116, 59.658, 59.86, 80.192, ]
proto_speed["10k"] = [0.441539, 0.882758, 1.76217, 3.51301, 6.96505, 13.9703, 27.8181, ]

# streaming  payload size 10240
streaming_lantencys["10k"] = [43.322, 40.793, 39.991, 40.256, 58.403, 60.33, 60.154, ]
streaming_speed["10k"] = [0.441235, 0.887115, 1.77665, 3.50698, 6.98453, 13.6999, 26.5151, ]

# attachment  payload size 1048576
attachment_lantencys["1m"] = [127.05, 165.487, 165.902, 208.381, 234.04, 328.486, 526.953, ]
attachment_speed["1m"] = [32.9202, 68.3548, 121.386, 134.739, 137.406, 139.702, 144.64, ]

# proto  payload size 1048576
proto_lantencys["1m"] = [126.594, 146.754, 166.754, 206.182, 231.162, 329.894, 505.382, ]
proto_speed["1m"] = [32.2823, 64.804, 113.329, 134.263, 137.27, 139.794, 145.728, ]

# streaming  payload size 1048576
streaming_lantencys["1m"] = [100.514, 121.164, 142.992, 182.147, 209.539, 301.443, 455.309, ]
streaming_speed["1m"] = [41.0702, 76.2038, 89.2708, 136.982, 142.319, 151.017, 167.638, ]

# attachment  payload size 10485760
attachment_lantencys["10m"] = [249.016, 321.608, 470.501, 656.834, 1158.34, 2338.07, 4568.32, ]
attachment_speed["10m"] = [74.2385, 132.58, 135.604, 147.156, 169.631, 174.698, 195.375, ]

# proto  payload size 10485760
proto_lantencys["10m"] = [248.922, 327.932, 468.177, 672.302, 1193.54, 2261.11, 4564.71, ]
proto_speed["10m"] = [70.8065, 132.988, 134.654, 143.588, 154.995, 174.66, 195.897, ]



# ================ paste data here end ================


def draw(payload_size):
    # Create a new figure and two subplots
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8))

    # Plot array latencies 1 on the first subplot
    ax1.plot(parallelisms, attachment_lantencys[payload_size], marker='o', label='attachment', color='b')
    ax1.plot(parallelisms, proto_lantencys[payload_size], marker='o', label='proto', color='r')
    if(payload_size in streaming_lantencys):
        ax1.plot(parallelisms, streaming_lantencys[payload_size], marker='o', label='streaming', color='r')
    ax1.set_xlabel('Parallelisms')
    ax1.set_ylabel('Latency (ms)')
    ax1.set_title('99% Latency bewteen different method')
    ax1.grid(True)
    ax1.legend()

    # Plot array latencies 2 on the second subplot
    ax2.plot(parallelisms, attachment_speed[payload_size], marker='^', label='attachment', color='b')
    ax2.plot(parallelisms, proto_speed[payload_size], marker='^', label='proto', color='r')
    if(payload_size in streaming_speed):
        ax2.plot(parallelisms, streaming_speed[payload_size], marker='^', label='streaming', color='r')
    ax2.set_xlabel('Parallelisms')
    ax2.set_ylabel('throughput (ms)')
    ax2.set_title('throughput bewteen different method')
    ax2.grid(True)
    ax2.legend()

    # Adjust spacing between subplots
    plt.tight_layout()

    # Show the combined plot
    plt.savefig(f'array_latency_vs_parallelism_{payload_size}.png')

draw("10k")
draw("1m")
draw("10m")
