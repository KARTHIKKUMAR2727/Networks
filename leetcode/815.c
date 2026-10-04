#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int numBusesToDestination(int** routes, int routesSize, int* routesColSize,
                          int source, int target) {
    if (source == target)
        return 0;

    int maxStop = 1000000;

    int** stopToRoutes = calloc(maxStop + 1, sizeof(int*));
    int* routeCount = calloc(maxStop + 1, sizeof(int));
    int* routeCapacity = calloc(maxStop + 1, sizeof(int));

    for (int i = 0; i < routesSize; i++) {
        for (int j = 0; j < routesColSize[i]; j++) {
            int stop = routes[i][j];

            if (routeCount[stop] == routeCapacity[stop]) {
                routeCapacity[stop] =
                    routeCapacity[stop] == 0 ? 2 : routeCapacity[stop] * 2;

                stopToRoutes[stop] = realloc(
                    stopToRoutes[stop],
                    routeCapacity[stop] * sizeof(int)
                );
            }

            stopToRoutes[stop][routeCount[stop]++] = i;
        }
    }

    int* queue = malloc(routesSize * sizeof(int));
    bool* visitedRoute = calloc(routesSize, sizeof(bool));
    bool* visitedStop = calloc(maxStop + 1, sizeof(bool));

    int front = 0, rear = 0;

    for (int i = 0; i < routeCount[source]; i++) {
        int route = stopToRoutes[source][i];

        if (!visitedRoute[route]) {
            visitedRoute[route] = true;
            queue[rear++] = route;
        }
    }

    int buses = 1;

    while (front < rear) {
        int size = rear - front;

        while (size--) {
            int route = queue[front++];

            for (int j = 0; j < routesColSize[route]; j++) {
                int stop = routes[route][j];

                if (stop == target)
                    return buses;

                if (visitedStop[stop])
                    continue;

                visitedStop[stop] = true;

                for (int k = 0; k < routeCount[stop]; k++) {
                    int nextRoute = stopToRoutes[stop][k];

                    if (!visitedRoute[nextRoute]) {
                        visitedRoute[nextRoute] = true;
                        queue[rear++] = nextRoute;
                    }
                }
            }
        }

        buses++;
    }

    return -1;
}