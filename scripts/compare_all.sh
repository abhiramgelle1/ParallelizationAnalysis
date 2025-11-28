#!/bin/bash

# Script to compare Sequential, OpenMP, and MPI implementations
# Usage: ./compare_all.sh <graph_file> [source_node] [openmp_threads] [mpi_processes]

GRAPH_FILE=$1
SOURCE_NODE=${2:-0}
OMP_THREADS=${3:-4}
MPI_PROCS=${4:-4}

if [ -z "$GRAPH_FILE" ]; then
    echo "Usage: $0 <graph_file> [source_node] [openmp_threads] [mpi_processes]"
    echo "Example: $0 test_graph_small.txt 0 4 4"
    exit 1
fi

if [ ! -f "$GRAPH_FILE" ]; then
    echo "Error: Graph file '$GRAPH_FILE' not found"
    exit 1
fi

echo "=========================================="
echo "Performance Comparison: All Implementations"
echo "=========================================="
echo "Graph file: $GRAPH_FILE"
echo "Source node: $SOURCE_NODE"
echo "OpenMP threads: $OMP_THREADS"
echo "MPI processes: $MPI_PROCS"
echo ""

# Get graph info
if [ -f "$GRAPH_FILE" ]; then
    read nodes edges < <(head -1 "$GRAPH_FILE")
    echo "Graph: $nodes nodes, $edges edges"
    echo ""
fi

# Sequential
echo "1. Sequential Dijkstra:"
if [ -f "../build/dijkstra_sequential" ]; then
    ../build/dijkstra_sequential "$GRAPH_FILE" "$SOURCE_NODE" > seq_temp.out 2>&1
    TIME_SEQ=$(grep "execution time" seq_temp.out | grep -oE "[0-9]+\.[0-9]+")
    echo "   Time: ${TIME_SEQ} seconds"
    grep "^Node" seq_temp.out | head -5
    echo "   ..."
else
    echo "   Error: dijkstra_sequential not found"
    TIME_SEQ="N/A"
fi
echo ""

# OpenMP
echo "2. OpenMP Dijkstra ($OMP_THREADS threads):"
if [ -f "../build/dijkstra_openmp" ]; then
    ../build/dijkstra_openmp "$GRAPH_FILE" "$SOURCE_NODE" "$OMP_THREADS" > omp_temp.out 2>&1
    TIME_OMP=$(grep "execution time" omp_temp.out | grep -oE "[0-9]+\.[0-9]+")
    echo "   Time: ${TIME_OMP} seconds"
    if [ "$TIME_SEQ" != "N/A" ] && [ -n "$TIME_SEQ" ] && [ -n "$TIME_OMP" ]; then
        SPEEDUP_OMP=$(echo "scale=4; $TIME_SEQ / $TIME_OMP" | bc)
        EFF_OMP=$(echo "scale=2; $SPEEDUP_OMP / $OMP_THREADS * 100" | bc)
        echo "   Speedup: ${SPEEDUP_OMP}x"
        echo "   Efficiency: ${EFF_OMP}%"
    fi
    grep "^Node" omp_temp.out | head -5
    echo "   ..."
    
    # Verify correctness
    if [ -f seq_temp.out ]; then
        grep "^Node" seq_temp.out | sort > seq_dist.txt
        grep "^Node" omp_temp.out | sort > omp_dist.txt
        if diff -q seq_dist.txt omp_dist.txt > /dev/null; then
            echo "   Correctness: PASSED"
        else
            echo "   Correctness: FAILED"
        fi
        rm -f seq_dist.txt omp_dist.txt
    fi
else
    echo "   Error: dijkstra_openmp not found"
    TIME_OMP="N/A"
fi
echo ""

# MPI
echo "3. MPI Dijkstra ($MPI_PROCS processes):"
if [ -f "../build/dijkstra_mpi" ]; then
    if command -v mpirun &> /dev/null; then
        mpirun -np "$MPI_PROCS" ../build/dijkstra_mpi "$GRAPH_FILE" "$SOURCE_NODE" > mpi_temp.out 2>&1
        TIME_MPI=$(grep "execution time" mpi_temp.out | grep -oE "[0-9]+\.[0-9]+")
        echo "   Time: ${TIME_MPI} seconds"
        if [ "$TIME_SEQ" != "N/A" ] && [ -n "$TIME_SEQ" ] && [ -n "$TIME_MPI" ]; then
            SPEEDUP_MPI=$(echo "scale=4; $TIME_SEQ / $TIME_MPI" | bc)
            EFF_MPI=$(echo "scale=2; $SPEEDUP_MPI / $MPI_PROCS * 100" | bc)
            echo "   Speedup: ${SPEEDUP_MPI}x"
            echo "   Efficiency: ${EFF_MPI}%"
        fi
        grep "^Node" mpi_temp.out | head -5
        echo "   ..."
        
        # Verify correctness
        if [ -f seq_temp.out ]; then
            grep "^Node" seq_temp.out | sort > seq_dist.txt
            grep "^Node" mpi_temp.out | sort > mpi_dist.txt
            if diff -q seq_dist.txt mpi_dist.txt > /dev/null; then
                echo "   Correctness: PASSED"
            else
                echo "   Correctness: FAILED"
            fi
            rm -f seq_dist.txt mpi_dist.txt
        fi
    else
        echo "   Error: mpirun not found. Install MPI (mpich or openmpi)"
        TIME_MPI="N/A"
    fi
else
    echo "   Error: dijkstra_mpi not found. Compile with: make mpi"
    TIME_MPI="N/A"
fi
echo ""

# Summary
echo "=========================================="
echo "Summary"
echo "=========================================="
printf "%-20s %15s\n" "Implementation" "Time (seconds)"
echo "----------------------------------------"
printf "%-20s %15s\n" "Sequential" "${TIME_SEQ:-N/A}"
printf "%-20s %15s\n" "OpenMP ($OMP_THREADS threads)" "${TIME_OMP:-N/A}"
printf "%-20s %15s\n" "MPI ($MPI_PROCS processes)" "${TIME_MPI:-N/A}"
echo ""

# Cleanup
rm -f seq_temp.out omp_temp.out mpi_temp.out 2>/dev/null

echo "Comparison complete!"

