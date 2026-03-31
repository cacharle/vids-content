#!/bin/bash

FILES="test_1KB test_100KB test_1MB test_100MB test_200MB test_500MB"
OUT="bench_out"

# Disable CoW on the output file so copy_file_range does a real copy on btrfs
touch "$OUT"
chattr +C "$OUT"

for f in $FILES; do
    hyperfine \
        --warmup 1 \
        --runs 5 \
        --prepare "sync; echo 3 | sudo tee /proc/sys/vm/drop_caches > /dev/null; rm -f $OUT; touch $OUT; chattr +C $OUT" \
        --export-json "bench_${f}.json" \
        -n "read_write" "./main $f $OUT" \
        -n "copy_file_range" "./main_with_copy_range $f $OUT"
done

rm -f "$OUT"
