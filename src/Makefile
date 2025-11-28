CC = gcc
MPICC = mpicc
CFLAGS = -Wall -Wextra -O3
OPENMP_FLAGS = -fopenmp
MPI_FLAGS = -O3
BUILD_DIR = ../build

# Default target
all: $(BUILD_DIR)/graph_generator $(BUILD_DIR)/dijkstra_sequential $(BUILD_DIR)/dijkstra_openmp $(BUILD_DIR)/performance_test

# Graph generator
$(BUILD_DIR)/graph_generator: graph_generator.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/graph_generator graph_generator.c

# Sequential Dijkstra
$(BUILD_DIR)/dijkstra_sequential: dijkstra_sequential.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/dijkstra_sequential dijkstra_sequential.c

# Parallel Dijkstra with OpenMP
$(BUILD_DIR)/dijkstra_openmp: dijkstra_openmp.c
	@mkdir -p $(BUILD_DIR)
	@if ! echo "int main(){return 0;}" | $(CC) $(OPENMP_FLAGS) -x c - -o /dev/null 2>/dev/null; then \
		echo "Error: OpenMP support not found in compiler."; \
		echo "Install OpenMP support with: sudo apt install libomp-dev"; \
		echo "Or on Ubuntu/WSL: sudo apt install build-essential libomp-dev mpich -y"; \
		exit 1; \
	fi
	$(CC) $(CFLAGS) $(OPENMP_FLAGS) -o $(BUILD_DIR)/dijkstra_openmp dijkstra_openmp.c

# Performance testing tool
$(BUILD_DIR)/performance_test: performance_test.c
	@mkdir -p $(BUILD_DIR)
	@if ! echo "int main(){return 0;}" | $(CC) $(OPENMP_FLAGS) -x c - -o /dev/null 2>/dev/null; then \
		echo "Error: OpenMP support not found in compiler."; \
		echo "Install OpenMP support with: sudo apt install libomp-dev"; \
		echo "Or on Ubuntu/WSL: sudo apt install build-essential libomp-dev mpich -y"; \
		exit 1; \
	fi
	$(CC) $(CFLAGS) $(OPENMP_FLAGS) -o $(BUILD_DIR)/performance_test performance_test.c

# MPI target (optional, requires MPI installation)
mpi: $(BUILD_DIR)/dijkstra_mpi

# MPI Parallel Dijkstra (requires MPI)
$(BUILD_DIR)/dijkstra_mpi: dijkstra_mpi.c
	@mkdir -p $(BUILD_DIR)
	@if ! command -v mpicc >/dev/null 2>&1; then \
		echo "Error: mpicc not found. MPI is not installed."; \
		echo "Install MPI with: sudo apt install mpich"; \
		echo "Or on Ubuntu/WSL: sudo apt install build-essential libomp-dev mpich -y"; \
		exit 1; \
	fi
	$(MPICC) $(MPI_FLAGS) -o $(BUILD_DIR)/dijkstra_mpi dijkstra_mpi.c

# Clean build artifacts
clean:
	rm -f $(BUILD_DIR)/graph_generator $(BUILD_DIR)/dijkstra_sequential $(BUILD_DIR)/dijkstra_openmp $(BUILD_DIR)/dijkstra_mpi $(BUILD_DIR)/performance_test

# Test target
test: all
	@echo "Running sequential version on assignment test file..."
	$(BUILD_DIR)/dijkstra_sequential ../tests/test_assignment_example.txt 0
	@echo "\nRunning parallel version (OpenMP) on assignment test file..."
	$(BUILD_DIR)/dijkstra_openmp ../tests/test_assignment_example.txt 0 4
	@echo "\nRunning performance comparison..."
	$(BUILD_DIR)/performance_test ../tests/test_assignment_example.txt 4

.PHONY: all clean test mpi
