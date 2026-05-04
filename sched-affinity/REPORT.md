# P-core vs E-core

CPU: i5-12600K (P-cores ~4.7 GHz, E-cores ~3.5 GHz)
Compiled with: `gcc -O2 -fno-tree-vectorize`

## Wall time

|        | P-core (0) | E-core (14) | P/E ratio |
|--------|------------|-------------|-----------|
| Scalar | 8.54 s     | 13.32 s     | **1.56x** |
| AVX2   | 2.95 s     | 12.94 s     | **4.39x** |

## Scalar: more than just clock speed

| Factor      | P-core  | E-core  | Ratio |
|-------------|---------|---------|-------|
| Clock speed | 4.7 GHz | 3.5 GHz | 1.34x |
| IPC         | 4.0     | 3.4     | 1.18x |
| Combined    |         |         | 1.58x ≈ measured 1.56x |

## AVX2: free on P-core, useless on E-core

This CPU's E-cores have 128-bit SIMD units. 256-bit ops split into two ops

| Metric        | P-core AVX2 | E-core AVX2 |
|---------------|-------------|-------------|
| IPC           | 3.9         | 1.2         |
| Backend-bound | 35.4%       | 56.6%       |
| Speedup vs scalar | 2.9x    | 1.03x       |
