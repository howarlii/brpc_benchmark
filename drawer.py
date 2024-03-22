import os
import matplotlib.pyplot as plt
import csv

class DataDrawer:
    def __init__(self):
        self.x_axis = None
        self.lantencys = dict()
        self.speed =  dict()
        self.qps =  dict()
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
        if("lantencys" in rows[0]) and ("streaming" not in test_type):
            self.lantencys[test_type] = [float(row["lantencys"]) for row in rows]
        if("qps" in rows[0]):
            self.qps[test_type] = [float(row["qps"]) for row in rows]

    def draw(self):
        colors = ['b', 'r', 'g', 'y', 'm', 'c', 'k']

        # Create a new figure and two subplots
        fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8))

        # ax3 = ax2.twinx()
        for i in range(len(self.test_types)):
            test_type = self.test_types[i]
            color = colors[i]
            # Plot array latencies 1 on the first subplot
            if(test_type in self.lantencys):
                ax1.plot(self.x_axis, self.lantencys[test_type], marker='o', label=test_type, color=color)
            if(test_type in self.speed):
                ax2.plot(self.x_axis, self.speed[test_type], marker='^', label=f"{test_type} throughput (MB/S)", color=color)
            # if(test_type in self.qps):
            #     ax3.plot(self.x_axis, self.qps[test_type], marker='*', linestyle='--', label=f"{test_type} qps", color=color)

        if(self.x_axis[-1] / self.x_axis[0] > 100):
            ax1.set_xscale('log')
            ax2.set_xscale('log')

        ax1.set_xlabel(self.x_tag)
        ax1.set_ylabel('Latency (ms)')
        ax1.set_title('99% Latency(ms)')
        ax1.grid(True)
        ax1.legend()

        ax2.set_xlabel(self.x_tag)
        ax2.set_ylabel('throughput(MB)')
        ax2.grid(True)
        ax2.legend()

        # ax3.set_ylabel('QPS')
        # ax3.legend().set_loc('center right')

        # Adjust spacing between subplots
        plt.tight_layout()

        # Show the combined plot
        plt.savefig(f'result/array_latency_vs_{self.x_tag}_{self.sub_title}.png')

# sub_titles = ["1k", "10k", "100k", "1m"]
# for title in sub_titles:
#     p_drawer = DataDrawer()
#     p_drawer.setDrawer("parallel", title)
#     p_drawer.readCsv("proto", f"result/brpc_parallel_proto_reqsz({title})_para(1-50)_streamsz(9k)_prot(baidu_std).csv")
#     p_drawer.readCsv("attachment", f"result/brpc_parallel_attachment_reqsz({title})_para(1-50)_streamsz(9k)_prot(baidu_std).csv")
#     p_drawer.readCsv("c-streaming", f"result/brpc_parallel_sstreaming_reqsz({title})_para(1-50)_streamsz(9k)_prot(baidu_std).csv")
#     p_drawer.readCsv("iperf", f"result/iperf3_res.csv")
#     p_drawer.draw()

sub_titles = ["1"]
for title in sub_titles:
    p_drawer = DataDrawer()
    p_drawer.setDrawer("req-size", title)
    p_drawer.readCsv("proto", f"result/brpc_req-size_attachment_reqsz(256-32m)_para({title})_streamsz(8k)_prot(baidu_std).csv")
    p_drawer.readCsv("attachment", f"result/brpc_req-size_attachment_reqsz(256-32m)_para({title})_streamsz(8k)_prot(baidu_std).csv")
    p_drawer.readCsv("c-streaming", f"result/brpc_req-size_cstreaming_reqsz(256-32m)_para({title})_streamsz(8k)_prot(baidu_std).csv")
    p_drawer.readCsv("s-streaming", f"result/brpc_req-size_sstreaming_reqsz(256-32m)_para({title})_streamsz(8k)_prot(baidu_std).csv")
    # p_drawer.readCsv("iperf", f"result/iperf3_res.csv")
    p_drawer.draw()

# sub_titles = ["rs1m_p1"]
# for title in sub_titles:
#     p_drawer = DataDrawer()
#     p_drawer.setDrawer("req-size", title)
#     p_drawer.readCsv("streaming", f"result/brpc_streaming-size_streaming_{title}.csv")
#     p_drawer.draw()