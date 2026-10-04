#include <stdio.h>

#define INF 9999
#define MAX 20

void bellmanford(int graph[MAX][MAX], int n, int start)
{
    int distance[MAX];
    int nextnode[MAX];

    int i, j, k;
    int changed;
    int chk;

    for(i = 0; i < n; i++)
    {
        distance[i] = INF;
        nextnode[i] = -1;
    }

    distance[start] = 0;
    nextnode[start] = start;

    for(i = 0; i < n - 1; i++)
    {
        changed = 0;

        for(j = 0; j < n; j++)
        {
            for(k = 0; k < n; k++)
            {
                if(graph[j][k] != 0 &&
                   distance[j] != INF)
                {
                    chk = distance[j] + graph[j][k];

                    if(distance[k] > chk)
                    {
                        distance[k] = chk;

                        if(j == start)
                            nextnode[k] = k;
                        else
                            nextnode[k] = nextnode[j];

                        changed = 1;
                    }
                }
            }
        }

        if(!changed)
            break;
    }

    for(j = 0; j < n; j++)
    {
        for(k = 0; k < n; k++)
        {
            if(graph[j][k] != 0 &&
               distance[j] != INF)
            {
                chk = distance[j] + graph[j][k];

                if(distance[k] > chk)
                {
                    printf("\nNegative weight cycle exists.\n");
                    return;
                }
            }
        }
    }

    printf("\nBellman-Ford Algorithm\n");
    printf("----------------------------------\n");
    printf("Vertex\tNextNode\tDistance\n");
    printf("----------------------------------\n");

    for(i = 0; i < n; i++)
    {
        if(distance[i] == INF)
            printf("%d\t%d\t\tNo path\n",
                   i, nextnode[i]);
        else
            printf("%d\t%d\t\t%d\n",
                   i, nextnode[i], distance[i]);
    }
}

int main()
{
    int graph[MAX][MAX] = {0};
    int n;
    int i, j;
    int weight;
    int start;

    printf("Enter the number of vertices: ");
    scanf("%d", &n);

    printf("\nEnter the cost for each edge\n");

    for(i = 0; i < n; i++)
    {
        for(j = i + 1; j < n; j++)
        {
            printf("cost between vertex %d and %d: ",
                   i, j);

            scanf("%d", &weight);

            graph[i][j] = weight;
            graph[j][i] = weight;
        }
    }

    printf("\nEnter the starting vertex: ");
    scanf("%d", &start);

    if(start < 0 || start >= n)
    {
        printf("Invalid starting vertex.\n");
        return 0;
    }

    bellmanford(graph, n, start);

    return 0;
}
