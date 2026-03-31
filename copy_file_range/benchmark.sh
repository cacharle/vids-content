#!/bin/bash

DIR="${1:-.}"
FILES="test_1KB test_100KB test_1MB test_100MB test_200MB test_500MB test_1GB"
OUT="$DIR/bench_out"

# Disable CoW on the output file so copy_file_range does a real copy on btrfs
touch "$OUT"
# chattr +C "$OUT"
#; chattr +C $OUT" \

for f in $FILES; do
    hyperfine \
        --warmup 1 \
        --min-runs 10 \
        --prepare "sync; echo 3 | sudo tee /proc/sys/vm/drop_caches > /dev/null; rm -f $OUT; touch $OUT" \
        --export-json "bench_${f}.json" \
        -n "read_write" "./main $DIR/$f $OUT" \
        -n "copy_file_range" "./main_with_copy_range $DIR/$f $OUT"
done

rm -f "$OUT"
