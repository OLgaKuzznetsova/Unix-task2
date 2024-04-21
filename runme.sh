#!/bin/bash
make clean
make

NUM_THREADS=10
PIDS=()
for ((i=0; i<NUM_THREADS; i++)); do
    ./2 myfile.txt &
    PIDS+=($!)
done

sleep 5m

for pid in "${PIDS[@]}"; do
    kill -2 $pid
done
