import pyarrow.parquet as pq

table = pq.read_table("/home/charles/Downloads/00-00-00_to_00-14-51.parquet")
pq.write_table(
    table,
    "out.parquet",
    row_group_size=32000,
    compression="ZSTD",
)
