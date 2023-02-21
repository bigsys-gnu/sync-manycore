#!/bin/bash

echo "====================== RCU BENCHMARK ============================"
make -C ./rcu/ -j

for i in 1 2 4 8 10 20 30 40 50 60 70 80 90 100
do
    ./rcu/benchmark -r 200000 -o 0 -d 10 -t $i -w zipf | tee -a file.txt
    echo "======================= " Done $i "=========================="
done

awk ' /Add/{print $2}' file.txt > rcuadd.txt
awk ' /Remove/{print $2}' file.txt > rcuremove.txt
awk ' /Search/ {print $2}' file.txt > rcusearch.txt
rm file.txt

echo "====================== MV-RLU BENCHMARK ============================"
make -C ./rlu/ -j

for i in 1 2 4 8 10 20 30 40 50 60 70 80 90 100
do
    result=$(./rlu/benchmark -r 200000 -o 0 -d 10 -t $i -w zipf)

    while [ $? -ne 0 ]; do
        echo "Retry Benchmark cause of segfault"
        result=$(./rlu/benchmark -r 200000 -o 0 -d 10 -t $i -w zipf)
    done
    echo "$result" | tee -a file.txt
    echo "======================= " Done $i "=========================="
done

awk ' /Add/{print $2}' file.txt > rluadd.txt
awk ' /Remove/{print $2}' file.txt > rluremove.txt
awk ' /Search/ {print $2}' file.txt > rlusearch.txt
rm file.txt

gnuplot << EOF
        set xlabel "#of threads"
        set ylabel "Operations"
        set xtics ("1" 0, "2" 1, "4" 2, "8" 3, "10" 4, "20" 5, "30" 6, "40" 7, "50" 8, "60" 9, "70" 10, "80" 11, "90" 12, "100" 13)
        set autoscale
        set grid
	plot "rcuremove.txt" with linespoints, "rluremove.txt" with linespoints
        pause mouse key
EOF
