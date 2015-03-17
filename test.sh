#!/bin/bash
# for i in `seq 1 $j` :
#  do     time ./tcpclient -p 2000 -h localhost -f /file -t ;
# done
j=5
i=0
# while [[ $i -le $j ]]
# do
#     echo -n "$i "
#     ((i = i + 1))
#     sleep 1
# done

for i in `seq 1 10` 
do
    j=$i
    time(for (( c=1; c<=$j; c++ ))
    do
        ./a2/tcpclient -p 1125 -h localhost -f /file -t 
    done)
done
