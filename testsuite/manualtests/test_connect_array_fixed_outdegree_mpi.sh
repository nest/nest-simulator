#!/bin/bash
#
#  This file is part of NEST.
#
# Checks the output of the SLI script
# test_connect_array_fixed_outdegree_mpi.sli
# It should be run in the directory that contains the SLI script output files
# (proc_0.tmp, proc_1.tmp, ...)
# Usage: ./test_connect_array_fixed_outdegree_mpi.sh number-of-MPI-processes
#
# Author: Bruno Golosio 2016
#
#

re='^[0-9]+$'
if [ "$#" -ne 1 ] || ! [[ $1 =~ $re ]]; then
    echo "Usage: $0 number-of-MPI-processes"
    exit
fi

NP=$1  # number of MPI processes
N=20   # number of neurons
K=5    # number of outgoing connections per source neuron

:> conn.tmp
for neur in $(seq 1 $N); do  # loop on source neurons
    conn=""
    for p in $(seq 0 $(expr $NP - 1)); do  # loop on MPI processes
        filename=proc_$p.tmp
        if [ ! -f $filename ]; then
            echo "Error: file $filename not found."
            exit
        fi
        # get outgoing neuron connections created in the process p
        conn1=$(cat $filename | grep "^$neur:" | sed "s/$neur://")
        conn="$conn $conn1"  # join all connections of neuron p
    done
    K1=$(echo $conn | tr ' ' '\n' | wc -l)
    if [ "$K1" -ne "$K" ]; then  # check the number of outgoing connections
        echo Error: wrong number of connections.
        exit
    fi
    echo $conn | tr ' ' '\n' | sort -n >> conn.tmp  # sort connections
done
seq 1 $(expr $N \* $K) > conn1.tmp # file containing the sequence 1,...,N*K
conn_diff=$(diff conn.tmp conn1.tmp)  # check connection elements
if [ -n "$conn_diff" ]; then
    echo Error: wrong connections.
    exit
fi
echo OK
