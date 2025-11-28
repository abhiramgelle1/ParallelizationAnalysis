#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>
#include <time.h>
#include <mpi.h>

#define INF INT_MAX
#define TAG_MIN_NODE 1
#define TAG_MIN_DIST 2
#define TAG_DISTANCES 3
#define TAG_CONTINUE 4

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

// Parallel Dijkstra's algorithm using MPI
void dijkstra_mpi(Graph *graph, int source, int *distances, int rank, int size) {
    bool *visited = (bool*)calloc(graph->num_nodes, sizeof(bool));
    
    for (int i = 0; i < graph->num_nodes; i++) {
        distances[i] = INF;
    }
    distances[source] = 0;
    
    MPI_Bcast(distances, graph->num_nodes, MPI_INT, 0, MPI_COMM_WORLD);
    
    for (int count = 0; count < graph->num_nodes; count++) {
        int local_min_dist = INF;
        int local_min_node = -1;
        
        int nodes_per_proc = graph->num_nodes / size;
        int start_node = rank * nodes_per_proc;
        int end_node = (rank == size - 1) ? graph->num_nodes : (rank + 1) * nodes_per_proc;
        
        for (int v = start_node; v < end_node; v++) {
            if (!visited[v] && distances[v] < local_min_dist) {
                local_min_dist = distances[v];
                local_min_node = v;
            }
        }
        
        int *all_min_dists = NULL;
        int *all_min_nodes = NULL;
        
        if (rank == 0) {
            all_min_dists = (int*)malloc(size * sizeof(int));
            all_min_nodes = (int*)malloc(size * sizeof(int));
        }
        
        MPI_Gather(&local_min_dist, 1, MPI_INT, all_min_dists, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Gather(&local_min_node, 1, MPI_INT, all_min_nodes, 1, MPI_INT, 0, MPI_COMM_WORLD);
        
        int min_dist = INF;
        int min_node = -1;
        
        if (rank == 0) {
            for (int i = 0; i < size; i++) {
                if (all_min_nodes[i] != -1 && all_min_dists[i] < min_dist) {
                    min_dist = all_min_dists[i];
                    min_node = all_min_nodes[i];
                }
            }
            free(all_min_dists);
            free(all_min_nodes);
        }
        
        MPI_Bcast(&min_dist, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&min_node, 1, MPI_INT, 0, MPI_COMM_WORLD);
        
        if (min_dist == INF || min_node == -1) {
            break;
        }
        
        visited[min_node] = true;
        
        int adj_size = graph->adj_size[min_node];
        int neighbors_per_proc = adj_size / size;
        int start_neighbor = rank * neighbors_per_proc;
        int end_neighbor = (rank == size - 1) ? adj_size : (rank + 1) * neighbors_per_proc;
        
        for (int i = start_neighbor; i < end_neighbor; i++) {
            int neighbor = graph->adj_list[min_node][i].dest;
            int weight = graph->adj_list[min_node][i].weight;
            
            if (!visited[neighbor] && distances[min_node] != INF) {
                int new_dist = distances[min_node] + weight;
                if (new_dist < distances[neighbor]) {
                    distances[neighbor] = new_dist;
                }
            }
        }
        
        MPI_Allreduce(MPI_IN_PLACE, distances, graph->num_nodes, MPI_INT, MPI_MIN, MPI_COMM_WORLD);
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
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    if (argc != 3) {
        if (rank == 0) {
            fprintf(stderr, "Usage: mpirun -np <num_processes> %s <input_file> <source_node>\n", argv[0]);
            fprintf(stderr, "Example: mpirun -np 4 %s weighted_graph.txt 0\n", argv[0]);
        }
        MPI_Finalize();
        return EXIT_FAILURE;
    }
    
    const char *filename = argv[1];
    int source = atoi(argv[2]);
    
    Graph *graph = NULL;
    if (rank == 0) {
        graph = read_graph_from_file(filename);
        if (!graph) {
            MPI_Finalize();
            return EXIT_FAILURE;
        }
        
        if (source < 0 || source >= graph->num_nodes) {
            fprintf(stderr, "Error: Source node must be between 0 and %d\n", graph->num_nodes - 1);
            free_graph(graph);
            MPI_Finalize();
            return EXIT_FAILURE;
        }
    }
    
    int num_nodes, num_edges;
    if (rank == 0) {
        num_nodes = graph->num_nodes;
        num_edges = graph->num_edges;
    }
    MPI_Bcast(&num_nodes, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&num_edges, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    if (rank != 0) {
        graph = create_graph(num_nodes, num_edges);
    }

    if (rank != 0) {
        FILE *file = fopen(filename, "r");
        if (file) {
            int dummy_nodes, dummy_edges;
            fscanf(file, "%d %d", &dummy_nodes, &dummy_edges);
            for (int i = 0; i < num_edges; i++) {
                int u, v, weight;
                fscanf(file, "%d %d %d", &u, &v, &weight);
                add_edge(graph, u, v, weight);
            }
            fclose(file);
        }
    }
    
    int *distances = (int*)malloc(num_nodes * sizeof(int));
    
    double start = MPI_Wtime();
    dijkstra_mpi(graph, source, distances, rank, size);
    double end = MPI_Wtime();
    
    double execution_time = end - start;
    double max_time;
    MPI_Reduce(&execution_time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    
    if (rank == 0) {
        printf("Shortest distances from node %d:\n", source);
        for (int i = 0; i < graph->num_nodes; i++) {
            if (distances[i] == INF) {
                printf("Node %d: INF\n", i);
            } else {
                printf("Node %d: %d\n", i, distances[i]);
            }
        }
        printf("\nMPI execution time (%d processes): %.6f seconds\n", size, max_time);
    }
    
    free(distances);
    free_graph(graph);
    
    MPI_Finalize();
    return EXIT_SUCCESS;
}

