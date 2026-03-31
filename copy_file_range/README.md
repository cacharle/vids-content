# copy_file_range vs read/write on Linux

## What is copy_file_range?

`copy_file_range()` is a Linux syscall (since 4.5) that copies data between two file descriptors entirely in kernel space, avoiding the user-kernel data transfer that a traditional `read()`/`write()` loop requires.

GNU `cp` uses `copy_file_range` by default since coreutils commit [4b04a0c](https://github.com/coreutils/coreutils/commit/4b04a0c3b792d27909670a81d21f2a3b3e0ea563).

## The catch: it depends on the filesystem

Not all filesystems implement `copy_file_range` natively. When a filesystem doesn't, the kernel falls back to a **generic splice-based path** — it creates an internal pipe, splices data from the source into the pipe, then splices from the pipe into the destination.

### Filesystems with native implementations

- **btrfs** — uses its own optimized extent-aware copy. Can also do reflinks (instant copy-on-write clones), but even with reflinks disabled, the native copy path is faster than the generic fallback.
- **XFS** — supports reflinks and has a native implementation.
- **NFS/CIFS** — can do server-side copies.

### Filesystems without native implementations

- **ext4** — has no `.copy_file_range` in its `file_operations` struct ([source](https://github.com/torvalds/linux/blob/master/fs/ext4/file.c)). Falls back to the generic splice path.

## Benchmark results

### ext4 on NVMe (desktop)

Wall clock time: `copy_file_range` is **consistently equal or slightly slower** than `read/write` with a 128KB buffer. The splice/pipe overhead in the generic fallback negates the benefit of avoiding the user-kernel memcpy.

CPU time (user + system): `copy_file_range` uses roughly **half the system time** when the read/write version uses a small (4KB) buffer. With a 128KB buffer (what `cp` uses), the difference shrinks to under 5%.

### btrfs on NVMe (laptop, CoW disabled)

Wall clock time: `copy_file_range` is **10-30% faster** consistently. btrfs has a native implementation that bypasses the generic splice path, even when reflinks/CoW are disabled.

## Why ext4 doesn't implement it

ext4 has no copy acceleration mechanism (no reflinks, no extent sharing). A native implementation would just be a kernel-space read/write loop — similar to what the generic path does, but without the pipe overhead. Nobody has submitted a patch for this, and there's no discussion on the kernel mailing lists about it.

One HN user ([jandrese](https://news.ycombinator.com/item?id=26186577)) benchmarked on ext4 and found no measurable difference. Kernel contributor the8472 confirmed ext4 has no reflink support and thus no acceleration path.

## What this means for cp on ext4

Since `cp` uses `copy_file_range`, it goes through the generic splice fallback on ext4. A plain `read()`/`write()` loop with a 128KB buffer would be equally fast or marginally faster. This affects the majority of Linux desktops and servers, since ext4 is the most common filesystem.

## Files in this repo

- `main.c` — file copy using `read()`/`write()` with a configurable buffer
- `main_with_copy_range.c` — file copy using `copy_file_range()`
- `benchmark.sh` — hyperfine benchmark script, takes a directory as argument
- `plot.py` — generates CPU usage comparison chart from benchmark JSON

## Running

```bash
# Create test files
for size in 1K 100K 1M 100M 200M 500M 1G; do
    dd if=/dev/urandom of=test_$size bs=$size count=1 2>/dev/null
done

# Compile
gcc -O2 -o main main.c
gcc -O2 -o main_with_copy_range main_with_copy_range.c

# Benchmark (current directory)
sudo ./benchmark.sh .

# Benchmark (different filesystem)
sudo ./benchmark.sh /path/to/other/fs

# Plot results
python plot.py
```
