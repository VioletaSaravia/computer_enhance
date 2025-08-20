import sys
import re
import matplotlib.pyplot as plt

# Read piped input
console_output = sys.stdin.read()

# Extract iteration data
pattern = r"\[\d+\]\s+([\d.]+)\s+ms,\s+([\d.\-nan(ind)]+)\s+MB/s"
matches = re.findall(pattern, console_output)

# Parse values and filter out invalid entries
times = []
bandwidths = []
iterations = []

for i, (time_str, bandwidth_str) in enumerate(matches):
    try:
        time_val = round(float(time_str), 6)
        bandwidth_val = float(bandwidth_str)
        times.append(time_val)
        bandwidths.append(bandwidth_val)
        iterations.append(i)
    except ValueError:
        continue  # Skip invalid entries

# Create the plot
fig, ax1 = plt.subplots()
ax1.plot(iterations, times, "b-o", label="Time (ms)")
ax1.set_xlabel("Iteration")
ax1.set_ylabel("Time (ms)", color="b")
ax1.tick_params(axis="y", labelcolor="b")
ax1.set_ylim(0, 1)  # Set Y-axis range for time


ax2 = ax1.twinx()
ax2.plot(iterations, bandwidths, "r-s", label="Bandwidth (MB/s)")
ax2.set_ylabel("Bandwidth (MB/s)", color="r")
ax1.set_ylim(0, 15000)  # Set Y-axis range for time
ax2.tick_params(axis="y", labelcolor="r")

plt.title("Profiler Output: Time and Bandwidth per Iteration")
fig.tight_layout()
plt.show()
