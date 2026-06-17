import pyarrow.parquet as pq
import os

src = "/home/charles/Downloads/00-00-00_to_00-14-51.parquet"
table = pq.read_table(src)

ts_cols = {"time", "broker_send_time"}
dict_cols = [c for c in table.schema.names if c not in ts_cols]

pq.write_table(
    table,
    "out.parquet",
    use_dictionary=dict_cols,
    column_encoding={c: "DELTA_BINARY_PACKED" for c in ts_cols},
    compression="ZSTD",
)
