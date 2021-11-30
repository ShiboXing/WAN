#!/bin/bash

# add BBR protocol to /etc/sysctl.conf:
# 	net.core.default_qdisc=fq
#	net.ipv4.tcp_congestion_control=bbr

# aliasing programs
alias iperf3='~/WAN/proj4/iperf-3.10.1/src/iperf3'

# Step 1: set bandwidth to 10mbps (during GENI node setup)
# Step 2: set RTT to 40ms
HOST=`hostname`
echo "hostname: $HOST"
if [[ "$HOST" == *"sender"* ]]; then
	echo 'setting RTT delay on sender'
	sudo tc qdisc add dev eth1 root netem delay 40ms
	sudo tc qdisc replace dev eth1 root netem delay 40ms
fi

