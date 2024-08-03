import re
import pandas as pd 
import subprocess
import csv

def database_append(cmd, algorithm, bufferpool_size, disk_size, operation): 
    process = subprocess.Popen([cmd], stdout=subprocess.PIPE, universal_newlines=True, shell=True)
    res, _ = process.communicate()
    prog_hit = re.compile(r"Buffer Hit: ([0-9]+)", re.IGNORECASE)
    prog_miss = re.compile(r"Buffer Miss: ([0-9]+)", re.IGNORECASE)
    prog_read_io = re.compile(r"Read IO: ([0-9]+)", re.IGNORECASE)
    prog_write_io = re.compile(r"Write IO: ([0-9]+)", re.IGNORECASE)
    prog_total_time = re.compile(r"Global Clock: ([0-9]*\.[0-9]+)", re.IGNORECASE)
    hit_rate = re.search(prog_hit, res).group(1) if re.search(prog_hit, res) else -1
    miss_rate = re.search(prog_miss, res).group(1) if re.search(prog_miss, res) else -1
    read_io = re.search(prog_read_io, res).group(1) if re.search(prog_read_io, res) else -1
    write_io = re.search(prog_write_io, res).group(1) if re.search(prog_write_io, res) else -1
    total_time = re.search(prog_total_time, res).group(1) if re.search(prog_write_io, res) else -1

    new_row = [algorithm, hit_rate, miss_rate, read_io, write_io, bufferpool_size, disk_size, total_time, operation]
    with open('EvictPol.csv', 'a', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(new_row)


def main():
    headerlist = ["algorithm", "hit_rate", "miss_rate", "read_io", "write_io", "bufferpool_size", "disk_size", "total_time", "num_operations"]
    with open("EvictPol.csv", 'w') as file: 
        df = csv.DictWriter(file, delimiter=',', fieldnames=headerlist) 
        df.writeheader()
    for algorithm in [0, 1, 2]:
        for disk_size in [10000, 20000, 100000]:
                for operation in [100000, 200000, 1000000]:
                    bufferpool_size = 4000 if (disk_size == 10000) else int((disk_size * 0.8))
                    cmd = "./buffermanager -a " + f"{algorithm}" + " -b " + f"{bufferpool_size}" + " -n " + f"{disk_size}" + " -x " + f"{operation}" 
                    database_append(cmd, algorithm, bufferpool_size, disk_size, operation)


if __name__ == "__main__":
    main()