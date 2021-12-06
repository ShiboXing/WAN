#!/bin/bash
help="usage: source exp1_run.sh [queue(# of BDPs)] [# of BBRs] [# of Renos] [bandwidth(mbps)] [RTT(ms)]"
res_pth="./iperf3_results"
bbr_port=5001
loss_port=5002

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
# set up and run exp
else
    BDP=$1
    BBR_num=$2
    reno_num=$3
    bw=$4
    RTT=$5
    source exp1_setup.sh $1 $2 $5
    mkdir -p "$res_pth"
    
    # run on server
    if [[ "$HOST" == *"server"* ]]; 
        then
        
        # kill processes occupying ports
        lsof -i:$bbr_port -t | xargs kill -9
        lsof -i:$loss_port -t | xargs kill -9
        
        # run exp
        iperf3 -s -J -p $bbr_port &
        iperf3 -s -J -p $loss_port &     
    
    # run on sender
    elif [[ "$HOST" == *"sender"* ]]; 
        then
        
        # kill processes occupying ports
        lsof -i:$bbr_port -t | xargs kill -9
        lsof -i:$loss_port -t | xargs kill -9
        
        # run exp
        iperf3 -c server -C bbr -P $2 -b $4mb -p $bbr_port -t 60 -T "BBR" > "${res_pth}/exp1_bbr_${BDP}_${BBR_num}.json"&
        iperf3 -c server -C reno -P $3 -b $4mb -p $loss_port -t 60 -T "RENO" > "${res_pth}/exp1_reno_${BDP}_${BBR_num}.json"&        
    fi
fi
