#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"

INPUT=${INPUT:-/tmp/tee_input}
SIZE_MB=${SIZE_MB:-2048}
RUNS=${RUNS:-5}
NFILES=${NFILES:-4}

if [ ! -f "$INPUT" ]; then
    dd if=/dev/urandom of="$INPUT" bs=1M count="$SIZE_MB" status=none
fi

gcc -O2 -o tee tee.c -lsystemd
gcc -O2 -o tee_naive tee_naive.c

OUTFILES=()
for i in $(seq 1 "$NFILES"); do
    OUTFILES+=("/tmp/tee_out_$i")
done

bench() {
    local prog=$1
    echo "=== $prog ==="
    local samples=()
    # warmup
    cat "$INPUT" | ./"$prog" "${OUTFILES[@]}" > /dev/null
    for r in $(seq 1 "$RUNS"); do
        local t
        t=$({ /usr/bin/bash -c "time cat '$INPUT' | ./$prog ${OUTFILES[*]} > /dev/null"; } 2>&1 | awk '/^real/ {print $2}')
        samples+=("$t")
        echo "  run $r: $t"
    done
    printf "  samples: %s\n" "${samples[*]}"
}

bench tee
bench tee_naive

rm -f "${OUTFILES[@]}"
