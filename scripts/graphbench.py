import json
import re
import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path


def parse_benchmark_files(file_paths: list[str]):
    all_results = []
    
    # Regex to extract: operation_name, type, and N
    # Matches: BM_GenericRecursiveAVLTreeInsertion<u64>/1024/repeats:2
    #          BM_StdUnorderedMapInsertion<uint64_t>/1024/repeats:2
    #          BM_StdSetDeletion<uint64_t>/1024/repeats:2
    pattern = re.compile(r"BM_(?P<op>\w+)<(?P<type>.*?)>/(?P<n>\d+)")


    # Check if file paths given are incorrect
    # print failure to the screen before starting any work.
    countNotFiles = 0
    for file_path in file_paths:
        file_path = Path(file_path)
        if not file_path.is_file():
            print("Filepath {} was not found. Either the path is incorrect or the file doesn't exist\n".format(file_path))
            countNotFiles += 1
        
    if countNotFiles > 0:
        print("Some filepaths were incorrect/not found. stopping execution")
        return None


    for file_path in file_paths:
        with open(file_path, 'r') as f:
            data = json.load(f)
            
            for run in data.get('benchmarks', []):
                # We only want 'mean' aggregates to avoid plotting raw repetitions
                if run.get('run_type') == 'aggregate' and run.get('aggregate_name') == 'mean':
                    match = pattern.search(run['name'])
                    if match:                        
                        all_results.append({ # type: ignore
                            'Operation': match.group('op'),
                            'TestType': match.group('op'),
                            'Type': match.group('type'),
                            'N': int(match.group('n')),
                            'RealTime': run['real_time'],
                            'TimeUnit': run['time_unit'],
                            'Source': Path(file_path).stem
                        })
    
    return pd.DataFrame(all_results)


def plot_results(df: pd.DataFrame):
    df['TestType'] = df['Operation'].str.extract('(Insertion|Deletion|Search)', expand=False)
    data_types = df['Type'].unique()
    test_types = df['TestType'].unique()

    print(data_types)
    print(test_types)
    print(df)
    print(df.to_string())
    for dtype in data_types:
        for ttype in test_types:
            subset = df[(df['Type'] == dtype) & (df['TestType'] == ttype)]
            plt.figure(figsize=(10, 6))
            
            # Group by operation and source file
            for label, group in subset.groupby(['TestType', 'Source']):
                group = group.sort_values('N')
                plt.plot(group['N'], group['RealTime'], marker='o', label=f"{label[1]}")

            

            plt.title(f"Performance Benchmark - TestType: {ttype} DataType: {dtype}", loc='left')
            plt.xlabel("Size (N)")
            plt.ylabel(f"Real Time ({subset['TimeUnit'].iloc[0]})")
            plt.xscale('linear')
            plt.yscale('linear')
            plt.grid(True, which="both", ls="-", alpha=0.5)
            plt.legend()
            plt.tight_layout()
            plt.show()



if __name__ == "__main__":
    # fileList = [
    #     'results_avltree_c_generic.json',
    #     'results_avltree_cpp_generic_recursive.json',
    #     'results_avltree_cpp_generic.json',
    #     'results_avltree_cpp.json',
    #     'results_std_set_generic.json',
    #     'results_std_unordered_set.json'
    # ]
    fileList = []
    
    JsonDirToScan = Path('data')
    for file in JsonDirToScan.iterdir():
        if file.is_file():
            fileList.append(file)

    df = parse_benchmark_files(fileList)

    if df is None:
        print("Error Processing File List")
    else:
        plot_results(df)