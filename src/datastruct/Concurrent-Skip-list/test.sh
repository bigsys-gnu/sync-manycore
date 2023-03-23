#!/bin/bash
echo "====================== MV-RLU BENCHMARK ============================"

for i in 1 2 4 8 10 20 30 40 50 60 70 80 90 100
do
    ./rlu/benchmark -r 200000 -o 0 -d 10 -t $i -w zipf
    sleep 1
    echo "======================= " Done $i "=========================="
done
