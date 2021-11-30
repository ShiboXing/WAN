#!/bin/bash

# add BBR protocol to /etc/sysctl.conf:
# 	net.core.default_qdisc=fq
#	net.ipv4.tcp_congestion_control=bbr

# aliasing programs
alias iperf3='~/WAN/proj4/iperf-3.10.1/src/iperf3'
alias init_exp="jupyter notebook --no-browser --port=8008 &"

# TESTBED SETUP
# Step 1: set bandwidth to 10mbps (during GENI node setup)
HOST=`hostname`
echo "hostname: $HOST"

# Step 2: set RTT to 40ms on sender
if [[ "$HOST" == *"sender"* ]]; 
	then
	echo 'setting RTT delay on sender'
	sudo tc qdisc add dev eth1 root netem delay 40ms
	sudo tc qdisc replace dev eth1 root netem delay 40ms
fi
