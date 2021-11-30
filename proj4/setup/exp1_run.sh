#!/bin/bash
help="source exp1_run.sh [queue(# of BDPs)] [# of BBRs]"
res_pth="\~/.iperf3_results"
bbr_port=5001
loss_port=5002

# check parameters
if [ -z "$1" ] 
    then
    echo "please enter a BDP number for the bottleneck queue"
elif [ -z "$2" ] 
    then
    echo "please enter the number of BBR flows"
# set up and run exp
else
    BDP=$1
    BBR_num=$2
    source exp1_setup.sh $1 $2
    
    
    if [[ "$HOST" == *"server"* ]]; 
        then
        mkdir -p "$res_pth"
        
        # kill processes occupying ports
        lsof -i:$bbr_port -t | xargs kill -9
        lsof -i:$loss_port -t | xargs kill -9
        
        # run exp
        iperf3 -s -J -p $bbr_port > "${res_pth}/exp1_bbr_${BDP}_${BBR_num}.json" &
        iperf3 -s -J -p $loss_port > "${res_pth}/exp1_reno_${BDP}_${BBR_num}.json" &     
    elif [[ "$HOST" == *"sender"* ]]; 
        then
        
        # kill processes occupying ports
        lsof -i:$bbr_port -t | xargs kill -9
        lsof -i:$loss_port -t | xargs kill -9
        
        # run exp
        iperf3 -c server -C bbr -p $bbr_port -t 500 -T "BBR" &
        iperf3 -c server -C reno -p $loss_port -t 500 -T "RENO" &        
    fi
fi
