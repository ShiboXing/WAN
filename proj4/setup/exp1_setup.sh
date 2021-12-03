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
