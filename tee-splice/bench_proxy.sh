#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"

INPUT=${INPUT:-./proxy_input}
SIZE_MB=${SIZE_MB:-1024}
RUNS=${RUNS:-5}
SIZE=$((SIZE_MB << 20))

[ -f "$INPUT" ] && [ "$(stat -c%s "$INPUT")" -eq "$SIZE" ] || \
    dd if=/dev/urandom of="$INPUT" bs=1M count="$SIZE_MB" status=none

gcc -O2 -o proxy       proxy.c
gcc -O2 -o proxy_naive proxy_naive.c

trap 'pkill -P $$ 2>/dev/null || true' EXIT
TIMEFORMAT='%R'

wait_port() { while ! ss -ltn "sport = :$1" | grep -q LISTEN; do sleep 0.01; done; }

bench() {
    echo "=== $1 ==="
    : > runs
    for r in $(seq 1 "$RUNS"); do
        ncat -l 127.0.0.1 19001 > sink0 &
        ncat -l 127.0.0.1 19002 > sink1 &
        wait_port 19001; wait_port 19002
        ./"$1" 2>cpu &
        wait_port 19000
        wall=$({ time ncat 127.0.0.1 19000 < "$INPUT"; } 2>&1)
        wait
        [ "$(stat -c%s sink0)" -eq "$SIZE" ] && [ "$(stat -c%s sink1)" -eq "$SIZE" ] \
            || { echo "  run $r: size mismatch"; exit 1; }
        echo "  run $r: wall=${wall}s $(<cpu)" | tee -a runs
    done
    awk '{ for (i=1; i<=NF; i++) if (split($i, a, "=") == 2) s[a[1]] += a[2]+0 }
         END { printf "  mean:  wall=%.3fs user=%.3fs sys=%.3fs total=%.3fs\n", \
                      s["wall"]/NR, s["user"]/NR, s["sys"]/NR, s["total"]/NR }' runs
    rm -f sink0 sink1 cpu runs
}

bench proxy_naive
bench proxy
