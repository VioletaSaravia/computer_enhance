import json
import sys
import numpy as np

length: int = int(sys.argv[1]) if len(sys.argv) > 1 else exit()

mult = np.zeros(4, dtype=np.float32)
mult[0] = 200
mult[1] = 160
mult[2] = 200
mult[3] = 160
input_uniform = np.random.random_sample((length, 4)) * mult - mult / 2

with open("homework/part2/input_uniform.json", "w", encoding="utf-8") as f:
    json.dump(
        {
            "pairs": [
                {"x0": row[0], "y0": row[1], "x1": row[2], "y1": row[3]}
                for row in input_uniform
            ]
        },
        indent=2,
        fp=f,
    )


def generate_smooth_noise_tuples(N, scale=0.1):
    x = np.linspace(0, 1, N)
    noise_base = np.sin(2 * np.pi * x) + np.random.normal(0, scale, N)
    result = []

    for i in range(N):
        a = noise_base[i]
        b = np.sin(2 * np.pi * (x[i] + 0.25)) + np.random.normal(0, scale)
        c = np.sin(2 * np.pi * (x[i] + 0.5)) + np.random.normal(0, scale)
        d = np.sin(2 * np.pi * (x[i] + 0.75)) + np.random.normal(0, scale)
        result.append((a, b, c, d))

    return result


input_smooth = generate_smooth_noise_tuples(length, 80)
with open("homework/part2/input_smooth.json", "w", encoding="utf-8") as f:
    json.dump(
        {
            "pairs": [
                {"x0": row[0], "y0": row[1], "x1": row[2], "y1": row[3]}
                for row in input_smooth
            ]
        },
        indent=2,
        fp=f,
    )
