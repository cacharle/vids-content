import json
import matplotlib.pyplot as plt

files = ["test_1KB", "test_100KB", "test_1MB", "test_100MB", "test_200MB", "test_500MB"]
labels = ["1KB", "100KB", "1MB", "100MB", "200MB", "500MB"]

rw_user = []
rw_system = []
cfr_user = []
cfr_system = []

for f in files:
    with open(f"bench_{f}.json") as fh:
        data = json.load(fh)
    for result in data["results"]:
        user_ms = result["user"] * 1000
        system_ms = result["system"] * 1000
        if result["command"] == "read_write":
            rw_user.append(user_ms)
            rw_system.append(system_ms)
        else:
            cfr_user.append(user_ms)
            cfr_system.append(system_ms)

x = range(len(labels))
width = 0.35

fig, ax = plt.subplots(figsize=(10, 6))

bars1_sys = ax.bar([i - width/2 for i in x], rw_system, width, label="read/write (system)", color="#e74c3c")
bars1_usr = ax.bar([i - width/2 for i in x], rw_user, width, bottom=rw_system, label="read/write (user)", color="#f1948a")

bars2_sys = ax.bar([i + width/2 for i in x], cfr_system, width, label="copy_file_range (system)", color="#2ecc71")
bars2_usr = ax.bar([i + width/2 for i in x], cfr_user, width, bottom=cfr_system, label="copy_file_range (user)", color="#82e0aa")

ax.set_xlabel("File Size")
ax.set_ylabel("CPU Time (ms)")
ax.set_title("CPU Usage: read/write vs copy_file_range")
ax.set_xticks(x)
ax.set_xticklabels(labels)
ax.legend()

for bars_bottom, bars_top in [(bars1_sys, bars1_usr), (bars2_sys, bars2_usr)]:
    for b, t in zip(bars_bottom, bars_top):
        total = b.get_height() + t.get_height()
        ax.annotate(f"{total:.1f}",
                    xy=(t.get_x() + t.get_width() / 2, total),
                    xytext=(0, 3), textcoords="offset points",
                    ha="center", va="bottom", fontsize=8)

plt.tight_layout()
plt.savefig("benchmark.png", dpi=150)
print("Saved benchmark.png")
