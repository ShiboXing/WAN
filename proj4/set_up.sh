# Step 1: set bandwidth to 10mbps (during GENI node setup)
# Step 2: set RTT to 40ms
sudo tc qdisc replace dev eth1 root netem delay 40ms
alias iperf3='~/WAN/proj4/iperf-3.10.1/src/iperf3'
