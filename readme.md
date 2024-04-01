# BRPC Benchmark

测试BRPC使用不同的方式，传输大量binary数据的性能表现。

测试不同的传输方式包括：
- 以bytes data的形式放在proto中；
- 以attachment的形式放在cntl中；
- 以BRPC streaming的方式；

其中attachment会绕过protobuf的序列化/反序列化和BRPC框架的压缩过程，[详细说明见官方文档](https://brpc.apache.org/docs/client/basics/#attachment)。

其中BRPC streaming可以参考[brpc对streaming的描述](https://brpc.apache.org/docs/client/streaming-rpc/)。简单来说，一个streaming由一次RPC触发而建立，client/server均可以往streaming中持续读/写数据。

Streaming的设计意图在于C/S段建立一个流式数据通道，从而持续的收发数据，而且创建一个新的streaming的代价较高，因此本文对streaming的测试方式有两种：
- Single-streaming: 不复用streaming，每次重新创建新的streaming，发送完payload之后关闭streaming；
- Continue-streaming: 复用streaming，一个client只在开始时创建一个streaming，之后所有的数据均通过此streaming发送/接受；

下文所有关于Streaming的测试，除非特殊说明，均使用Continue-streaming的方式。

其中Continue-streaming的single_msg_size为payload_size。

测试分为以下几个大类：
- 固定并发度、延迟，测试不同payload size下，不同方式的concurency/thourghput;
- ~~固定payload size，测试不同并发下，不同方式的delay/thourghput;~~

## 结论

- `baidu_std`支持本文三种传输方式，throughput/latency表现也比hulu, grpc优异；
- Attachment的方式在payload较大时throughput可以达到Streaming的50%，但胜在无需建立Streaming通道；
- Streaming的方式throughput最高，但是建立新streaming通道的时间代价较高；
- 使用Streaming的方式时，适当提高`max_buf_size`可以显著提高throughput，测试中设置`max_buf_size=32M`即可达到最佳性能；单次发送的`IOBuffer`大小超过`max_buf_size`时会有巨大性能损失；

### 使用BRPC Streaming的Tips

- 使用Streaming的方式时，适当提高`max_buf_size`可以显著提高throughput；单次发送的`IOBuffer`大小超过`max_buf_size`时会有巨大性能损失；
- Streaming保证message的边界和顺序，server端`on_recieve`被调用时可能同时传入多组数据（由`messages_in_batch`控制，最大传入组数不超过此数值）；
- 从测试结果看来，`messages_in_batch`并不会控制streaming收发逻辑；
- 发送massgae时，请保证`StreamWrite()`传入的`butil::IOBuf`非空，**传入空的、未初始化的`butil::IOBuf`为未定义行为**；

## 遗留问题

- 当`parallelism=8`时，attachment的方式在`req_size=180M`时throughput有异常上升，原因暂不明确；

# Benchmark Result

所有原始结果数据位于目录`./result/`。

测试的机器采用lab-wroker2(96c192t)，
测试方法为：Client/Server均运行在同一个docker中，，使用`127.0.0.1`为server address, 通过`tc qdisc add dev lo root netem delay Xms`设定localhost延迟。
其中Continue-streaming的`single_msg_size=req_size`

## Iperf3 Test
测试本地TCP连接的throughput，设定`delay=0ms/1ms/10ms`：

![iperf3](./result/figs/iperf3.png)

注意：iperf3的parallel会开启多个TCP connection进行传输，而下列BRPC的测试中将只会启用一个TCP connection。因此对照的具体速率应当为：

| Delay       | Throughput |
| ---------- | -----------  |
| 0ms        | 4332.478MB/S |
| 1ms        | 1381.071MB/S |
| 10ms       | 136.542MB/S |

## Request Size
当`delay=1ms`时，单个Client的Throughput/Lantency vs Request Size数据如下：

![delay=1ms, parallel=1](./result/figs/req-size_delay1ms_reqsz(256-256m)_para(1)_streamsz(8k)_prot(baidu_std).png)

可以看到，使用proto的方式进行传输，在req_size较大时，Lantency/Throughput表现均劣化，估计是受到了序列化/反序列化的性能影响。
当`req_size=45MB`时，所有方式throughput还未劣化，后文所有的具体分析均取此时的数据进行分析。

在总体的传输throughput上，Streaming的方式基本相比attachment, proto的方式均有很大的优势，当`single_msg_size`较大时会显著降低throughput，因此使用streaming应当避免一次性发送过大的massage，官方对此在文档中的说明为：
>We do not support segment large messages automatically so that multiple Streams on a single TCP connection may lead to Head-of-line blocking problem. Please avoid putting huge data into single message until we provide automatic segmentation.

## Parallel
保持`delay=1ms`，使用1/2/4个Client进行并发传输，总Throughput/Lantency vs Request Size数据如下：

![delay=1ms](./result/figs/req-size_delay1ms_reqsz(256-256m)_paras_streamsz(8k)_prot(baidu_std).png)

结果表示，并发调用对BRPC的throughput有一定的帮助，使用两个client进行通讯可以将throughput几乎翻倍，但当parallelism从2提升到4之后，对throughput的提升非常有限, 但RPC的latency却提升了。取具体数值进行分析：

| req_size=45MB | Poto (P=1) | Poto (P=2) | Poto (P=4)      | Attachment (P=1) | Attachment (P=2) | Attachment (P=4)  |
| -----------  | -----------  | ---------- | ----------      | -----------  | ---------- | ----------      |
| Throughput   | 263.0 (100%) | 473.2 (180%) | 870.4 (331%)  | 347.8 (100%) | 606.8 (174%) | 991.6 (285%) |
| Latency      | 144.6 (100%) | 198.0 (137%) | 315.9 (218%)  | 127.0 (100%) | 168.8 (133%) | 263.6 (208%) |

但是对于attachment，其throughput在`parallelism=8`时有异常的提升，原因未知。

## Delay
使用单个Client，设定`delay=0ms/1ms/10ms`，Throughput/Lantency vs Request Size数据如下：

> 注意：这里的`delay`指使用tc设定的网络延迟，`latency`指rpc端到端的延迟。

![delay=1ms](./result/figs/req-size_delays_reqsz(256-256m)_para(1)_streamsz(8k)_prot(baidu_std).png)

同时，结果显示，当lantency较大时，所有传输方式的throughput均有很大程度下降。

提高并发度之后，delay对throughput的影响一定幅度减少：
![delay=1ms](./result/figs/req-size_delays_reqsz(256-256m)_para(4)_streamsz(8k)_prot(baidu_std).png)
当parallelism=4时，delay=1ms和delay=0ms的throughput差距非常小，但是delay=10ms的throughput依旧有较大差距。


## Different Protocal
保持`delay=1ms`，使用1个Client进行传输，设置不同RPC protocol的情况下，Throughput/Lantency vs Request Size数据如下：

![delay=1ms parallel=1 prots](./result/figs/req-size_delay1ms_reqsz(256-256m)_para(1)_streamsz(8k)_prots.png)


## Streaming Options

BRPC的streaming存在若干optina，可能与性能有关的option有：

```cpp
// The max size of unconsumed data allowed at remote side.
// If |max_buf_size| <= 0, there's no limit of buf size
// default: 2097152 (2M)
int max_buf_size;

// Maximum messages in batch passed to handler->on_received_messages
// default: 128
size_t messages_in_batch;
```

注意：由于程序统计方式的问题，`messages_in_batch`数值过大会导致统计精度严重下降，测出的`throughput`数值偏小。下图中不同`messages_in_batch`的`throughput`变化并不显著，因此我认为，此变化是由误差导致的，`messages_in_batch`并不参与streaming的网络收发逻辑。

对于`max_buf_size`，可以看到提高的`max_buf_size`可以显著提高`throughput`，在`max_buf_size=32M`时提升达到瓶颈。框架支持单次发送的`IOBuffer`size大于`max_buf_size`，但会有严重的性能损失。

![delay=1ms parallel=1 streams](./result/figs/req-size_delay1ms_reqsz(256-256m)_para(1)_streamsz(8k)_prot(baidu_std)_streams.png)



# Run The Benchmark

```bash
./run_benchmark.sh
```