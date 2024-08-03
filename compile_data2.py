import re
import pandas as pd 
import subprocess
import csv 

def database_append(cmd, algorithm, skew, perct_data, window_size = None): 
    process = subprocess.Popen([cmd], stdout=subprocess.PIPE, universal_newlines=True, shell=True)
    res, _ = process.communicate()
    prog_hit = re.compile(r"Buffer Hit: ([0-9]+)", re.IGNORECASE)
    prog_miss = re.compile(r"Buffer Miss: ([0-9]+)", re.IGNORECASE)
    prog_rp = re.compile(r"Read Percentage: ([0-9]+)", re.IGNORECASE)
    prog_wp = re.compile(r"Write Percentage: ([0-9]+)", re.IGNORECASE)
    prog_read_io = re.compile(r"Read IO: ([0-9]+)", re.IGNORECASE)
    prog_write_io = re.compile(r"Write IO: ([0-9]+)", re.IGNORECASE)
    prog_total_time = re.compile(r"Global Clock: ([0-9]*\.[0-9]+)", re.IGNORECASE)
    hit_rate = re.search(prog_hit, res).group(1) if re.search(prog_hit, res) else -1
    miss_rate = re.search(prog_miss, res).group(1) if re.search(prog_miss, res) else -1
    read_perct = re.search(prog_rp, res).group(1) if re.search(prog_rp, res) else -1
    write_perct = re.search(prog_wp, res).group(1) if re.search(prog_wp, res) else -1
    read_io = re.search(prog_read_io, res).group(1) if re.search(prog_read_io, res) else -1
    write_io = re.search(prog_write_io, res).group(1) if re.search(prog_write_io, res) else -1
    total_time = re.search(prog_total_time, res).group(1) if re.search(prog_write_io, res) else -1

    new_row = [algorithm, hit_rate, miss_rate, read_perct, write_perct, read_io, write_io, total_time, skew, perct_data]
    with open('EvictPol2.csv', 'a', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(new_row)


def main():
    headerlist = ["algorithm", "hit_rate", "miss_rate", "read_percentage", "write_percentage", "read_io", "write_io", "total_time", "skew", "perct_data"]
    with open("EvictPol2.csv", 'w') as file: 
        df2 = csv.DictWriter(file, delimiter=',', fieldnames=headerlist) 
        df2.writeheader()

    for algorithm in [0, 1, 2]:
        for bufferpool_size in [4000]:
            for disk_size in [10000]:
                for read_ratio in [10, 30, 50, 70, 90]:
                    for operation in [1000000]:
                        for skew in [80, 90, 95, 100]:
                            if skew == 100:
                                amount_data = 100
                            else:
                                amount_data = 100 - skew
                            cmd = "./buffermanager -a " + f"{algorithm}" + " -b " + f"{bufferpool_size}" + " -r " + f"{read_ratio}" + " -n " + f"{disk_size}" + " -x " + f"{operation}" + " -s " + f"{skew}" + " -d " + f"{amount_data}"
                            database_append(cmd, algorithm, skew, amount_data)


if __name__ == "__main__":
    main()