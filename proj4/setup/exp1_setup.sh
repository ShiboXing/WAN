#!/bin/bash

# Step 1: set up testbed
source testbed_setup.sh

# Step 2: set bottleneck queue on router
if [[ "$HOST" == *"router"* ]]; 
	then
    queue=$(( $2 * 1000000 / 1000 * $3 * $1 / 1000 ))
    ott=$(( $3 / 2))
    echo "setting router queue to '$1' BDPs, ${queue}kb"
    sudo tc qdisc replace dev eth2 root tbf rate 6mbit limit ${queue}kb burst 10kb
    echo "setting router to sender single-trip time to ${ott}ms"
    sudo tc qdisc replace dev eth1 root netem delay ${ott}ms
fi

# Step 2: set RTT to 40ms on sender
if [[ "$HOST" == *"sender"* ]]; 
	then
	echo "setting eth1 one-trip delay on sender to ${2}ms"
	sudo tc qdisc replace dev eth1 root netem delay $2ms
fi

