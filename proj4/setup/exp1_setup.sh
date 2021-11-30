#!/bin/bash

# Step 1: set up testbed
source testbed_setup.sh

# Step 2: set bottleneck queue on router
if [[ "$HOST" == *"router"* ]]; 
	then
    queue=$(( $1 * 200 ))
    echo "setting router queue to '$1' BDPs, ${queue}kb"
    sudo tc qdisc add dev eth2 root tbf rate 100mbit limit ${queue}kb burst 10kb
    sudo tc qdisc replace dev eth2 root tbf rate 100mbit limit ${queue}kb burst 10kb
fi
