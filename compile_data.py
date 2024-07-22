import re
import pandas as pd 
import subprocess

def database_append(database, cmd, prog, algorithm, workload, bufferpool_size, read_ratio, window_size = None): 
    process = subprocess.Popen(cmd, stdout=subprocess.PIPE, universal_newlines=True, shell=True)
    res, _ = process.communicate()
    res_time = prog.search(res)
    result = res_time.group(1) if res_time else -1
    database.append({'algorithm': algorithm,
                    'workload': workload,
                    'execution_time': result,
                    "bufferpool_size": bufferpool_size, 
                    "read_ratio": read_ratio})
    if window_size != None: 
        database[(len(database)-1)]["window_size"] = window_size


def main():
    prog = re.compile(r"Total time: ([0-9]+)")
    database = []
    for algorithm in [0, 1, 2]:
        for workload in ['wl1.data', 'wl2.data']:
            for bufferpool_size in [50, 100, 500, 1000]:
                for read_ratio in [0, 30, 50, 70, 100]:
                    cmd = [
                            "./buffermanager",
                            f"-a {algorithm}", 
                            f"-b {bufferpool_size}", 
                            f"-r {read_ratio}"
                        ]
                    if algorithm != 1: 
                        database_append(database, cmd, prog, algorithm, workload, bufferpool_size, read_ratio)
                    else: 
                        for window_size in [1, 3, 4, 6]: 
                            cmd.append(f"-i {window_size}")
                            database_append(database, cmd, prog, algorithm, workload, bufferpool_size, read_ratio, window_size)


    df = pd.DataFrame(database)
    print(df.head(50))

if __name__ == "__main__":
    main()