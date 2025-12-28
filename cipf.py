from collections import Counter
import math
import matplotlib.pyplot as plt

with open("build/lemmas.txt", "r", encoding="utf-8") as f:
    lemmas = [line.strip() for line in f if line.strip()]

freq = Counter(lemmas)
items = sorted(freq.items(), key=lambda x: x[1], reverse=True)

ranks = list(range(1, len(items) + 1))
counts = [c for _, c in items]

filtered = [(r, c) for r, c in zip(ranks, counts) if c >= 2]
ranks_f, counts_f = zip(*filtered) if filtered else (ranks, counts)

log_r = [math.log(r) for r in ranks_f]
log_f = [math.log(c) for c in counts_f]

n = len(log_r)
mx = sum(log_r) / n
my = sum(log_f) / n
b = sum((x-mx)*(y-my) for x, y in zip(log_r, log_f)) / sum((x-mx)**2 for x in log_r)
a = my - b * mx
s = -b

print(f"Zipf exponent s ≈ {s:.3f}")

plt.figure()
plt.plot(log_r, log_f)
plt.xlabel("log(rank)")
plt.ylabel("log(freq)")
plt.title(f"Zipf law (lemmas), s≈{s:.3f}")
plt.grid(True)
plt.show()
