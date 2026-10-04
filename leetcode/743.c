#include <stdlib.h>
#include <limits.h>

int networkDelayTime(int** times, int timesSize, int* timesColSize, int n, int k) {
    int** graph = malloc((n + 1) * sizeof(int*));
    int* graphSize = calloc(n + 1, sizeof(int));

    for (int i = 0; i <= n; i++)
        graph[i] = malloc(2 * (timesSize + 1) * sizeof(int));

    for (int i = 0; i < timesSize; i++) {
        int u = times[i][0];
        int v = times[i][1];
        int w = times[i][2];

        graph[u][graphSize[u] * 2] = v;
        graph[u][graphSize[u] * 2 + 1] = w;
        graphSize[u]++;
    }

    int* dist = malloc((n + 1) * sizeof(int));
    int* visited = calloc(n + 1, sizeof(int));

    for (int i = 1; i <= n; i++)
        dist[i] = INT_MAX;

    dist[k] = 0;

    for (int count = 1; count <= n; count++) {
        int u = -1;
        int minDist = INT_MAX;

        for (int i = 1; i <= n; i++) {
            if (!visited[i] && dist[i] < minDist) {
                minDist = dist[i];
                u = i;
            }
        }

        if (u == -1)
            break;

        visited[u] = 1;

        for (int i = 0; i < graphSize[u]; i++) {
            int v = graph[u][i * 2];
            int w = graph[u][i * 2 + 1];

            if (dist[u] != INT_MAX &&
                dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
            }
        }
    }

    int ans = 0;

    for (int i = 1; i <= n; i++) {
        if (dist[i] == INT_MAX)
            return -1;

        if (dist[i] > ans)
            ans = dist[i];
    }

    return ans;
}