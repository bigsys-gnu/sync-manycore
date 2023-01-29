#!/bin/bash

for i in 1 2 4 8 10 20 30 40 50 60 70 80 90 100
do
        ./benchmark -r 200000 -o 0 -d 10 -t $i >> file.txt

done
awk ' /Add/{print $2}' file.txt >> rcuadd.txt
awk ' /Remove/{print $2}' file.txt >> rcuremove.txt
awk ' /Search/ {print $2}' file.txt >> rcusearch.txt



./graph.sh
