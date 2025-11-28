# Parallel Dijkstra's Algorithm Implementation

This project implements parallel versions of Dijkstra's shortest path algorithm for a parallel algorithms course assignment. It includes sequential, OpenMP, and MPI implementations.

## Project Structure

```
parallel-algorthims-programming-assignment/
├── src/                    # Source code files
│   ├── graph_generator.c
│   ├── dijkstra_sequential.c
│   ├── dijkstra_openmp.c
│   ├── dijkstra_mpi.c
│   ├── performance_test.c
│   └── Makefile
├── docs/                   # Documentation
│   ├── Report.md          # Analysis report
│   ├── EXECUTION_COMMANDS.md
│   └── Programming Assignment.pdf
├── tests/                  # Test graph files
│   ├── test_assignment_example.txt
│   ├── test1.txt
│   ├── test_medium_500_10000.txt
│   ├── test_large_80000_500000.txt
│   └── custom_test_case.txt
├── scripts/                # Test and comparison scripts
│   ├── test_script.sh
│   └── compare_all.sh
└── build/                  # Compiled executables (created after compilation)
```

## Quick Start

First, navigate to the source directory:

```bash
cd src
```

Then compile everything:

```bash
make
```

This will create all the executables in the `../build/` directory.

To run a quick test:

```bash
# From src directory
make test

# Or manually from project root
./build/dijkstra_sequential tests/test_assignment_example.txt 0
./build/dijkstra_openmp tests/test_assignment_example.txt 0 4
```

## Compilation

From the src directory:

```bash
cd src

# Compile all (OpenMP)
make

# Compile MPI version (requires MPI)
make mpi

# Clean build files
make clean

# Run quick test
make test
```

If you prefer manual compilation:

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

## Testing

Quick test:

```bash
cd src
make test
```

Full test suite:

```bash
cd scripts
chmod +x test_script.sh
./test_script.sh
```

Performance comparison:

```bash
# From project root
./build/performance_test tests/test_assignment_example.txt 4
```

## Project Overview

This project implements three versions of Dijkstra's algorithm:

1. Sequential - Baseline implementation
2. OpenMP - Shared-memory parallelism (primary implementation)
3. MPI - Distributed-memory parallelism (alternative implementation)

All implementations use the same input format (edge list), produce identical results (correctness verified), and can use the same test graph files.

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
- `test_assignment_example.txt` - 5 nodes, 6 edges (matches assignment example)
- `test1.txt` - 2000 nodes, 15000 edges
- `test_medium_500_10000.txt` - 500 nodes, 10000 edges
- `test_large_80000_500000.txt` - 80000 nodes, 500000 edges
- `custom_test_case.txt` - User-generated test cases
- Generate your own with `graph_generator`

Scripts (scripts/):
- `test_script.sh` - Automated testing script
- `compare_all.sh` - Comparison tool for all implementations

## Assignment Requirements

The project includes:
- Sequential Dijkstra implementation
- Parallel Dijkstra (OpenMP)
- Alternative implementation (MPI)
- Graph generator
- Performance testing tools
- Comprehensive documentation
- Execution instructions
- Analysis report with proofs

## Troubleshooting

See `docs/EXECUTION_COMMANDS.md` for:
- Platform-specific setup
- Common errors and solutions
- Compilation issues
