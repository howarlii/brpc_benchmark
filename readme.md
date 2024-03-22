# BRPC Benchmarker



## Add lantency at localhost

add delay:
`tc qdisc add dev lo root netem delay 10ms`
delete delay:
`tc qdisc del dev lo root netem delay 10ms`
show statu:
`tc qdisc show`

## Benchmark raw TCP using netperf

