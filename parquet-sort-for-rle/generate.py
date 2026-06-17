import pyarrow.parquet as pq
import pyarrow as pa

in_path = "/home/charles/Downloads/00-00-00_to_00-14-51.parquet"
out_path = "out.parquet"

table = pq.read_table(in_path)
indices = pa.compute.sort_indices(
    table,
    sort_keys=[("broker_name", "ascending"), ("pair_name", "ascending")],
)
table = table.take(indices)

pq.write_table(
    table,
    out_path,
    row_group_size=1024,
    compression="zstd",
    use_dictionary=True,
    data_page_version="2.0",
    write_statistics=True,
)
