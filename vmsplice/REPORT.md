# vmsplice benchmark report

Question: when does `vmsplice(2)` actually save work compared to a plain
`write(2)` of the same user-memory buffer?

## Setup

- Source: 1 MiB anonymous `mmap` region, memset to `'a'`.
- Total transferred per run: 4 GiB (4096 chunks of 1 MiB).
- Pipe capacity raised to 1 MiB via `F_SETPIPE_SZ`.
- Single-threaded producer (no fork) unless noted.
- Wall time: `clock_gettime(CLOCK_MONOTONIC)`. CPU time: bash `time` builtin.

Paths compared:

- **vmsplice**: `vmsplice(user_mem → pipe)`, then `splice(pipe → sink)`.
- **write** (naive): `write(user_mem → sink)` directly.

## Results

### Sink = regular file (`/tmp/vmsplice_bench.out`)

| | throughput | user | sys | total CPU |
|---|---|---|---|---|
| vmsplice | 4.4 GiB/s | 0.01 s | 0.93 s | 0.94 s |
| write    | 6.4 GiB/s | 0.00 s | 0.64 s | 0.64 s |

`write` wins — both on wall time and CPU.

### Sink = TCP socket (loopback, `recv` process draining with `recv(2)`)

| | throughput | user | sys | total CPU | %CPU |
|---|---|---|---|---|---|
| vmsplice | 7.2 GiB/s | 0.01 s | 0.27 s | 0.28 s | 50% |
| write    | 4.2 GiB/s | 0.00 s | 0.79 s | 0.79 s | 84% |

`vmsplice` wins: ~1.7× throughput and ~3× less CPU per GiB.

## Why the split

`vmsplice` avoids the user→kernel memcpy by *mapping* user pages as pipe
buffer entries rather than copying their contents. Whether that saving
survives all the way to the sink depends on what the sink's
`splice_write` handler does with those pages.

- **File** (`generic_file_splice_write` / `iter_file_splice_write`): copies
  the pipe page into the page cache. The user→kernel memcpy that `vmsplice`
  skipped happens later anyway, plus `vmsplice`+`splice` costs an extra
  syscall and per-page bookkeeping. Net: `vmsplice` slower than `write`.
- **TCP socket**: the kernel's send path takes the pipe page by reference
  via `sendpage` / MSG_SPLICE_PAGES. No copy; the page is attached directly
  to the outgoing sk_buff. Net: `vmsplice` substantially faster and
  cheaper.
- **`/dev/null`**: truly discards — a degenerate "zero-copy" sink useful
  for microbenchmarks but not a realistic target.

The rule of thumb: **`vmsplice` only pays off when the downstream fd can
consume pipe pages without copying them**. That's sockets (and other
pipes), not files.

## Secondary observations

- **`SPLICE_F_GIFT`** gave ~15% lift (4.0 → 4.6 GiB/s, file sink).
  It tells the kernel it can take ownership of the pages, saving some
  refcount/bookkeeping. Must not touch the pages afterward — they belong
  to the kernel.
- **Fork** (producer/consumer on two CPUs) only helps when the consumer
  side is genuinely cheap. With `/dev/null` as sink, fork takes vmsplice
  from ~4 GiB/s to ~20 GiB/s via producer-consumer pipelining. With a
  file sink, fork gives nothing — the page-cache write serialises on
  one side anyway.
- **Naive-with-pipe vs naive-direct**: if you force the `write` path
  through a pipe too (`write → pipe`, `splice → file`), it does *two*
  copies and collapses to ~1 GiB/s. The honest `write` baseline is
  direct, one syscall, one copy.

## Conclusion

`vmsplice`'s marketing ("zero-copy from user memory") is accurate only
when paired with a sink that also avoids copying. Sockets are the canonical
fit. For file output, a boring `write()` is both faster and more
CPU-efficient — which matches the intuition that when source or sink is
a file, `splice` alone (file → pipe → sink, or source → pipe → file)
is the right primitive; `vmsplice` belongs on the network path.
