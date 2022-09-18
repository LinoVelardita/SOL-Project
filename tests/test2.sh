#!/bin/bash


./server ./tests/config2.txt &
SERVER_PID=$!
./client -p -t 200 -f mysock -D tests/fileEvict -w tests/files

kill -s SIGHUP ${SERVER_PID}

sleep 1
exit 0
