#include <stdio.h>

#define INF 9999
#define MAX 20

void dijkstra(int graph[MAX][MAX], int n, int start)
{
    int distance[MAX];
    int visited[MAX];
    int nextnode[MAX];

    int i, count, min, next;

    for(i = 0; i < n; i++)
    {
        distance[i] = INF;
        visited[i] = 0;
        nextnode[i] = -1;
    }

    distance[start] = 0;
    nextnode[start] = start;

    for(count = 0; count < n - 1; count++)
    {
        min = INF;
        next = -1;

        for(i = 0; i < n; i++)
        {
            if(!visited[i] && distance[i] < min)
            {
                min = distance[i];
                next = i;
            }
        }

        if(next == -1)
            break;

        visited[next] = 1;

        for(i = 0; i < n; i++)
        {
            if(!visited[i] &&
               graph[next][i] != 0 &&
               distance[next] != INF &&
               distance[next] + graph[next][i] < distance[i])
            {
                distance[i] = distance[next] + graph[next][i];

                if(next == start)
                    nextnode[i] = i;
                else
                    nextnode[i] = nextnode[next];
            }
        }
    }

    printf("\nDijkstra's Algorithm\n");
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

            if(weight < 0)
            {
                printf("Negative weight is not allowed in Dijkstra.\n");
                return 0;
            }

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

    dijkstra(graph, n, start);

    return 0;
}
