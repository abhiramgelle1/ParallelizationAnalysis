#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>
#include <time.h>

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
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file> <source_node>\n", argv[0]);
        fprintf(stderr, "Example: %s weighted_graph.txt 0\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    const char *filename = argv[1];
    int source = atoi(argv[2]);
    
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
    
    clock_t start = clock();
    dijkstra_sequential(graph, source, distances);
    clock_t end = clock();
    
    double cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("Shortest distances from node %d:\n", source);
    for (int i = 0; i < graph->num_nodes; i++) {
        if (distances[i] == INF) {
            printf("Node %d: INF\n", i);
        } else {
            printf("Node %d: %d\n", i, distances[i]);
        }
    }
    
    printf("\nSequential execution time: %.6f seconds\n", cpu_time_used);
    
    // Cleanup
    free(distances);
    free_graph(graph);
    
    return EXIT_SUCCESS;
}

