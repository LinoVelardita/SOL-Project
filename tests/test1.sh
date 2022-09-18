#!/bin/bash


valgrind --leak-check=full ./server tests/config1.txt &
SERVER_PID=$!
export SERVER_PID
bash -c 'sleep 3 && kill -s SIGHUP ${SERVER_PID}' &
./client -p -t 200 -f mysock -W tests/files/pic.png,tests/files/file2
./client -p -t 200 -f mysock -d tests/fileLetti/ -R n=0
./client -p -t 200 -f mysock  -l tests/files/pic.png,tests/files/file2 -u tests/files/pic.png,tests/files/file2
./client -p -t 200 -f mysock  -c tests/files/pic.png,tests/files/file2
sleep 1
exit 0
