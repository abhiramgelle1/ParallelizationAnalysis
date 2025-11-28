#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    if (u < 0 || u >= graph->num_nodes || v < 0 || v >= graph->num_nodes) {
        fprintf(stderr, "Error: Invalid edge (%d, %d) - nodes must be between 0 and %d\n", 
                u, v, graph->num_nodes - 1);
        return;
    }
    
    int u_size = graph->adj_size[u];
    int v_size = graph->adj_size[v];
    
    graph->adj_list[u] = (Edge*)realloc(graph->adj_list[u], (u_size + 1) * sizeof(Edge));
    if (!graph->adj_list[u]) {
        perror("Error reallocating memory");
        return;
    }
    graph->adj_list[u][u_size].dest = v;
    graph->adj_list[u][u_size].weight = weight;
    graph->adj_size[u]++;
    
    graph->adj_list[v] = (Edge*)realloc(graph->adj_list[v], (v_size + 1) * sizeof(Edge));
    if (!graph->adj_list[v]) {
        perror("Error reallocating memory");
        return;
    }
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

// Parallel Dijkstra's algorithm using OpenMP
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
        
        #pragma omp parallel for num_threads(num_threads) reduction(min:min_dist)
        for (int v = 0; v < graph->num_nodes; v++) {
            if (!visited[v] && distances[v] < min_dist) {
                min_dist = distances[v];
            }
        }
        
        if (min_dist == INF) {
            break;
        }
        
        for (int v = 0; v < graph->num_nodes; v++) {
            if (!visited[v] && distances[v] == min_dist) {
                min_node = v;
                break;
            }
        }
        
        if (min_node == -1) {
            break;
        }
        
        visited[min_node] = true;
        
        int adj_size = graph->adj_size[min_node];
        #pragma omp parallel for num_threads(num_threads)
        for (int i = 0; i < adj_size; i++) {
            int neighbor = graph->adj_list[min_node][i].dest;
            int weight = graph->adj_list[min_node][i].weight;
            
            if (!visited[neighbor] && distances[min_node] != INF) {
                int new_dist = distances[min_node] + weight;
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

void dijkstra_parallel_optimized(Graph *graph, int source, int *distances, int num_threads) {
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
        
        if (adj_size > 0 && graph->adj_list[min_node] != NULL) {
            #pragma omp parallel for num_threads(num_threads)
            for (int i = 0; i < adj_size; i++) {
                int neighbor = graph->adj_list[min_node][i].dest;
                int weight = graph->adj_list[min_node][i].weight;
                
                if (neighbor >= 0 && neighbor < graph->num_nodes && !visited[neighbor] && base_dist != INF) {
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
    }
    
    free(visited);
}

Graph* read_graph_from_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
        perror("fopen");
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
    int result = fscanf(file, "%d %d", &num_nodes, &num_edges);
    if (result != 2) {
        rewind(file);
        char first_line[256];
        if (fgets(first_line, sizeof(first_line), file)) {
            fprintf(stderr, "Error: Failed to parse graph header from '%s'\n", filename);
            fprintf(stderr, "First line (as read): '");
            for (size_t i = 0; i < strlen(first_line) && i < 50; i++) {
                unsigned char c = (unsigned char)first_line[i];
                if (c >= 32 && c < 127) {
                    fprintf(stderr, "%c", c);
                } else if (c == '\n') {
                    fprintf(stderr, "\\n");
                } else if (c == '\r') {
                    fprintf(stderr, "\\r");
                } else if (c == '\t') {
                    fprintf(stderr, "\\t");
                } else {
                    fprintf(stderr, "\\x%02x", c);
                }
            }
            fprintf(stderr, "'\n");
            fprintf(stderr, "Expected format: '<num_nodes> <num_edges>' (two integers)\n");
            fprintf(stderr, "fscanf returned %d (expected 2)\n", result);
        } else {
            fprintf(stderr, "Error: Failed to read first line from '%s'\n", filename);
        }
        fclose(file);
        return NULL;
    }
    
    if (num_nodes <= 0 || num_edges < 0) {
        fprintf(stderr, "Error: Invalid graph size: %d nodes, %d edges\n", num_nodes, num_edges);
        fclose(file);
        return NULL;
    }
    
    Graph *graph = create_graph(num_nodes, num_edges);
    if (!graph) {
        fclose(file);
        return NULL;
    }
    
    for (int i = 0; i < num_edges; i++) {
        int u, v, weight;
        if (fscanf(file, "%d %d %d", &u, &v, &weight) != 3) {
            fprintf(stderr, "Error: Failed to read edge %d\n", i);
            free_graph(graph);
            fclose(file);
            return NULL;
        }
        add_edge(graph, u, v, weight);
    }
    
    fclose(file);
    return graph;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <input_file> <source_node> <num_threads>\n", argv[0]);
        fprintf(stderr, "Example: %s weighted_graph.txt 0 4\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    const char *filename = argv[1];
    int source = atoi(argv[2]);
    int num_threads = atoi(argv[3]);
    
    if (num_threads <= 0) {
        fprintf(stderr, "Error: Number of threads must be positive\n");
        return EXIT_FAILURE;
    }
    
    omp_set_num_threads(num_threads);
    
    Graph *graph = read_graph_from_file(filename);
    if (!graph) {
        return EXIT_FAILURE;
    }
    
    if (source < 0 || source >= graph->num_nodes) {
        fprintf(stderr, "Error: Source node must be between 0 and %d\n", graph->num_nodes - 1);
        free_graph(graph);
        return EXIT_FAILURE;
    }
    
    int *distances = (int*)malloc(graph->num_nodes * sizeof(int));
    
    double start = omp_get_wtime();
    dijkstra_parallel_optimized(graph, source, distances, num_threads);
    double end = omp_get_wtime();
    
    double execution_time = end - start;
    
    printf("Shortest distances from node %d:\n", source);
    for (int i = 0; i < graph->num_nodes; i++) {
        if (distances[i] == INF) {
            printf("Node %d: INF\n", i);
        } else {
            printf("Node %d: %d\n", i, distances[i]);
        }
    }
    
    printf("\nParallel execution time (%d threads): %.6f seconds\n", num_threads, execution_time);
    
    free(distances);
    free_graph(graph);
    
    return EXIT_SUCCESS;
}

