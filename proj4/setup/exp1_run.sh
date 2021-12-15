#!/bin/bash
help="usage: source exp1_run.sh [queue(# of BDPs)] [# of BBRs] [# of Renos] [bandwidth(mbps)] [RTT(ms)] [duration(s)]"
res_pth="./iperf3_results"
alias iperf3="~/proj4/iperf-3.10.1/src/iperf3"

# check parameters
if [ -z "$1" ] 
    then
    echo $help
elif [ -z "$2" ] 
    then
    echo $help
elif [ -z "$3" ] 
    then
    echo $help
elif [ -z "$4" ] 
    then
    echo $help
elif [ -z "$5" ] 
    then
    echo $help
elif [ -z "$6" ] 
    then
    echo $help
# set up and run exp
else
    BDP=$1
    BBR_num=$2
    reno_num=$3
    bw=$4
    RTT=$5
    mkdir -p "$res_pth"
    source testbed_setup.sh
    
    # run on server
    if [[ "$HOST" == *"server"* ]]; 
        then
        
        # run exp
        ~/proj4/iperf-3.10.1/src/iperf3 -s -p $bbr_port &
        ~/proj4/iperf-3.10.1/src/iperf3 -s -p $loss_port &     
    
    # run on sender
    elif [[ "$HOST" == *"sender"* ]]; 
        then
        echo "running the experiments now queue: $1 bbr_num: $2 cubic_num: $3 bw: $bw rtt: $5"
        # run exp
        ~/proj4/iperf-3.10.1/src/iperf3 -c server -C bbr -P $2 -p $bbr_port -t $6 -T "BBR" -J > "${res_pth}/bbr_$1_$2_$3_$4_$5.json" &
        ~/proj4/iperf-3.10.1/src/iperf3 -c server -C cubic -P $3 -p $loss_port -t $6 -T "CUBIC" -J > "${res_pth}/cubic_$1_$2_$3_$4_$5.json" &    
        
    fi
fi
