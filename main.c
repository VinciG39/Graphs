#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <stdbool.h>

typedef struct
{
    int to;
    int weight;
}Edge;

typedef struct
{
    char name[2];
    int capacity;
    int value;
    int degree;
    Edge *edges;
}Vertex;

typedef struct
{
    int count;
    Vertex *vertices;
}Graph;

Edge createEdge (int to, int weight)
{   
    Edge e = {.to = to, .weight = weight};
    return e;
}

int initVertex(Vertex *v, int value, int degree, const int *neighbours, const int *edgesV)
{
    v->value  = value;
    v->degree = degree;
    v->edges  = NULL;

    if (degree > 0)
    {
        v->edges = malloc(degree * sizeof *v->edges);
        if (v->edges == NULL) return -1;
    }

    for (int i = 0; i < degree; i++)
    {
        v->edges[i] = createEdge(neighbours[i], edgesV[i]);
    }

    return 0;
}

Graph *createGraph (int vertexCount)
{
    Graph *g = malloc(sizeof(Graph));
    if (g == NULL) return NULL;

    g->count = vertexCount;
    g->vertices = calloc(vertexCount, sizeof(*g->vertices));
    if (g->vertices == NULL)
    {
        free(g);
        return NULL;
    }

    return g;
}

void freeGraph (Graph *g)
{
    if (g == NULL)
    {
        return;
    }

    for (int i = 0 ; i < g->count; i++)
    {
        free(g->vertices[i].edges);
    }
    free(g->vertices);
    free(g);
}

int exportDot(const Graph *g, const char *path)
{
    FILE *f = fopen(path, "w");
    if (f == NULL)
        return -1;

    fprintf(f, "strict graph G {\n");
    fprintf(f, "    node [shape=circle];\n");

    for (int i = 0; i < g->count; i++)
    {
        fprintf(f, "    %d [label=\"%c\\n(%d)\"];\n", i, 'A' + i, g->vertices[i].value);
    }
    for (int i = 0; i < g->count; i++) {
        const Vertex *v = &g->vertices[i];
        for (int k = 0; k < v->degree; k++)
        {
            fprintf(f, "    %d -- %d [label=\"%d\"];\n", i, v->edges[k].to, v->edges[k].weight);
        }
    }

    fprintf(f, "}\n");
    fclose(f);
    return 0;
}

int loadEdges (FILE *file, int **to, int **values)
{
    int capacity = 2;
    int iterator = 0;

    int *tempTo = malloc(capacity * sizeof(int));
    int *tempValues = malloc(capacity * sizeof(int));

    int c;
    while ((c = fgetc(file)) != '\n' || c != EOF)
    {
        if (isspace(c))
        {
            continue;
        }

        char tag = (char)c;
        int value = 0;

        while((c = fgetc(file)) == ' ');

        while (isdigit(c))
        {
            value = value * 10 + (c - '0');
            c = fgetc(file);
        }

        if (iterator >= capacity)
        {   
            capacity *= 2;
            tempTo = realloc(tempTo, capacity * sizeof(int));
            tempValues = realloc(tempValues, capacity * sizeof(int));
        }

        tempTo[iterator] = tag - 'A';
        tempValues[iterator] = value;
        iterator++;
        
        if (c == '\n' || c == EOF)
        {
            break;
        }

        ungetc(c, file);
    }

    *to = tempTo;
    *values = tempValues;

    return iterator;
}

void loadVertecis (FILE *file, char *vertecis, int *vertecisValues)
{
    int c;
    int iterator = 0;

    while ((c = fgetc(file)) != '\n' || c != EOF)
    {
        if (isspace(c))
        {
            continue;
        }

        char tag = (char)c;
        int value = 0;

        while((c = fgetc(file)) == ' ');

        while (isdigit(c))
        {
            value = value * 10 + (c - '0');
            c = fgetc(file);
        }

        //printf("%c %d\n", tag, value);
        vertecis[iterator] = tag;
        vertecisValues[iterator] = value;
        iterator++;
        
        if (c == '\n' || c == EOF)
        {
            break;
        }

        ungetc(c, file);
    }
}

void free2Dint (int **array, int size)
{   

    if (array == NULL)
    {
        return;
    }
    for (int i = 0; i < size; i++)
    {
        free(array[i]);
    }
    free(array);
}

int nearestNeighbour(Graph *g)
{
    bool *visited = calloc(g->count, sizeof(bool));

    int start = 0;
    int current = start;
    visited[current] = true;

    int cost = 0;

    for (int i = 1; i < g->count; i++)
    {
        Vertex *cur = &g->vertices[current];
        int lowest = INT_MAX;
        int best = -1;

        for (int j = 0; j < cur->degree; j++)
        {
            int to = cur->edges[j].to;
            int weight = cur->edges[j].weight;

            if (!visited[to] && weight < lowest)
            {
                lowest = weight;
                best = to;
            }
        }

        if (best == -1)
        {
            free(visited);
            fprintf(stderr, "Seknuty v slepej ulicke\n");
            return -1;
        }

        visited[best] = true;
        cost += lowest;
        current = best;
    }

    // Vertex *cur = &g->vertices[current];
    // for (int i = 0; i < cur->degree; i++)
    // {
    //     int to = cur->edges[i].to;
    //     int weight = cur->edges[i].weight;

    //     if (to == start)
    //     {
    //         cost += weight;
    //     }
    // }


    free(visited);
    return cost;
}

int main (int argc, char *argv[]) 
{   
    int count = 0;

    if (argc < 2)
    {
        fprintf(stderr, "chyba cesta k suboru!\n");
    }

    FILE *file = fopen(argv[1], "r");
    
    if (file == NULL)
    {
         fprintf(stderr, "chyba v ceste k suboru!\n");
    }

    fscanf(file, "%d", &count);
    Graph *G = createGraph(count);
    
    if (G == NULL)
    {
        return 1;
    }

    char *vertecis = malloc(count * sizeof(char));
    int *vertecisV = malloc(count * sizeof(int));
    
    int **edgesV = malloc(count * sizeof(int *));
    int **neighbours = malloc(count * sizeof(int *));
    int *degree = malloc(count * sizeof(int)); 

    loadVertecis(file, vertecis, vertecisV);

    for (int i = 0; i < count; i++)
    {
        degree[i] = loadEdges(file, &neighbours[i], &edgesV[i]);
    }

    fclose(file);

    for (int i = 0; i < count; i++)
    {
        if (initVertex(&G->vertices[i], vertecisV[i], degree[i], neighbours[i], edgesV[i]) != 0)
        {
            freeGraph(G);
            return -1;
        }
    }

    exportDot(G, "output/NN.dot");

    int costOfGraph = nearestNeighbour(G);
    printf("NN path costs: %d", costOfGraph);

    //free
    free(vertecis);
    free(vertecisV);
    free(degree);

    free2Dint(edgesV, count);
    free2Dint(neighbours, count);
    freeGraph(G);

    system("dot -Tpng output/NN.dot -o output/NN.png");
    return 0;
}