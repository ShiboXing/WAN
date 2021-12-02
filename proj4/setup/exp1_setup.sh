#!/bin/bash

# Step 1: set up testbed
source testbed_setup.sh

# Step 2: set bottleneck queue on router
if [[ "$HOST" == *"router"* ]]; 
	then
    queue=$(( $1 * 400000 / 8 ))
    echo "setting router queue to '$1' BDPs, ${queue} bytes"
    sudo tc qdisc replace dev eth2 root bfifo limit ${queue}
fi
