# Huge pages benchmark

1 GiB anonymous `mmap`, 1,000,000,000 random single-byte writes, `MAP_POPULATE` on all three (page faults excluded from the measurement).

## Reserving huge pages (as root)

2 MiB pages (e.g. 600 pages ≈ 1.2 GiB):

```sh
sudo sysctl -w vm.nr_hugepages=600
```

1 GiB pages (needs 1 GiB of contiguous physical memory — usually reserved at boot via `default_hugepagesz=1G hugepagesz=1G hugepages=N` on the kernel cmdline):

```sh
echo 1 | sudo tee /sys/kernel/mm/hugepages/hugepages-1048576kB/nr_hugepages
```

Verify:

```sh
grep Huge /proc/meminfo
```

## Where the TLB sits

```
  CPU core ──▶ virtual addr ──▶ [ TLB ] ──hit──▶ physical addr ──▶ cache/RAM
                                   │
                                   miss
                                   ▼
                             [ page walker ]
                                   │
                                   ▼
                        4-level page table in RAM
```

### TLB reach vs 1 GiB working set (512-entry L1 dTLB)

- **4 KiB pages** → TLB covers 512 × 4 KiB = **2 MiB** (0.2% of working set) → constant misses.
- **2 MiB pages** → TLB covers 512 × 2 MiB = **1 GiB** (100%) → essentially no misses.
- **1 GiB pages** → 1 entry covers everything + shorter walk on the rare miss.

## Wall-clock time

| Version | Page size | Time | ns/write |
|---|---|---|---|
| `naive.c` | 4 KiB | 10.10 s | 10.1 |
| `with_huge_pages.c` | 2 MiB | 9.63 s | 9.6 |
| `with_huge_pages_1gb.c` | 1 GiB | 8.64 s | 8.6 |

## `perf stat` — P-core (`cpu_core`) counters

| Counter | 4 KiB | 2 MiB | 1 GiB |
|---|---:|---:|---:|
| dTLB-loads | 36,377,550,333 | 36,085,310,971 | 36,068,031,832 |
| dTLB-load-misses | 352,753 | 9,313 | 506 |
| dTLB-stores | 21,204,481,487 | 21,048,385,057 | 21,038,116,271 |
| **dTLB-store-misses** | **993,542,504** | **74,337** | **35,225** |
| cycles | 49,124,787,522 | 44,390,082,517 | 42,022,594,805 |
| instructions | 116,418,583,669 | 115,222,435,919 | 115,165,618,225 |

## Memory footprint (`/proc/[pid]/status`)

| Version | VmRSS | HugetlbPages | **VmPTE** |
|---|---:|---:|---:|
| 4 KiB | 1,050,004 kB | 0 | **2,092 kB** |
| 2 MiB | 1,460 kB | 1,048,576 kB | **44 kB** |
| 1 GiB | 1,424 kB | 1,048,576 kB | **40 kB** |

The payload is ~1 GiB in every case, but the page table itself (`VmPTE`) shrinks by ~50×. With 4 KiB pages, mapping 1 GiB needs 262,144 leaf PTEs (~2 MiB of page-table memory per process); with 2 MiB pages only 512; with 1 GiB pages just 1. Huge-page memory is accounted in `HugetlbPages` instead of `VmRSS`, which is why the naive version's RSS looks large while the huge-page RSS is nearly empty — the underlying data size is identical.

## Takeaways

- `dTLB-store-misses` drops by ~13,000× going from 4 KiB to 2 MiB pages, and another ~2× from 2 MiB to 1 GiB. The 1 GiB working set (~1 GiB × ~1 byte per random write) fits entirely in the L1 dTLB once pages are 2 MiB (512 entries × 2 MiB = 1 GiB covered).
- Wall-clock savings are much smaller than the miss-ratio suggests (~15% total). Modern cores overlap page walks with out-of-order execution and have a page-walk cache, so a TLB miss isn't a full stall.
- Going from 2 MiB → 1 GiB mainly saves on the remaining page-walk cost and cuts one level off the walk (3 instead of 4), giving a further ~1 ns/write.
- Instruction count is identical across versions — the speedup comes entirely from cycles/instruction improvements caused by fewer TLB-induced stalls.
