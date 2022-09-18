#!/bin/bash
while true
do
./client -f mysock -t 500 -D tests/fileEvict/ -W tests/files/file1
./client -f mysock -t 500 -c /tests/files/file1
#./client -f mysock -l /tests/files/30mb_file
done
