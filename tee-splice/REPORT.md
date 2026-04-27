# tee / splice broadcaster

Benchmark comparing a zero-copy `splice + tee` TCP broadcaster against a
`read + write` version that takes the same shape.

## Scenario

One TCP source → two subscriber sockets. Every byte the source sends is
delivered to both subscribers.

```
                  splice
 source (tcp) -----------> main_p --[tee]----> sub_p --[splice]--> sub0
                              \
                               \-[splice]--------------------------> sub1
```

`main_p` holds each chunk once. `tee()` duplicates the refcounted pipe
pages into `sub_p` without copying the payload. `sub1` consumes
`main_p` directly, which drains it.

Listen port `19000`, subscribers `19001` and `19002` — all hardcoded so
the example carries no argument-parsing noise.

## Files

- `proxy.c` — splice + tee broadcaster
- `proxy_naive.c` — `read()` into a userspace buffer, then `write()` to
  each subscriber
- `bench_proxy.sh` — orchestrator: starts two `ncat` sinks, starts the
  broadcaster, times an `ncat` source feeding the input file in,
  validates each sink file, prints wall-clock + CPU from `getrusage`

Both broadcasters call `getrusage(RUSAGE_SELF)` at exit and print
`user=… sys=… total=…` to stderr so the bench can report CPU time, not
just wall-clock.

## Method

- Source is `ncat 127.0.0.1 19000 < INPUT`; sinks are
  `ncat -l 127.0.0.1 1900{1,2} > OUTFILE`. Both endpoints do their own
  user-space `read`/`write`, so the harness puts a floor under the
  numbers — the *delta* between `proxy` and `proxy_naive` is what the
  bench is measuring, not the absolute throughput.
- Sink writes go to regular files on disk (btrfs), not `/dev/null` or
  tmpfs, so the splice-to-destination path is not taking a kernel fast
  path (`/dev/null` has a `splice_write_null` that consumes pipe buffers
  without reading them).

## Results

7 runs, 64 MB input, 128 MB total fanout per run:

|                        | mean wall | user CPU | sys CPU | total CPU |
| ---------------------- | --------: | -------: | ------: | --------: |
| `proxy` (splice + tee) |   0.046s  |  0.000s  | 0.006s  |  0.006s   |
| `proxy_naive`          |   0.048s  |  0.001s  | 0.016s  |  0.017s   |

Splice + tee burns **~3× less sys CPU**. The userspace→kernel copy
done by `write()` is gone, and that's where the difference lives. Wall
time barely moves because the bench is bounded by `ncat` on both ends,
not by the proxy.

## Caveats

1. **Wall-clock variance** from btrfs writeback flushing. CPU time is
   steady across runs and is the right metric to quote.
2. **Where tee/splice shines**: CPU-bound fanout. Many subscribers,
   fast destinations (sockets, pipes, in-RAM filesystems, live-tap
   tooling), or any setup where the proxy is the bottleneck. With a
   single slow destination the `write()` cost disappears in the wait.
3. **Why not `sendfile`?** `sendfile` goes `fd → fd` with no fanout.
   For multiple destinations you'd need one `sendfile` per sub from
   the source, which requires the source be a seekable file — not a
   socket. `splice` + `tee` are the right primitives when the input
   is a stream and you need to mirror it.

## Reproducing

```sh
SIZE_MB=64 RUNS=7 ./bench_proxy.sh
```

Env overrides: `SIZE_MB`, `RUNS`, `INPUT` (path to cached urandom
input file).
