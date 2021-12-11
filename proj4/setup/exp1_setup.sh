#!/bin/bash

# Step 1: set up testbed
source testbed_setup.sh

exp1_setup_help="usage: source exp1_setup [# of BDP] [bandwidth(mbits)] [RTT(ms)] [divisor (for fraction BDP)] [loss_rate(%)]"

# check parameters
if [ -z "$1" ] 
    then
    echo $exp1_setup_help
elif [ -z "$2" ] 
    then
    echo $exp1_setup_help
elif [ -z "$3" ] 
    then
    echo $exp1_setup_help
elif [ -z "$4" ] 
    then
    echo $exp1_setup_help
elif [ -z "$5" ] 
    then
    echo $exp1_setup_help
else
	rate=$(($2 * 1000))
	queue=$(( $2 * 1000000 / 1000 * $3/2 * $1 / 1000 / $4))

	ott=$3
	# Step 2: set bottleneck queue on router
	if [[ "$HOST" == *"router"* ]]; 
		then
	    echo "setting router queue to '$1' BDPs, ${queue}kb"
	    echo "setting bandwidth limit to $2mbps, ${rate}kbps"
	    sudo tc qdisc replace dev eth2 root tbf rate ${rate}kbit limit ${queue}kb burst 10kb
	    echo "setting router to sender single-trip time to ${ott}ms"
	    echo "setting router loss rate to ${5} percent"
	    sudo tc qdisc replace dev eth1 root netem delay ${ott}ms loss $5
	fi
fi



