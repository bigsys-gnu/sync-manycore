#!/bin/bash

for i in 1 4 8 10 20 30 40 50 60 70 80 90 100 110 1111111111120
do
	time ./bench -n$i
done
