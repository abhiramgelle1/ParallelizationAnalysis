#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>
#include <time.h>
#include <omp.h>

#define INF INT_MAX

typedef struct {
    int dest;
    int weight;
} Edge;

typedef struct {
    int num_nodes;
    int num_edges;
    Edge **adj_list;
    int *adj_size;
} Graph;

Graph* create_graph(int num_nodes, int num_edges) {
    Graph *graph = (Graph*)malloc(sizeof(Graph));
    graph->num_nodes = num_nodes;
    graph->num_edges = num_edges;
    graph->adj_list = (Edge**)malloc(num_nodes * sizeof(Edge*));
    graph->adj_size = (int*)calloc(num_nodes, sizeof(int));
    
    for (int i = 0; i < num_nodes; i++) {
        graph->adj_list[i] = (Edge*)malloc(sizeof(Edge));
    }
    
    return graph;
}

void add_edge(Graph *graph, int u, int v, int weight) {
    int u_size = graph->adj_size[u];
    int v_size = graph->adj_size[v];
    
    graph->adj_list[u] = (Edge*)realloc(graph->adj_list[u], (u_size + 1) * sizeof(Edge));
    graph->adj_list[u][u_size].dest = v;
    graph->adj_list[u][u_size].weight = weight;
    graph->adj_size[u]++;
    
    graph->adj_list[v] = (Edge*)realloc(graph->adj_list[v], (v_size + 1) * sizeof(Edge));
    graph->adj_list[v][v_size].dest = u;
    graph->adj_list[v][v_size].weight = weight;
    graph->adj_size[v]++;
}

void free_graph(Graph *graph) {
    for (int i = 0; i < graph->num_nodes; i++) {
        free(graph->adj_list[i]);
    }
    free(graph->adj_list);
    free(graph->adj_size);
    free(graph);
}

// Sequential Dijkstra's algorithm
void dijkstra_sequential(Graph *graph, int source, int *distances) {
    bool *visited = (bool*)calloc(graph->num_nodes, sizeof(bool));
    
    for (int i = 0; i < graph->num_nodes; i++) {
        distances[i] = INF;
    }
    distances[source] = 0;
    
    for (int count = 0; count < graph->num_nodes; count++) {
        int min_dist = INF;
        int min_node = -1;
        
        for (int v = 0; v < graph->num_nodes; v++) {
            if (!visited[v] && distances[v] < min_dist) {
                min_dist = distances[v];
                min_node = v;
            }
        }
        
        if (min_node == -1) {
            break;
        }
        
        visited[min_node] = true;
        
        for (int i = 0; i < graph->adj_size[min_node]; i++) {
            int neighbor = graph->adj_list[min_node][i].dest;
            int weight = graph->adj_list[min_node][i].weight;
            
            if (!visited[neighbor] && distances[min_node] != INF) {
                int new_dist = distances[min_node] + weight;
                if (new_dist < distances[neighbor]) {
                    distances[neighbor] = new_dist;
                }
            }
        }
    }
    
    free(visited);
}

// Parallel Dijkstra's algorithm
void dijkstra_parallel(Graph *graph, int source, int *distances, int num_threads) {
    bool *visited = (bool*)calloc(graph->num_nodes, sizeof(bool));
    
    #pragma omp parallel for num_threads(num_threads)
    for (int i = 0; i < graph->num_nodes; i++) {
        distances[i] = INF;
    }
    distances[source] = 0;
    
    for (int count = 0; count < graph->num_nodes; count++) {
        int min_dist = INF;
        int min_node = -1;
        
        #pragma omp parallel num_threads(num_threads)
        {
            int local_min_dist = INF;
            int local_min_node = -1;
            
            #pragma omp for nowait
            for (int v = 0; v < graph->num_nodes; v++) {
                if (!visited[v] && distances[v] < local_min_dist) {
                    local_min_dist = distances[v];
                    local_min_node = v;
                }
            }
            
            #pragma omp critical
            {
                if (local_min_dist < min_dist) {
                    min_dist = local_min_dist;
                    min_node = local_min_node;
                }
            }
        }
        
        if (min_dist == INF || min_node == -1) {
            break;
        }
        
        visited[min_node] = true;
        
        int adj_size = graph->adj_size[min_node];
        int base_dist = distances[min_node];
        
        #pragma omp parallel for num_threads(num_threads)
        for (int i = 0; i < adj_size; i++) {
            int neighbor = graph->adj_list[min_node][i].dest;
            int weight = graph->adj_list[min_node][i].weight;
            
            if (!visited[neighbor] && base_dist != INF) {
                int new_dist = base_dist + weight;
                #pragma omp critical
                {
                    if (new_dist < distances[neighbor]) {
                        distances[neighbor] = new_dist;
                    }
                }
            }
        }
    }
    
    free(visited);
}

bool verify_results(int *dist1, int *dist2, int num_nodes) {
    for (int i = 0; i < num_nodes; i++) {
        if (dist1[i] != dist2[i]) {
            return false;
        }
    }
    return true;
}

Graph* read_graph_from_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }
    
    int first = fgetc(file);
    int second = fgetc(file);
    int third = fgetc(file);
    
    if (first == 0xEF && second == 0xBB && third == 0xBF) {
    } else {
        rewind(file);
    }
    
    int num_nodes, num_edges;
    fscanf(file, "%d %d", &num_nodes, &num_edges);
    
    Graph *graph = create_graph(num_nodes, num_edges);
    
    for (int i = 0; i < num_edges; i++) {
        int u, v, weight;
        fscanf(file, "%d %d %d", &u, &v, &weight);
        add_edge(graph, u, v, weight);
    }
    
    fclose(file);
    return graph;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input_file> [num_threads]\n", argv[0]);
        fprintf(stderr, "Example: %s weighted_graph.txt 4\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    const char *filename = argv[1];
    int num_threads = (argc >= 3) ? atoi(argv[2]) : 4;
    int source = 0;
    
    if (num_threads <= 0) {
        num_threads = 4;
    }
    
    omp_set_num_threads(num_threads);
    
    Graph *graph = read_graph_from_file(filename);
    if (!graph) {
        return EXIT_FAILURE;
    }
    
    printf("Graph loaded: %d nodes, %d edges\n", graph->num_nodes, graph->num_edges);
    printf("Testing with source node: %d\n", source);
    printf("Number of threads: %d\n\n", num_threads);
    
    int *dist_seq = (int*)malloc(graph->num_nodes * sizeof(int));
    int *dist_par = (int*)malloc(graph->num_nodes * sizeof(int));
    
    printf("Running sequential Dijkstra...\n");
    double start_seq = omp_get_wtime();
    dijkstra_sequential(graph, source, dist_seq);
    double end_seq = omp_get_wtime();
    double time_seq = end_seq - start_seq;
    
    printf("Running parallel Dijkstra...\n");
    double start_par = omp_get_wtime();
    dijkstra_parallel(graph, source, dist_par, num_threads);
    double end_par = omp_get_wtime();
    double time_par = end_par - start_par;
    
    bool correct = verify_results(dist_seq, dist_par, graph->num_nodes);
    
    printf("\n=== Performance Results ===\n");
    printf("Sequential time:  %.6f seconds\n", time_seq);
    printf("Parallel time:    %.6f seconds\n", time_par);
    printf("Speedup:          %.4fx\n", time_seq / time_par);
    printf("Efficiency:       %.2f%%\n", (time_seq / time_par) / num_threads * 100);
    printf("Correctness:      %s\n\n", correct ? "PASSED" : "FAILED");
    
    printf("Sample distances (first 10 nodes):\n");
    for (int i = 0; i < (graph->num_nodes < 10 ? graph->num_nodes : 10); i++) {
        if (dist_seq[i] == INF) {
            printf("  Node %d: INF\n", i);
        } else {
            printf("  Node %d: %d\n", i, dist_seq[i]);
        }
    }
    
    free(dist_seq);
    free(dist_par);
    free_graph(graph);
    
    return correct ? EXIT_SUCCESS : EXIT_FAILURE;
}

