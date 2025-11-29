# Execution Commands

Project structure:
- parallel-algorthims-programming-assignment/ (project root)
  - src/ (source code)
  - build/ (compiled executables - created after make)
  - tests/ (test graph files)
  - scripts/ (test scripts)

1. Install WSL and packages
   - Run PowerShell as admin
   - wsl --install (reboot if prompted)
   - After Ubuntu opens the first time, update packages
   - cd /<cloned_project_path> (or your project location)
   - sudo apt update

2. Build everything (OpenMP + MPI)
   - cd src
   - make (for OpenMP builds)
   - make mpi (for MPI build)
   - Note: If you get errors about missing OpenMP or MPI, install with the commands shown in the error message

3. Run OpenMP build on the assignment test file
   - From project root: ./build/dijkstra_openmp tests/test_assignment_example.txt 0 4
   - Change the graph file or thread count as needed
   - Note: Must be run from project root, not from src directory

4. Run MPI build on the same test file
   - From project root: mpirun -np 4 ./build/dijkstra_mpi tests/test_assignment_example.txt 0
   - Adjust the graph file or process count as needed
   - Note: Must be run from project root, not from src directory

5. Compare sequential / OpenMP / MPI with helper script
   - From project root: cd scripts
   - chmod +x compare_all.sh
   - ./compare_all.sh ../tests/test_assignment_example.txt 0 4 4

   Also, To test performance and efficiency
   - From project root:
   - ./build/performance_test tests/test_assignment_example.txt 4

6. Run automated regression tests
   - From project root: cd scripts
   - chmod +x test_script.sh
   - ./test_script.sh

7. Generate a new test graph (optional)
   - From project root: ./build/graph_generator <nodes> <edges> <max_weight> tests/<custom_test_file_name>.txt
   - Example: ./build/graph_generator 2000 15000 50 tests/<custom_test_file_name>.txt
   - Test it with OpenMP: ./build/dijkstra_openmp tests/<custom_test_file_name>.txt 0 4
   - Test it with MPI: mpirun -np 4 ./build/dijkstra_mpi tests/<custom_test_file_name>.txt 0

8. Clean and rebuild if needed
   - cd src
   - make clean
   - make
