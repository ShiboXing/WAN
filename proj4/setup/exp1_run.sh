#!/bin/bash
help="source exp1_run.sh [queue(# of BDPs)] [# of BBRs]"

# check parameters
if [ -z "$1" ] 
    then
    echo "please enter a BDP number for the bottleneck queue"
elif [ -z "$2" ] 
    then
    echo "please enter the number of BBR flows"
else
    BDP=$1
    BBR_num=$2
    source exp1_setup.sh $1 $2
fi
