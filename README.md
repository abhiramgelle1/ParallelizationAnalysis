# Parallel Dijkstra's Algorithm Implementation

This project implements parallel versions of Dijkstra's shortest path algorithm. It includes sequential, OpenMP, and MPI implementations.

## Project Structure

```
├── src/                    # Source code files
│   ├── graph_generator.c
│   ├── dijkstra_sequential.c
│   ├── dijkstra_openmp.c
│   ├── dijkstra_mpi.c
│   ├── performance_test.c
│   └── Makefile
├── docs/                  
│   ├── Report.md          
│   ├── EXECUTION_COMMANDS.md
│   └── Programming Assignment.pdf     # Given Assignment taken from iCollege 
├── tests/                  
│   ├── test_assignment_example.txt
│   ├── test1.txt
│   ├── test_medium_500_10000.txt
│   ├── test_large_80000_500000.txt
│   └── custom_test_case.txt
├── scripts/                
│   ├── test_script.sh
│   └── compare_all.sh
└── build/                  # Compiled executables (created after compilation)
```

## Compilation

From the src directory:

```bash
cd src

# Clean build files
make clean

# Compile all (OpenMP)
make

# Compile MPI version (requires MPI)
make mpi

```

Only If you prefer manual compilation: [Otherwise, IGNORE]

```bash
cd src

# Graph generator
gcc -Wall -Wextra -O3 -o ../build/graph_generator graph_generator.c

# Sequential
gcc -Wall -Wextra -O3 -o ../build/dijkstra_sequential dijkstra_sequential.c

# OpenMP
gcc -Wall -Wextra -O3 -fopenmp -o ../build/dijkstra_openmp dijkstra_openmp.c

# Performance test
gcc -Wall -Wextra -O3 -fopenmp -o ../build/performance_test performance_test.c

# MPI (requires MPI installation)
mpicc -O3 -o ../build/dijkstra_mpi dijkstra_mpi.c
```

## Usage Examples

Generate a test graph:

```bash
# From project root
./build/graph_generator 1000 5000 10 tests/my_graph.txt
```

Run the sequential version:

```bash
# From project root
./build/dijkstra_sequential tests/test_assignment_example.txt 0
```

Run OpenMP version with 4 threads:

```bash
# From project root
./build/dijkstra_openmp tests/test_assignment_example.txt 0 4
```

Run MPI version with 4 processes:

```bash
# From project root
mpirun -np 4 ./build/dijkstra_mpi tests/test_assignment_example.txt 0
```

Compare all implementations:

```bash
# From project root
cd scripts
chmod +x compare_all.sh
./compare_all.sh ../tests/test_assignment_example.txt 0 4 4
```

For complete command reference, see `docs/EXECUTION_COMMANDS.md`.

Performance comparison:

```bash
# From project root
./build/performance_test tests/test_assignment_example.txt 4
```

## Project Overview

This project implements three versions of Dijkstra's algorithm:

1. Sequential - Baseline implementation
2. OpenMP - Shared-memory parallelism (primary implementation)
3. MPI - Distributed-memory parallelism (alternative implementation [I haven't used it in performace analysis but can view execution time using commands mentioned above])

## Prerequisites

You'll need:
- GCC with OpenMP support
- Make utility (optional but recommended)
- MPI (optional, for MPI implementation)
  - Ubuntu/WSL: `sudo apt-get install mpich`
  - macOS: `brew install mpich`

## Key Files

Source Code (src/):
- `graph_generator.c` - Generate test graphs
- `dijkstra_sequential.c` - Sequential implementation
- `dijkstra_openmp.c` - OpenMP parallel implementation
- `dijkstra_mpi.c` - MPI parallel implementation
- `performance_test.c` - Performance comparison tool

Documentation (docs/):
- `Report.md` - Detailed analysis with proofs
- `EXECUTION_COMMANDS.md` - Complete command reference

Test Files (tests/):
- `test_assignment_example.txt` - 5 nodes, 6 edges (taken from the example given in assignment. Kept file in /docs/)
- `test1.txt` - 2000 nodes, 15000 edges
- `test_medium_500_10000.txt` - 500 nodes, 10000 edges
- `test_large_80000_500000.txt` - 80000 nodes, 500000 edges
- `custom_test_case.txt` - User-generated test case
- Generate your own with `graph_generator`

Scripts (scripts/):
- `test_script.sh` - Automated testing script
- `compare_all.sh` - Comparison tool for all implementations
