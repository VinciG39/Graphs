#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    int to;
    int weight;
}Edge;

typedef struct
{
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

int initVertex(Vertex *v, int value, int degree, const int *neighbors)
{
    v->value  = value;
    v->degree = degree;
    v->edges  = NULL;

    if (degree > 0) {
        v->edges = malloc(degree * sizeof *v->edges);
        if (v->edges == NULL)
            return -1;
    }

    for (int i = 0; i < degree; i++)
        v->edges[i] = createEdge(neighbors[i], 1);

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

int main () 
{
    int count = 5;
    Graph *G = createGraph(count);
    if (G == NULL)
    {
        return 1;
    }

    // Vertecis
    int V0[] = {1, 2, 3, 4}; // A: B C D E
    int V1[] = {0, 2, 4};    // B: A C E
    int V2[] = {1, 0, 3};    // C: B A D
    int V3[] = {2, 0};       // D: C A
    int V4[] = {1, 0};       // E: B A

    if (initVertex(&G->vertices[0], 3, 4, V0) != 0 ||
        initVertex(&G->vertices[1], 1, 3, V1) != 0 ||
        initVertex(&G->vertices[2], 0, 3, V2) != 0 ||
        initVertex(&G->vertices[3], 4, 2, V3) != 0 ||
        initVertex(&G->vertices[4], 2, 2, V4) != 0)
    {
        freeGraph(G);
        return -1;
    }

    exportDot(G, "graf.dot");
    freeGraph(G);

    system("dot -Tpng graf.dot -o graf.png");
    return 0;
}