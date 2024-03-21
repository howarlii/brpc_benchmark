import os
import matplotlib.pyplot as plt
import csv

x_axis = dict()
attachment_lantencys = {"parallel": dict(), "req-size": dict()}
attachment_speed =  {"parallel": dict(), "req-size": dict()}
proto_lantencys =  {"parallel": dict(), "req-size": dict()}
proto_speed =  {"parallel": dict(), "req-size": dict()}
streaming_lantencys =  {"parallel": dict(), "req-size": dict()}
streaming_speed =  {"parallel": dict(), "req-size": dict()}

# # ================ paste data here begin ================
# x_axis["parallel"] = [1, 6, 11, 16, 21, 26, 31, 36, 41, 46, 51, 56, 61, ]

# # attachment  payload size 10240
# attachment_lantencys["parallel"]["10k"] = [43.152, 41.017, 59.854, 79.557, 97.192, 97.258, 97.925, 95.552, 91.675, 73.619, 82.495, 98.319, 79.943, ]
# attachment_speed["parallel"]["10k"] = [0.440399, 2.64264, 4.79431, 6.89358, 9.03093, 11.2264, 13.3624, 15.5207, 17.6066, 19.8223, 22.0345, 24.0496, 26.4739, ]

# # proto  payload size 10240
# proto_lantencys["parallel"]["10k"] = [40.87, 41.011, 80.221, 61.232, 78.308, 95.771, 96.334, 98.528, 85.549, 108.916, 85.979, 101.065, 78.787, ]
# proto_speed["parallel"]["10k"] = [0.441728, 2.64387, 4.75739, 6.93121, 9.02083, 11.1938, 13.329, 15.4026, 17.7981, 19.7989, 22.0481, 24.0227, 26.3927, ]

# # streaming  payload size 10240
# streaming_lantencys["parallel"]["10k"] = [41.892, 40.817, 40.78, 60.537, 60.436, 60.658, 60.473, 80.285, 80.275, 60.854, 81.04, 80.985, 81.135, ]
# streaming_speed["parallel"]["10k"] = [0.441395, 2.62881, 4.75576, 6.97926, 8.98846, 11.0768, 13.191, 14.9187, 17.1092, 19.0531, 21.077, 23.038, 24.0637, ]

# # attachment  payload size 102400
# attachment_lantencys["parallel"]["100k"] = [63.889, 122.041, 122.349, 139.583, 141.944, 136.966, 160.191, 154.72, 150.856, 149.121, 149.305, 173.67, 150.823, ]
# attachment_speed["parallel"]["100k"] = [4.14602, 24.4942, 44.8018, 64.6553, 86.1716, 106.358, 126.642, 134.347, 133.707, 133.944, 133.327, 132.839, 134.794, ]

# # proto  payload size 102400
# proto_lantencys["parallel"]["100k"] = [61.751, 121.693, 122.304, 121.659, 138.995, 160.091, 178.309, 155.768, 177.362, 153.362, 122.581, 152.204, 151.189, ]
# proto_speed["parallel"]["100k"] = [4.21182, 24.8131, 44.5693, 65.5769, 85.6541, 104.633, 124.5, 134.157, 131.758, 133.961, 132.42, 134.154, 134.57, ]

# # streaming  payload size 102400
# streaming_lantencys["parallel"]["100k"] = [41.919, 100.711, 120.692, 121.408, 121.377, 122.115, 122.545, 123.877, 125.183, 125.008, 141.491, 120.564, 122.441, ]
# streaming_speed["parallel"]["100k"] = [4.27014, 24.5001, 43.887, 60.026, 83.5556, 100.987, 108.145, 116.108, 121.466, 126.477, 128.388, 133.932, 132.301, ]

# # attachment  payload size 1048576
# attachment_lantencys["parallel"]["1m"] = [127.418, 205.054, 208.705, 270.861, 258.684, 306.017, 360.192, 386.288, 423.239, 452.352, 466.968, 500.751, 488.036, ]
# attachment_speed["parallel"]["1m"] = [31.7604, 131.431, 133.32, 132.758, 135.149, 138.165, 137.741, 139.485, 139.641, 141.347, 143.98, 145.674, 146.82, ]

# # proto  payload size 1048576
# proto_lantencys["parallel"]["1m"] = [147.084, 186.619, 194.036, 252.017, 305.936, 304.702, 341.761, 403.618, 419.582, 434.477, 470.934, 523.808, 520.367, ]
# proto_speed["parallel"]["1m"] = [31.1416, 131.684, 134.521, 133.986, 134.575, 137.307, 140.013, 138.583, 140.312, 142.602, 144.199, 143.563, 145.24, ]

# # streaming  payload size 1048576
# streaming_lantencys["parallel"]["1m"] = [100.466, 160.319, 186.435, 202.712, 250.374, 284.834, 310.036, 359.321, 350.722, 380.61, 412.096, 460.567, 520.133, ]
# streaming_speed["parallel"]["1m"] = [40.394, 131.271, 138.997, 142.976, 145.045, 144.972, 151.962, 153.005, 159.805, 162.681, 167.449, 171.57, 172.325, ]

# # attachment  payload size 10485760
# attachment_lantencys["parallel"]["10m"] = [266.809, 419.287, 897.103, 1145.62, 1551.46, 2021.15, 2361.64, 2553.97, 2918.37, 3300.43, 3610.51, 3851.93, 4301.82, ]
# attachment_speed["parallel"]["10m"] = [69.3105, 137.404, 153.108, 165.601, 172.804, 177.168, 181.937, 184.846, 190.161, 195.092, 203.38, 212.625, 210.575, ]

# # proto  payload size 10485760
# proto_lantencys["parallel"]["10m"] = [253.768, 535.384, 882.742, 1164.05, 1502.58, 1713.06, 2225.2, 2639.3, 2999.16, 3393.63, 3630.58, 3593.15, 4304.18, ]
# proto_speed["parallel"]["10m"] = [65.5999, 142.813, 152.59, 163.577, 173.174, 195.456, 181.325, 184.876, 190.115, 194.038, 201.31, 211.449, 207.661, ]


# # attachment  payload size 4194300
# x_axis["req-size"] = [10240, 219443, 428646, 637849, 847052, 1056255, 1265458, 1474661, 1683864, 1893067, 2102270, 2311473, 2520676, 2729879, 2939082, 3148285, 3357488, 3566691, 3775894, 3985097, 4194300, ]
# attachment_lantencys["req-size"]["3m"] = [20.991, 40.042, 27.878, 124.076, 125.035, 125.738, 166.252, 146.844, 147.596, 168.724, 149.329, 172.944, 171.212, 190.302, 171.294, 172.038, 173.024, 172.19, 193.257, 174.125, 174.62, ]
# attachment_speed["req-size"]["3m"] = [0.443382, 8.65544, 16.1208, 22.6082, 29.3233, 36.4633, 41.3651, 46.8047, 51.8399, 56.4102, 61.0121, 65.5111, 68.3055, 72.1799, 76.3199, 53.9403, 56.6385, 58.9736, 60.4343, 62.7931, 65.5713, ]

# # proto  payload size 4194300
# proto_lantencys["req-size"]["3m"] = [40.846, 123.183, 37.345, 38.668, 145.323, 125.754, 165.747, 167.524, 148.051, 168.931, 149.86, 150.422, 170.136, 170.854, 179.791, 173.004, 172.953, 177.784, 175.602, 195.11, 196.838, ]
# proto_speed["req-size"]["3m"] = [0.443219, 8.70821, 16.0776, 22.5961, 28.8467, 33.7099, 38.4747, 42.6424, 47.0881, 50.8189, 54.7544, 57.9592, 60.7652, 63.3787, 66.2954, 48.1125, 50.0107, 53.0419, 55.0703, 55.4404, 58.4383, ]



# ================ paste data here end ================

class DataDrawer:
    def __init__(self):
        self.x_axis = None
        self.lantencys = dict()
        self.speed =  dict()
        self.test_types = []

    def setDrawer(self, x_tag, sub_title):
        self.x_tag = x_tag
        self.sub_title = sub_title

    def readCsv(self, test_type, file_path):
        if os.path.exists(file_path) == False:
            print (f"File {file_path} not exists")
            return
        with open(file_path, newline='') as csvfile:
            reader = csv.DictReader(csvfile)
            rows = list(reader)

        self.test_types.append(test_type)

        if(self.x_axis == None):
            self.x_axis = [int(row["x_axis"]) for row in rows]
        # else:
        #     t = [int(row["x_axis"]) for row in rows]

        if("speed" in rows[0]):
            self.speed[test_type] = [float(row["speed"]) for row in rows]
        if("lantencys" in rows[0]):
            self.lantencys[test_type] = [float(row["lantencys"]) for row in rows]

    def draw(self):
        colors = ['b', 'r', 'g', 'y', 'm', 'c', 'k']

        # Create a new figure and two subplots
        fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8))

        for i in range(len(self.test_types)):
            test_type = self.test_types[i]
            color = colors[i]
            # Plot array latencies 1 on the first subplot
            if(test_type in self.lantencys):
                ax1.plot(self.x_axis, self.lantencys[test_type], marker='o', label=test_type, color=color)
            if(test_type in self.speed):
                ax2.plot(self.x_axis, self.speed[test_type], marker='^', label=test_type, color=color)

        ax1.set_xlabel(self.x_tag)
        ax1.set_ylabel('Latency (ms)')
        ax1.set_title('99% Latency bewteen different method')
        ax1.grid(True)
        ax1.legend()

        ax2.set_xlabel(self.x_tag)
        ax2.set_ylabel('throughput (MB)')
        ax2.set_title('throughput bewteen different method')
        ax2.grid(True)
        ax2.legend()

        # Adjust spacing between subplots
        plt.tight_layout()

        # Show the combined plot
        plt.savefig(f'result/array_latency_vs_{self.x_tag}_{self.sub_title}.png')

sub_titles = ["10k", "100k", "1m", "10m"]
for title in sub_titles:
    p_drawer = DataDrawer()
    p_drawer.setDrawer("parallel", title)
    p_drawer.readCsv("proto", f"result/brpc_parallel_proto_{title}.csv")
    p_drawer.readCsv("attachment", f"result/brpc_parallel_attachment_{title}.csv")
    p_drawer.readCsv("streaming", f"result/brpc_parallel_streaming_{title}.csv")
    p_drawer.readCsv("iperf", f"result/iperf3_res.csv")
    p_drawer.draw()

sub_titles = ["1"]
for title in sub_titles:
    p_drawer = DataDrawer()
    p_drawer.setDrawer("req-size", title)
    p_drawer.readCsv("proto", f"result/brpc_req-size_proto_{title}.csv")
    p_drawer.readCsv("attachment", f"result/brpc_req-size_attachment_{title}.csv")
    p_drawer.readCsv("streaming", f"result/brpc_req-size_streaming_{title}.csv")
    # p_drawer.readCsv("iperf", f"result/iperf3_res.csv")
    p_drawer.draw()