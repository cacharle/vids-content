#!/bin/bash
set -e

REQUESTS=10000
CONCURRENCY=50

cc -O2 -DPORT=9001 -o server_writev main.c
cc -O2 -DPORT=9002 -o server_naive main_naive.c

bench() {
    local name=$1 bin=$2 port=$3

    ./$bin &
    pid=$!
    sleep 0.2

    echo "=== $name ==="
    ab -n $REQUESTS -c $CONCURRENCY -q "http://127.0.0.1:$port/" 2>&1 \
        | grep -E 'Requests per second|Time per request|Transfer rate|Failed'

    kill $pid 2>/dev/null
    wait $pid 2>/dev/null || true
    echo
}

bench "writev (single syscall)" server_writev 9001
sleep 1
bench "write x2 (two syscalls)" server_naive 9002

rm -f server_writev server_naive
