#!/bin/bash
make 
rm -f file.lck stat.txt result.txt
printf "Inspection of failure to release the lock file\n"
./runme -f file &
./runme -f file &
rm -f file.lck
sleep 2
killall -SIGINT runme 

rm -f stat.txt
printf "Running 10 process\n"
for i in {1..10}
do
     ./runme -f file &
done
sleep 300
printf "SIGINT - 10 process\n"
killall -SIGINT runme
