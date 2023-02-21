#!/usr/bin/bash

gnuplot << EOF
        set xlabel "#of threads"
        set ylabel "Operations"
        set xtics ("1" 0, "2" 1, "4" 2, "8" 3, "10" 4, "20" 5, "30" 6, "40" 7, "50" 8, "60" 9, "70" 10, "80" 11, "90" 12, "100" 13)
        set autoscale
        set grid
        plot "rcu$1.txt" with linespoints, "rlu$1.txt" with linespoints
        pause mouse key
EOF
