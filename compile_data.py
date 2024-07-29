import re
import pandas as pd 
import subprocess

def database_append(database, cmd, algorithm, bufferpool_size, read_ratio, window_size = None): 
    process = subprocess.Popen([cmd], stdout=subprocess.PIPE, universal_newlines=True, shell=True)
    res, _ = process.communicate()
    prog_hit = re.compile(r"Buffer Hit: ([0-9]+)", re.IGNORECASE)
    prog_miss = re.compile(r"Buffer Miss: ([0-9]+)", re.IGNORECASE)
    prog_rp = re.compile(r"Read Percentage: ([0-9]+)", re.IGNORECASE)
    prog_wp = re.compile(r"Write Percentage: ([0-9]+)", re.IGNORECASE)
    prog_read_io = re.compile(r"Read IO: ([0-9]+)", re.IGNORECASE)
    prog_write_io = re.compile(r"Write IO: ([0-9]+)", re.IGNORECASE)
    hit_rate = re.search(prog_hit, res).group(1) if re.search(prog_hit, res) else -1
    miss_rate = re.search(prog_miss, res).group(1) if re.search(prog_miss, res) else -1
    read_perct = re.search(prog_rp, res).group(1) if re.search(prog_rp, res) else -1
    write_perct = re.search(prog_wp, res).group(1) if re.search(prog_wp, res) else -1
    read_io = re.search(prog_read_io, res).group(1) if re.search(prog_read_io, res) else -1
    write_io = re.search(prog_write_io, res).group(1) if re.search(prog_write_io, res) else -1
    database.append({'algorithm': algorithm,
                    'hit_rate': hit_rate,
                    "miss_rate": miss_rate, 
                    "read_percentage" : read_perct, 
                    "write_percentage" : write_perct, 
                    "read_io" : read_io, 
                    "write_io" : write_io, 
                    "bufferpool_size" : bufferpool_size, 
                    "read_ratio" : read_ratio})
    if window_size != None: 
        database[(len(database)-1)]["window_size"] = window_size


def main():
    database = []
    for algorithm in [0, 1, 2]:
        for bufferpool_size in [1000, 3000, 5000, 8000]:
            for read_ratio in [0, 30, 50, 70, 100]:
                cmd = "./buffermanager -a " + f"{algorithm}" + " -b " + f"{bufferpool_size}" + " -r " + f"{read_ratio}"
                if algorithm != 1: 
                    database_append(database, cmd, algorithm, bufferpool_size, read_ratio)
                else: 
                    for window_size in [1, 3, 4, 6]: 
                        cmd_store = cmd
                        cmd += " -i " + f"{window_size}"
                        database_append(database, cmd, algorithm, bufferpool_size, read_ratio, window_size)
                        cmd = cmd_store


    df = pd.DataFrame(database)
    print(df.head(10))
    eviction_policies_csv_data = df.to_csv("EvictPol.csv", index = True)
    print(eviction_policies_csv_data)

if __name__ == "__main__":
    main()