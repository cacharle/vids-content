import json
import matplotlib.pyplot as plt

files = ["test_1KB", "test_100KB", "test_1MB", "test_100MB", "test_200MB", "test_500MB"]
labels = ["1KB", "100KB", "1MB", "100MB", "200MB", "500MB"]

rw_times = []
cfr_times = []

for f in files:
    with open(f"bench_{f}.json") as fh:
        data = json.load(fh)
    for result in data["results"]:
        mean_ms = result["mean"] * 1000
        if result["command"] == "read_write":
            rw_times.append(mean_ms)
        else:
            cfr_times.append(mean_ms)

x = range(len(labels))
width = 0.35

fig, ax = plt.subplots(figsize=(10, 6))
bars1 = ax.bar([i - width/2 for i in x], rw_times, width, label="read/write", color="#e74c3c")
bars2 = ax.bar([i + width/2 for i in x], cfr_times, width, label="copy_file_range", color="#2ecc71")

ax.set_xlabel("File Size")
ax.set_ylabel("Time (ms)")
ax.set_title("read/write vs copy_file_range")
ax.set_xticks(x)
ax.set_xticklabels(labels)
ax.legend()

for bar in bars1 + bars2:
    height = bar.get_height()
    ax.annotate(f"{height:.1f}",
                xy=(bar.get_x() + bar.get_width() / 2, height),
                xytext=(0, 3), textcoords="offset points",
                ha="center", va="bottom", fontsize=8)

plt.tight_layout()
plt.savefig("benchmark.png", dpi=150)
print("Saved benchmark.png")
