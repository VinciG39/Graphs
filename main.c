#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

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

int initVertex(Vertex *v, int value, int degree, const int *neighbors, const int *edgesV)
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
        v->edges[i] = createEdge(neighbors[i], edgesV[i]);
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

int nearestNeighbor (Graph g)
{  
    Vertex *visited = malloc(g.count * sizeof(Vertex));
    Vertex start = g.vertices[0];
    Vertex point = start;
    Vertex end;

    int result = 0;
    int lowest = INT_MAX;

    for (int i = 0; i < g.count; i++)
    {
        for (int j = 0; j < g.vertices[i].degree; j++)
        {
            if (point.edges->weight < lowest)
            {
                lowest = point.edges->weight;
            }
        }
        result += lowest;
    }
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
    int **neighbors = malloc(count * sizeof(int *));
    int *degree = malloc(count * sizeof(int)); 

    loadVertecis(file, vertecis, vertecisV);

    for (int i = 0; i < count; i++)
    {
        degree[i] = loadEdges(file, &neighbors[i], &edgesV[i]);
    }

    fclose(file);

    for (int i = 0; i < count; i++)
    {
        if (initVertex(&G->vertices[i], vertecisV[i], degree[i], neighbors[i], edgesV[i]) != 0)
        {
            freeGraph(G);
            return -1;
        }
    }

    exportDot(G, "output/NN.dot");

    //free
    free(vertecis);
    free(vertecisV);
    free(degree);

    free2Dint(edgesV, count);
    free2Dint(neighbors, count);
    freeGraph(G);

    system("dot -Tpng output/NN.dot -o output/NN.png");
    return 0;
}