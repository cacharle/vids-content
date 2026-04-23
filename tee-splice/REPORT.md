# tee / splice broadcaster

Benchmark comparing a zero-copy `splice + tee` TCP broadcaster against a
`read + write` version that takes the same shape.

## Scenario

One TCP source → N subscriber sockets. Every byte the source sends is
delivered to every subscriber.

```
                  splice
 source (tcp) ------------> main_p --[tee]--> sub_p[0] -----> sub[0]  (splice)
                               \                                 ...
                                \--[tee]----> sub_p[N-2] -----> sub[N-2]
                                 \
                                  \-[splice]------------------> sub[N-1]
```

`main_p` holds each chunk once. `tee()` duplicates the refcounted pipe
pages into each `sub_p` without copying the payload. The last
subscriber consumes `main_p` directly, which drains it.

## Files

- `proxy.c` — splice + tee broadcaster
- `proxy_naive.c` — `read()` into a userspace buffer, then `write()` to
  every subscriber
- `proxy_sink.c` — subscriber: splice from socket to a regular file
- `proxy_client.c` — source: `sendfile()` a file to the broadcaster
- `bench_proxy.sh` — orchestrator: starts N sinks, starts the
  broadcaster, times the client, validates every sink file, prints
  wall-clock + CPU from `getrusage`

Both broadcasters call `getrusage(RUSAGE_SELF)` at exit and print
`user=… sys=… total=…` to stderr so the bench can report CPU time, not
just wall-clock.

## Method

- Source and sinks are C helpers using `sendfile`/`splice` respectively,
  so the *proxy* is the subject under test and isn't bounded by
  `ncat`-style userspace copies in the harness.
- Sink writes go to regular files on disk (btrfs), not `/dev/null` or
  tmpfs, so the splice-to-destination path is not taking a kernel fast
  path (`/dev/null` has a `splice_write_null` that consumes pipe buffers
  without reading them).
- Bench size kept below the pagecache dirty-page budget so disk
  writeback doesn't become the only signal (see caveat below).

## Results

7 runs each, 64 MB input, N = 8 subscribers, 512 MB total fanout per
run:

|                        |  wall  | user CPU | sys CPU | total CPU |
| ---------------------- | -----: | -------: | ------: | --------: |
| `proxy` (splice + tee) | 0.365s |  0.018s  | 0.199s  |  0.217s   |
| `proxy_naive`          | 0.372s |  0.012s  | 0.275s  |  0.287s   |

Splice + tee burns **~24% less CPU**, essentially all of it sys time —
the userspace→kernel copy done by `write()` is gone. Wall time
converges because the SSD caps both versions at roughly the same
writeback throughput.

Fanout scales with N:

| N subs | `proxy` best in | `proxy_naive` best in | relative |
| -----: | --------------: | --------------------: | -------: |
|      4 |       711 MB/s  |            556 MB/s   |  +28%    |
|      8 |       350 MB/s  |            234 MB/s   |  +50%    |

(Best-of-N wall clock at 64 MB, i.e. before disk becomes the wall. CPU
savings of course track the same trend but more smoothly.)

## Caveats

1. **Disk ceiling.** At 1 GB × 4 subs the SSD saturates (~4 GB written
   per run) and both versions converge around 3 s. The splice win is
   invisible here because kernel-copy time is a small fraction of disk
   wait. CPU time still differs, but the wall-clock user sees is
   dominated by I/O.
2. **Wall-clock variance** (up to ~2×) from btrfs writeback flushing.
   CPU time is steady across runs and is the right metric to quote.
3. **Where tee/splice shines**: CPU-bound fanout. Many subscribers,
   fast destinations (sockets, pipes, in-RAM filesystems, live-tap
   tooling), or any setup where the proxy is the bottleneck. With a
   single slow destination the `write()` cost disappears in the wait.
4. **Why not `sendfile`?** `sendfile` goes `fd → fd` with no fanout.
   For N destinations you'd need N `sendfile` calls from the source,
   which requires the source be a seekable file — not a socket.
   `splice` + `tee` are the right primitives when the input is a stream
   and you need to mirror it.

## Reproducing

```sh
SIZE_MB=64 RUNS=7 NUM_SUBS=8 ./bench_proxy.sh
```

Env overrides: `SIZE_MB`, `RUNS`, `NUM_SUBS`, `LISTEN_PORT`,
`SINK_PORT_BASE`, `INPUT` (path to cached urandom input file).
