#!/bin/sh
echo "Making test files"
dd bs=1 count=10 if=/dev/urandom of=small.file
dd bs=1024 count=8 if=/dev/urandom of=large.file