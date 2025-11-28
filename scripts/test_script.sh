#!/bin/bash

# Test script for Parallel Dijkstra's Algorithm
# This script generates test graphs and verifies correctness

echo "=========================================="
echo "Parallel Dijkstra's Algorithm Test Suite"
echo "=========================================="
echo ""

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if programs are compiled
if [ ! -f "../build/graph_generator" ]; then
    echo -e "${RED}Error: graph_generator not found. Please compile first (cd src && make).${NC}"
    exit 1
fi

if [ ! -f "../build/dijkstra_sequential" ]; then
    echo -e "${RED}Error: dijkstra_sequential not found. Please compile first (cd src && make).${NC}"
    exit 1
fi

if [ ! -f "../build/dijkstra_openmp" ]; then
    echo -e "${RED}Error: dijkstra_openmp not found. Please compile first (cd src && make).${NC}"
    exit 1
fi

if [ ! -f "../build/performance_test" ]; then
    echo -e "${RED}Error: performance_test not found. Please compile first (cd src && make).${NC}"
    exit 1
fi

echo -e "${GREEN}All programs compiled successfully!${NC}"
echo ""

# Test 1: Small graph (correctness test)
echo "=========================================="
echo "Test 1: Small Graph (Correctness)"
echo "=========================================="
../build/graph_generator 100 500 10 ../tests/test_small.txt
echo "Running sequential version..."
../build/dijkstra_sequential ../tests/test_small.txt 0 > seq_small.out 2>&1
echo "Running parallel version..."
../build/dijkstra_openmp ../tests/test_small.txt 0 4 > par_small.out 2>&1

# Extract distances and compare
grep "^Node" seq_small.out | sort > seq_distances.txt
grep "^Node" par_small.out | sort > par_distances.txt

if diff -q seq_distances.txt par_distances.txt > /dev/null; then
    echo -e "${GREEN}✓ Correctness test PASSED${NC}"
else
    echo -e "${RED}✗ Correctness test FAILED${NC}"
    echo "Differences:"
    diff seq_distances.txt par_distances.txt
fi
echo ""

# Test 2: Medium graph (performance)
echo "=========================================="
echo "Test 2: Medium Graph (Performance)"
echo "=========================================="
../build/graph_generator 1000 5000 10 ../tests/test_medium.txt
echo "Running performance comparison..."
../build/performance_test ../tests/test_medium.txt 4
echo ""

# Test 3: Large graph (scalability)
echo "=========================================="
echo "Test 3: Large Graph (Scalability)"
echo "=========================================="
../build/graph_generator 5000 25000 50 ../tests/test_large.txt
echo "Testing with different thread counts..."
for threads in 1 2 4 8; do
    echo "--- Testing with $threads threads ---"
    ../build/performance_test ../tests/test_large.txt $threads | grep -E "(Sequential|Parallel|Speedup|Efficiency)"
    echo ""
done

# Cleanup
echo "=========================================="
echo "Cleaning up test files..."
rm -f seq_*.out par_*.out *_distances.txt
echo -e "${GREEN}Test suite completed!${NC}"

