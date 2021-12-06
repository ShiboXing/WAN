#!/bin/bash

# Step 1: set up testbed
source testbed_setup.sh

# Step 2: set bottleneck queue on router
if [[ "$HOST" == *"router"* ]]; 
	then
    queue=$(( $1 * 16 ))
    echo "setting router queue to '$1' BDPs, ${queue} packets"
    sudo tc qdisc replace dev eth0 root fq limit ${queue}
fi

# Step 2: set RTT to 40ms on sender
if [[ "$HOST" == *"sender"* ]]; 
	then
	echo "setting eth1 RTT delay on sender to ${2}ms"
	sudo tc qdisc add dev eth1 root netem delay $2ms
	sudo tc qdisc replace dev eth1 root netem delay $2ms
fi

