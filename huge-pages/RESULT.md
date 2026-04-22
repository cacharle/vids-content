# Huge pages benchmark

1 GiB anonymous `mmap`, 1,000,000,000 random single-byte writes, `MAP_POPULATE` on all three (page faults excluded from the measurement).

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

## Takeaways

- `dTLB-store-misses` drops by ~13,000× going from 4 KiB to 2 MiB pages, and another ~2× from 2 MiB to 1 GiB. The 1 GiB working set (~1 GiB × ~1 byte per random write) fits entirely in the L1 dTLB once pages are 2 MiB (512 entries × 2 MiB = 1 GiB covered).
- Wall-clock savings are much smaller than the miss-ratio suggests (~15% total). Modern cores overlap page walks with out-of-order execution and have a page-walk cache, so a TLB miss isn't a full stall.
- Going from 2 MiB → 1 GiB mainly saves on the remaining page-walk cost and cuts one level off the walk (3 instead of 4), giving a further ~1 ns/write.
- Instruction count is identical across versions — the speedup comes entirely from cycles/instruction improvements caused by fewer TLB-induced stalls.
