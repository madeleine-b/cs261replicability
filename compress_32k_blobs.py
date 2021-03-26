import os.path
import os
import glob
import random
import subprocess
import numpy as np

zlib_level = 1
BYTES_IN_32KB = 1024 * 32
SAMPLE_SIZE = 50000

all_files = [f for f in glob.iglob("data/" + '**/**', recursive=True) if os.stat(f).st_size > BYTES_IN_32KB]
print("There are", len(all_files), "files larger than 32 KB")


result_dict = {}
COMPRESSION_RATIO_TOL = 0.01
for i in np.arange(0.0, 1.01, COMPRESSION_RATIO_TOL):
	result_dict[float(np.round(i, 2))] = []

# "old" is generated with the commented-out sampling method
for file_name in random.sample(all_files, k=SAMPLE_SIZE):
#for i in range(SAMPLE_SIZE):
#	file_name = random.choice(all_files)

	file_bytes = os.stat(file_name).st_size
	random_start_offset = random.choice(range(file_bytes // BYTES_IN_32KB))
	results = subprocess.run(["./zlib_utility", file_name, str(zlib_level), 
		 					 str(random_start_offset), str(-1 + random_start_offset + BYTES_IN_32KB)], 
						 	 check=True, capture_output=True).stdout.split()
	us_duration = int(results[0])
	ratio = float((np.round(float(results[2]), 2)))
	result_dict[ratio].append(us_duration)

result_file_name = "timing_results_level_%d_samples_%d.txt" % (zlib_level, SAMPLE_SIZE)
with open(result_file_name, "w") as f:
	for k, v in result_dict.items():
		f.write(str(k))
		f.write(" ")
		f.write(str(v))
		f.write("\n")