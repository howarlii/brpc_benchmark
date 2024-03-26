import os
import matplotlib.pyplot as plt
import csv

datas = dict()
x_axis = dict()

def readCsv(test_type, file_path):
    if os.path.exists(file_path) == False:
        print (f"File {file_path} not exists")
        return
    with open(file_path, newline='') as csvfile:
        reader = csv.DictReader(csvfile)
        rows = list(reader)

    global x_axis
    global datas
    x_axis[test_type] = [int(row["x_axis"]) for row in rows]

    datas[test_type] = dict()

    if("speed" in rows[0]):
        datas[test_type]["speed"] = [float(row["speed"]) for row in rows]
    if("lantencys" in rows[0]) and ("streaming" not in test_type):
        datas[test_type]["delays"]= [float(row["lantencys"]) for row in rows]
    if("qps" in rows[0]):
        datas[test_type]["qps"]= [float(row["qps"]) for row in rows]

class DataDrawer:
    def __init__(self):
        self.x_axis = None
        self.delays = dict()
        self.speed =  dict()
        self.qps =  dict()
        self.test_labels = []

    def setData(self, test_type, test_label):
        self.test_labels.append(test_label)
        if(self.x_axis == None):
            self.x_axis = x_axis[test_type]
        else:
            assert self.x_axis == x_axis[test_type]

        if("speed" in datas[test_type]):
            self.speed[test_label] = datas[test_type]["speed"]
        if("delays" in datas[test_type]) and ("streaming" not in test_type):
            self.delays[test_label] = datas[test_type]["delays"]
        if("qps" in datas[test_type]):
            self.qps[test_label] = datas[test_type]["qps"]

    def draw(self, xlabel, title, file_name):
        colors = ['b', 'r', 'g', 'y', 'm', 'c', 'k']

        # Create a new figure and two subplots
        fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8))

        # ax3 = ax2.twinx()
        for i in range(len(self.test_labels)):
            test_type = self.test_labels[i]
            color = colors[i]
            # Plot array latencies 1 on the first subplot
            if(test_type in self.delays):
                ax1.plot(self.x_axis, self.delays[test_type], marker='+', label=test_type, color=color)
            if(test_type in self.speed):
                ax2.plot(self.x_axis, self.speed[test_type], marker='+', label=f"{test_type}", color=color)
            # if(test_type in self.qps):
            #     ax3.plot(self.x_axis, self.qps[test_type], marker='*', linestyle='--', label=f"{test_type} qps", color=color)

        if(self.x_axis[-1] / self.x_axis[0] > 100):
            ax1.set_xscale('log')
            ax2.set_xscale('log')

        ax1.set_xlabel(xlabel)
        ax1.set_ylabel(f'99% Latency (ms)')
        # ax1.set_title('99% Latency(ms)')
        ax1.grid(True)
        ax1.legend()

        ax2.set_xlabel(xlabel)
        ax2.set_ylabel('Throughput (MB/S)')
        ax2.grid(True)
        ax2.legend()

        # ax3.set_ylabel('QPS')
        # ax3.legend().set_loc('center right')

        # Adjust spacing between subplots
        fig.suptitle(title)
        plt.tight_layout()

        # Show the combined plot
        plt.savefig(f'result/{file_name}.png')


def drawIperf():
    colors = ['b', 'r', 'g', 'y', 'm', 'c', 'k']

    # Create a new figure and two subplots
    fig, (ax1) = plt.subplots(1, 1, figsize=(10, 4))

    delays = ["0ms", "1ms", "10ms"]
    for i1 in range(len(delays)):
        test_type = f"iperf-{delays[i1]}"
        color = colors[i1]
        ax1.plot(x_axis[test_type], datas[test_type]["speed"], marker='^', label=f"{test_type}", color=color)

    ax1.set_xlabel("Parallelism")
    ax1.set_ylabel('Throughput (MB/S)')
    ax1.grid(True)
    ax1.legend()

    fig.suptitle("iperf3 benchmakr")

    plt.tight_layout()
    plt.savefig(f'result/iperf3.png')

def drawDelays(parallel):
    colors = ['b', 'r', 'g', 'y', 'm', 'c', 'k']
    linestyles = ['-', '--', ':', '-.']
    # markers = ['^', 'x', 's', 'v', '+','o', 'D']

    # Create a new figure and two subplots
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8), height_ratios=[1, 2])

    test_labels = ["proto", "attachment", "c-streaming"]
    delays = ["0ms", "1ms", "10ms"]
    for i2 in range(len(test_labels)):
        for i1 in range(len(delays)):
            d = delays[i1]
            test_type = f"{test_labels[i2]}-{d}-p{parallel}"
            color = colors[i2]
            if("delays" in datas[test_type]):
                ax1.plot(x_axis[test_type], datas[test_type]["delays"],linestyle = linestyles[i1], label=f"{test_type}", color=color)
            if("speed" in datas[test_type]):
                ax2.plot(x_axis[test_type], datas[test_type]["speed"],linestyle = linestyles[i1], label=f"{test_type}", color=color)

    if(x_axis[test_type][-1] / x_axis[test_type][0] > 100):
        ax1.set_xscale('log')
        ax2.set_xscale('log')

    ax1.set_ylabel(f'99% Latency (ms)')
    ax1.grid(True)
    ax1.legend()

    ax2.set_xlabel("req-size (Byte)")
    ax2.set_ylabel('Throughput (MB/S)')
    ax2.grid(True)
    ax2.legend()

    fig.suptitle(f"parallel={parallel} protocol=baidu_std")
    plt.tight_layout()
    plt.savefig(f'result/req-size_delays_reqsz(256-256m)_para({parallel})_streamsz(8k)_prot(baidu_std).png')

def drawParallel(delay):
    colors = ['b', 'r', 'g', 'y', 'm', 'c', 'k']
    linestyles = ['-', '--', ':', '-.', '-', '--']

    # Create a new figure and two subplots
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8), height_ratios=[1, 2])

    test_labels = ["proto", "attachment", "c-streaming"]
    # parallel = ["1", "2", "4", "6", "8"]
    for i2 in range(len(test_labels)):
        for i1 in range(len(parallel)):
            p = parallel[i1]
            test_type = f"{test_labels[i2]}-{delay}-p{p}"
            if("delays" in datas[test_type]):
                ax1.plot(x_axis[test_type], datas[test_type]["delays"],linestyle = linestyles[i1],  label=f"{test_type}", color=colors[i2])
            if("speed" in datas[test_type]):
                ax2.plot(x_axis[test_type], datas[test_type]["speed"],linestyle = linestyles[i1],  label=f"{test_type}", color=colors[i2])

    if(x_axis[test_type][-1] / x_axis[test_type][0] > 100):
        ax1.set_xscale('log')
        ax2.set_xscale('log')

    ax1.set_ylabel(f'99% Latency (ms)')
    ax1.grid(True)
    ax1.legend().set_loc('center left')

    ax2.set_xlabel("req-size (Byte)")
    ax2.set_ylabel('Throughput (MB/S)')
    ax2.grid(True)
    ax2.legend()

    fig.suptitle(f"delay={delay} protocol=baidu_std")
    plt.tight_layout()
    plt.savefig(f'result/req-size_delay{delay}_reqsz(256-256m)_paras_streamsz(8k)_prot(baidu_std).png')



delays = ["0ms", "1ms", "10ms"]
parallel = ["1", "2", "4", "6", "8"]
for delay in delays:
    readCsv(f"iperf-{delay}", f"result/{delay}/iperf3_res.csv")
    for pa in parallel:
        readCsv(f"proto-{delay}-p{pa}", f"result/{delay}/brpc_req-size_proto_reqsz(256-256m)_para({pa})_streamsz(8k)_prot(baidu_std).csv")
        readCsv(f"attachment-{delay}-p{pa}", f"result/{delay}/brpc_req-size_attachment_reqsz(256-256m)_para({pa})_streamsz(8k)_prot(baidu_std).csv")
        readCsv(f"c-streaming-{delay}-p{pa}", f"result/{delay}/brpc_req-size_cstreaming_reqsz(256-256m)_para({pa})_streamsz(8k)_prot(baidu_std).csv")

        p_drawer = DataDrawer()
        p_drawer.setData(f"proto-{delay}-p{pa}", "proto")
        p_drawer.setData(f"attachment-{delay}-p{pa}", "attachment")
        p_drawer.setData(f"c-streaming-{delay}-p{pa}", "c-streaming")
        p_drawer.draw("req-size (Byte)", f"delay={delay} parallel={pa} protocol=baidu_std",
                      f"req-size_delay{delay}_reqsz(256-256m)_para({pa})_streamsz(8k)_prot(baidu_std)")

delay = "1ms"
pa = 1
readCsv(f"proto-{delay}-p{pa}-h2:grpc", f"result/{delay}/brpc_req-size_proto_reqsz(256-256m)_para(1)_streamsz(8k)_prot(h2:grpc).csv")
readCsv(f"proto-{delay}-p{pa}-hulu", f"result/{delay}/brpc_req-size_proto_reqsz(256-256m)_para(1)_streamsz(8k)_prot(hulu_pbrpc).csv")
p_drawer = DataDrawer()
p_drawer.setData(f"proto-{delay}-p{pa}", "baidu_std")
p_drawer.setData(f"proto-{delay}-p{pa}-h2:grpc", "h2:grpc")
p_drawer.setData(f"proto-{delay}-p{pa}-hulu", "hulu")
p_drawer.draw("req-size (Byte)", f"delay={delay} parallel={pa}",
              f"req-size_delay{delay}_reqsz(256-256m)_para({pa})_streamsz(8k)_prots")


drawIperf()

drawDelays(parallel = 1)
drawDelays(parallel = 4)

drawParallel("1ms")
