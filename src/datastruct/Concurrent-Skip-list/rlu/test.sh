#!/bin/bash

for i in 1 2 4 8 10 20 30 40 50 60 70 80 90 100
do
        ./benchmark -r 200000 -o 0 -d 10 -t $i >> file.txt

done
awk ' /Add/{print $2}' file.txt >> rluadd.txt
awk ' /Remove/{print $2}' file.txt >> rluremove.txt
awk ' /Search/ {print $2}' file.txt >> rlusearch.txt

mv /home/kkm/sync-manycore/src/datastruct/Concurrent-Skip-list/rlu/rluadd.txt /home/kkm/sync-manycore/src/datastruct/Concurrent-Skip-list/rcu/rluadd.txt
mv /home/kkm/sync-manycore/src/datastruct/Concurrent-Skip-list/rlu/rluremove.txt /home/kkm/sync-manycore/src/datastruct/Concurrent-Skip-list/rcu/rluremove.txt
mv /home/kkm/sync-manycore/src/datastruct/Concurrent-Skip-list/rlu/rlusearch.txt /home/kkm/sync-manycore/src/datastruct/Concurrent-Skip-list/rcu/rlusearch.txt
