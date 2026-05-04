# P-core vs E-core

CPU: i5-12600K (P-cores ~4.7 GHz, E-cores ~3.5 GHz)

## Wall time

| Type | Vectorized | P-core (0) | E-core (14) | P/E ratio |
|------|------------|------------|-------------|-----------|
| `int` (UB)  | no  | 8.54 s |  13.32 s | **1.56x** |
| `int` (UB)  | yes | 2.95 s |  12.94 s | **4.39x** |
| `long`      | no  | 8.37 s |  13.15 s | **1.57x** |
| `long`      | yes | 2.61 s |   6.94 s | **2.66x** |

This CPU's E-cores have 128-bit SIMD units. 256-bit ops split into two ops.
