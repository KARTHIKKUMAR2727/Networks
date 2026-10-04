#include <stdlib.h>
#include <limits.h>

int minimumEffortPath(int** heights, int heightsSize, int* heightsColSize) {
    int m = heightsSize;
    int n = heightsColSize[0];
    int total = m * n;

    int* dist = malloc(total * sizeof(int));
    int* visited = calloc(total, sizeof(int));

    for (int i = 0; i < total; i++)
        dist[i] = INT_MAX;

    dist[0] = 0;

    int dr[] = {-1, 1, 0, 0};
    int dc[] = {0, 0, -1, 1};

    for (int count = 0; count < total; count++) {
        int u = -1;
        int minEffort = INT_MAX;

        for (int i = 0; i < total; i++) {
            if (!visited[i] && dist[i] < minEffort) {
                minEffort = dist[i];
                u = i;
            }
        }

        if (u == -1)
            break;

        if (u == total - 1)
            return dist[u];

        visited[u] = 1;

        int r = u / n;
        int c = u % n;

        for (int d = 0; d < 4; d++) {
            int nr = r + dr[d];
            int nc = c + dc[d];

            if (nr < 0 || nr >= m || nc < 0 || nc >= n)
                continue;

            int v = nr * n + nc;

            int effort = abs(heights[r][c] - heights[nr][nc]);
            int newEffort = dist[u] > effort ? dist[u] : effort;

            if (newEffort < dist[v])
                dist[v] = newEffort;
        }
    }

    return dist[total - 1];
}