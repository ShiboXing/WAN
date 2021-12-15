#!/bin/bash
bbr_port=5001
loss_port=5002
# add BBR protocol to /etc/sysctl.conf:
# 	net.core.default_qdisc=fq
#	net.ipv4.tcp_congestion_control=bbr

# aliasing programs
alias iperf3="~/proj4/iperf-3.10.1/src/iperf3"
alias init_exp="jupyter notebook --no-browser --port=8008 &"

# TESTBED SETUP
HOST=`hostname`
echo "hostname: $HOST"
