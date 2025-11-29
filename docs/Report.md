# Parallelization of Dijkstra's Algorithm Implementation Report

**Name:** Abhiram Gelle  
**Panther ID:** 002850818  

---

## 1. Parallelization Strategy

I parallelized Dijkstra's algorithm using OpenMP. Since the algorithm processes nodes sequentially, I parallelized two operations within each iteration:

**1. Finding Minimum Node:** Each thread searches a portion of nodes for a local minimum, then threads combine results using a critical section.

**2. Updating Neighbors:** After finding the minimum node, multiple threads update neighbor distances in parallel, using critical sections for thread-safe updates.

**Why OpenMP:** Dijkstra needs frequent shared memory access. OpenMP allows direct access to shared arrays, while MPI would require message passing for every update, creating too much overhead.

---

## 2. Performance Analysis

![Compilation Output](images/compilation_output.png)

### Performance Results Summary

![Performance Test - Small Graph](images/performance_test_small_graph.png)
![Performance Test - Large Graph](images/performance_test_large_graph.png)

| Graph Size | Nodes | Edges | Sequential Time | Parallel Time (4 threads) | Speedup | Efficiency |
|------------|-------|-------|-----------------|--------------------------|---------|------------|
| Small | 5 | 6 | 0.000001s | 0.000291s | 0.0028x | 0.07% |
| Medium | 40,000 | 500,000 | 3.22s | 0.88s | 3.64x | 91.01% |
| Large | 80,000 | 1,000,000 | 13.79s | 4.08s | 3.38x | 84.52% |
| Very Large | 150,000 | 1,000,000 | 51.13s | 15.65s | 3.27x | 81.68% |

### Analysis

**Small Graphs (5 nodes):** Parallel version is 291x slower due to overhead (thread creation, synchronization) exceeding computation time. Efficiency of 0.07% shows parallelization is counterproductive.

**Medium to Very Large Graphs (40K-150K nodes):** Consistent performance improvement:
- Speedup: 3.27x - 3.64x (near-linear for 4 threads)
- Efficiency: 81.68% - 91.01% (excellent utilization)
- Overhead becomes negligible as graph size increases

**Key Observations:**
1. **Break-even point:** Around 40,000 nodes - below this, sequential is faster
2. **Scalability:** Speedup remains consistent (3.2x-3.6x) across different large graph sizes
3. **Efficiency trend:** Slightly decreases with very large graphs (81.68% vs 91.01%) due to increased synchronization overhead
4. **Optimal range:** Medium to large graphs (40K-80K nodes) show best efficiency (84-91%)

### Implementation Comparison

![Implementation Comparison](images/implementation_comparison.png)

All implementations produce identical results, confirming correctness:

| Implementation | Time (test_assignment_example.txt) |
|----------------|-------------------------------------|
| Sequential | 0.000004s |
| OpenMP (4 threads) | 0.001043s |
| MPI (1 process) | 0.000026s |

---

## 3. Challenges and Lessons Learned

**Challenge 1: Thread Safety**
- **Problem:** Race conditions when multiple threads update distances
- **Solution:** Used critical sections for thread-safe updates
- **Lesson:** Thread safety is critical - always use proper synchronization

**Challenge 2: Overhead for Small Graphs**
- **Problem:** Parallel version slower than sequential for small graphs
- **Solution:** Expected behavior - need enough work to justify overhead
- **Lesson:** Test with different sizes - parallelization isn't always faster

**Challenge 3: Limited Parallelism**
- **Problem:** Algorithm is inherently sequential - can't parallelize main loop
- **Solution:** Focused on parallelizing what's possible (min-finding, neighbor updates)
- **Lesson:** Not all algorithms parallelize well - some have fundamental sequential dependencies

**Main Takeaways:**
1. Overhead matters - only parallelize when computation justifies it
2. Graph size matters - large graphs benefit, small graphs don't
3. Correctness first - always verify parallel and sequential produce same results
4. OpenMP was the right choice - shared memory access fits this algorithm better than MPI

---

## Conclusion

I successfully implemented parallel Dijkstra's using OpenMP. The results demonstrate clear patterns:

**For Small Graphs (< 10 nodes):** Sequential is faster. Overhead dominates, making parallelization counterproductive (0.07% efficiency).

**For Medium to Very Large Graphs (40K-150K nodes):** Parallelization provides significant benefits:
- Consistent 3.2x-3.6x speedup with 4 threads
- High efficiency (81-91%), showing effective use of parallel resources
- Best efficiency achieved with medium-large graphs (40K-80K nodes)

**Key Findings:**
1. **Break-even point:** Around 40,000 nodes - parallelization becomes beneficial
2. **Scalability:** Speedup remains consistent across different large graph sizes, indicating good scalability
3. **Efficiency:** Decreases slightly for very large graphs due to increased synchronization overhead
4. **Practical recommendation:** Use parallel version for graphs with higher nodes; use sequential for smaller graphs

The implementation successfully demonstrates that parallelization effectiveness depends on problem size - small problems suffer from overhead, while large problems benefit significantly from parallel execution.
