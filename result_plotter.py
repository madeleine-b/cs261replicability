import matplotlib.pyplot as plt
import numpy as np

level_0_y = []
level_1_y = []
level_0_x = []
level_1_x = []

with open("timing_results_level_1_samples_50000.txt", "r") as f:
	for line in f:
		line_split = line.split('[')
		comp_ratio = float(line_split[0])
		timings = [a.replace(']\n', '') for a in line_split[1].split(',')]
		timings = [int(a) for a in timings if a != '']
		if timings:
			level_1_y.append((np.average(timings), len(timings)))
			level_1_x.append(comp_ratio)


with open("timing_results_level_1_samples_50000_old.txt", "r") as f:
	for line in f:
		line_split = line.split('[')
		comp_ratio = float(line_split[0])
		timings = [a.replace(']\n', '') for a in line_split[1].split(',')]
		timings = [int(a) for a in timings if a != '']
		if timings:
			if comp_ratio not in level_1_x:
				level_1_y.append((np.average(timings), len(timings)))
				level_1_x.append(comp_ratio)
			else:
				index = level_1_x.index(comp_ratio)
				timings_entry = level_1_y[index]
				num_prev_timings = timings_entry[1]
				prev_avg = timings_entry[0]
				new_avg = (num_prev_timings * prev_avg + np.sum(timings)) / (num_prev_timings + len(timings))
				level_1_y[index] = (new_avg, num_prev_timings + len(timings))

level_1_y = [entry[0] for entry in level_1_y]
level_1_lists = sorted(zip(*[level_1_x, level_1_y]))
level_1_x, level_1_y = list(zip(*level_1_lists))


with open("timing_results_level_0_samples_50000.txt", "r") as f:
	for line in f:
		line_split = line.split('[')
		comp_ratio = float(line_split[0])
		timings = [a.replace(']\n', '') for a in line_split[1].split(',')]
		timings = [int(a) for a in timings if a != '']
		if timings:
			level_0_y.append((np.average(timings), len(timings)))
			level_0_x.append(comp_ratio)

with open("timing_results_level_0_samples_50000_old.txt", "r") as f:
	for line in f:
		line_split = line.split('[')
		comp_ratio = float(line_split[0])
		timings = [a.replace(']\n', '') for a in line_split[1].split(',')]
		timings = [int(a) for a in timings if a != '']
		if timings:
			if comp_ratio not in level_0_x:
				level_0_y.append((np.average(timings), len(timings)))
				level_0_x.append(comp_ratio)
			else:
				index = level_0_x.index(comp_ratio)
				timings_entry = level_0_y[index]
				num_prev_timings = timings_entry[1]
				prev_avg = timings_entry[0]
				new_avg = (num_prev_timings * prev_avg + np.sum(timings)) / (num_prev_timings + len(timings))
				level_0_y[index] = (new_avg, num_prev_timings + len(timings))

level_0_y = [entry[0] for entry in level_0_y]
level_0_lists = sorted(zip(*[level_0_x, level_0_y]))
level_0_x, level_0_y = list(zip(*level_0_lists))


#plt.errorbar(xs, level_1_y, yerr=level_1_yerr, marker='D', color='#02ccff')
#plt.errorbar(xs, level_0_y, yerr=level_0_yerr, marker='^', color='#008001')

plt.plot(level_1_x, level_1_y, marker='D', color='#02ccff')
plt.plot(level_0_x, level_0_y, marker='^', color='#008001')
plt.grid(linestyle='--')
plt.xticks([0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0])
plt.xlabel("Compression ratio")
plt.ylim(0, 1800)
plt.ylabel("Time (microseconds)")

plt.show()