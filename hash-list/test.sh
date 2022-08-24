#!/bin/bash

for i in 1 2 4 8 10 20 30 40 50 60 70 80 90 100 110 120
do
		time ./bench -b 100 -i 1000 -r 2000 -u50 -n$i
done
