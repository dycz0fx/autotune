# Autotuning scripts and benchmarks for HAN
## Prerequisite
Open MPI with HAN collective module

numpy 1.8.0

## Compile the benchmarks
```sh
make
```
    
## Steps to run the benchmarks and use python scripts to process the data
Currently, support two ways to generate the dynamic rule files for HAN.
### 1. Exhaustive search
1) Edit the search range in config if needed.

2) Set up the root directory of the autotune project and the output directory in run_es.sh.

3) Execute run_es.sh to run some benchmarks.
```sh
salloc -N 16 --ntasks-per-node 8
./run_es.sh
```
4) Run process_es.py; the first argument is the root directory of the autotune project, and the second argument is the location of the config file.
```sh
python process_es.py ~/.openmpi/autotune/ ./config
```

### 2. Task-based
1) Edit the search range in config if needed.

2) Set up the root directory of the autotune project and the output directory in run_tasks.sh.

3) Execute run_tasks.sh to run some benchmarks.
```sh
./run_tasks.sh
```
4) Run process_tasks.py, the first argument is the root directory of the autotune project, and the second argument is the location of the config file.
```sh
python process_tasks.py ~/.openmpi/autotune/ ./config
```
