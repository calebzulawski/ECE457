#!/bin/bash
echo "Starting server"
./tcprouter &
echo "Connecting client 0"
nc localhost 1234 > out.txt &
sleep 1
echo "Writing from two clients"
./counter.sh | nc localhost 1234 &
./counter.sh | nc localhost 1234 &