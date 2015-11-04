#!/bin/sh
smallfile='dd bs=1 count=10 if=/dev/urandom of=small.file'
largefile='dd bs=1 count=10000 if=/dev/urandom of=large.file'

if [ -e ./small.file ]
	then
	rm small.file
fi

if [ -e ./large.file ]
	then
	rm large.file
fi

echo "Running test A\n"
eval $smallfile
./mtests A
rm small.file
echo "\n\n"

echo "Running test B\n"
eval $largefile
./mtests B
rm large.file
echo "\n\n"

echo "Running test C\n"
eval $largefile
./mtests C
rm large.file
echo "\n\n"

echo "Running test D\n"
eval $largefile
./mtests D
rm large.file
echo "\n\n"

echo "Running test E\n"
eval $largefile
./mtests E
rm large.file
echo "\n\n"

echo "Running test F\n"
eval $smallfile
./mtests F
rm small.file