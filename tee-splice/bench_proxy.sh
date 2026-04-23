#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"

INPUT=${INPUT:-./proxy_input}
SIZE_MB=${SIZE_MB:-1024}
RUNS=${RUNS:-5}
NUM_SUBS=${NUM_SUBS:-4}
LISTEN_PORT=${LISTEN_PORT:-19000}
SINK_PORT_BASE=${SINK_PORT_BASE:-19001}

if [ ! -f "$INPUT" ] || [ "$(stat -c%s "$INPUT")" -ne $((SIZE_MB * 1024 * 1024)) ]; then
    dd if=/dev/urandom of="$INPUT" bs=1M count="$SIZE_MB" status=none
fi

gcc -O2 -o proxy        proxy.c
gcc -O2 -o proxy_naive  proxy_naive.c
gcc -O2 -o proxy_sink   proxy_sink.c
gcc -O2 -o proxy_client proxy_client.c

cleanup_ports() {
    pkill -f './proxy_sink '  2>/dev/null || true
    pkill -f './proxy '       2>/dev/null || true
    pkill -f './proxy_naive ' 2>/dev/null || true
}
trap cleanup_ports EXIT

bench() {
    local prog=$1
    echo "=== $prog (N=$NUM_SUBS subscribers) ==="
    local samples=()
    local user_samples=()
    local sys_samples=()
    for r in $(seq 0 "$RUNS"); do
        local sink_pids=()
        local sub_args=()
        for i in $(seq 0 $((NUM_SUBS - 1))); do
            local port=$((SINK_PORT_BASE + i))
            local out="./proxy_bench.sink_$i"
            ./proxy_sink "$port" "$out" &
            sink_pids+=($!)
            sub_args+=("127.0.0.1:$port")
        done
        for i in $(seq 0 $((NUM_SUBS - 1))); do
            local port=$((SINK_PORT_BASE + i))
            while ! ss -ltn "sport = :$port" | grep -q LISTEN; do sleep 0.01; done
        done

        local cpu_file
        cpu_file=$(mktemp)
        ./"$prog" "$LISTEN_PORT" "${sub_args[@]}" 2>"$cpu_file" &
        local proxy_pid=$!
        while ! ss -ltn "sport = :$LISTEN_PORT" | grep -q LISTEN; do sleep 0.01; done

        local t
        TIMEFORMAT='%R'
        t=$({ time ./proxy_client 127.0.0.1 "$LISTEN_PORT" "$INPUT"; } 2>&1)

        wait "$proxy_pid"
        for pid in "${sink_pids[@]}"; do wait "$pid"; done
        local cpu
        cpu=$(cat "$cpu_file")
        rm -f "$cpu_file"

        local input_sz
        input_sz=$(stat -c%s "$INPUT")
        for i in $(seq 0 $((NUM_SUBS - 1))); do
            local out="./proxy_bench.sink_$i"
            local sz
            sz=$(stat -c%s "$out")
            if [ "$sz" -ne "$input_sz" ]; then
                echo "  sink $i size mismatch: $sz vs $input_sz"
                exit 1
            fi
            rm -f "$out"
        done

        if [ "$r" -eq 0 ]; then
            echo "  warmup: ${t}s  [$cpu]"
        else
            samples+=("$t")
            local u s2
            u=$(echo "$cpu" | awk '{for(i=1;i<=NF;i++) if ($i ~ /^user=/) {sub("user=","",$i); print $i}}')
            s2=$(echo "$cpu" | awk '{for(i=1;i<=NF;i++) if ($i ~ /^sys=/) {sub("sys=","",$i); print $i}}')
            user_samples+=("$u")
            sys_samples+=("$s2")
            echo "  run $r: ${t}s  [$cpu]"
        fi
    done
    local min=${samples[0]}
    for s in "${samples[@]}"; do
        awk -v a="$s" -v b="$min" 'BEGIN { exit !(a+0 < b+0) }' && min=$s
    done
    local total_mb=$((SIZE_MB * NUM_SUBS))
    local mean_wall mean_user mean_sys
    mean_wall=$(printf '%s\n' "${samples[@]}"      | awk '{s+=$1} END{printf "%.3f", s/NR}')
    mean_user=$(printf '%s\n' "${user_samples[@]}" | awk '{s+=$1} END{printf "%.3f", s/NR}')
    mean_sys=$( printf '%s\n' "${sys_samples[@]}"  | awk '{s+=$1} END{printf "%.3f", s/NR}')
    awk -v mb="$SIZE_MB" -v tot="$total_mb" \
        -v tmin="$min" -v twall="$mean_wall" -v tu="$mean_user" -v ts="$mean_sys" \
        'BEGIN {
            printf "  best wall: %.3fs (in %.1f MB/s, fanout %.1f MB/s)\n", tmin, mb/tmin, tot/tmin;
            printf "  mean wall: %.3fs   mean cpu: user=%.3fs sys=%.3fs total=%.3fs\n",
                twall, tu, ts, tu+ts;
        }'
}

bench proxy
bench proxy_naive
