#!/bin/bash

set -e

IMG_DIR="/tmp"
MNT_DIR="/mnt"
SIZE_MB=2048
FILES="test_1KB test_100KB test_1MB test_100MB test_200MB test_500MB test_1GB"

for fs in ext4 btrfs; do
    img="$IMG_DIR/bench-${fs}.img"
    mnt="$MNT_DIR/bench-${fs}"

    echo "Creating $fs filesystem..."
    dd if=/dev/zero of="$img" bs=1M count=$SIZE_MB status=progress
    mkfs.$fs "$img"

    sudo mkdir -p "$mnt"
    sudo mount -o loop "$img" "$mnt"
    sudo chown "$USER:" "$mnt"

    echo "Copying test files to $mnt..."
    for f in $FILES; do
        cp "$f" "$mnt/"
    done

    echo "$fs ready at $mnt"
done
