#include "drone_structures.h"
#include <stdio.h>

static Zone zones[MAX_ZONES];
static Route routes[MAX_ZONES * MAX_ZONES];
static int zone_count = 0, route_count = 0;

int add_zone(int id, const char *name)
{
    if (zone_count >= MAX_ZONES)
        return 0;

    zones[zone_count].id = id;
    snprintf(zones[zone_count].name, 50, "%s", name);
    zones[zone_count].is_accessible = 1;
    zone_count++;
    return 1;
}

int add_route(int src, int dest, float distance)
{
    if (route_count >= MAX_ZONES * MAX_ZONES ||
        distance < 0)
        return 0;

    routes[route_count++] =
        (Route){src, dest, distance, 1};
    return 1;
}

void block_route(int src, int dest)
{
    for (int i = 0; i < route_count; i++)
        if (routes[i].source == src &&
            routes[i].destination == dest)
            routes[i].is_available = 0;
}

void display_routes(void)
{
    for (int i = 0; i < route_count; i++)
        if (routes[i].is_available)
            printf("%d -> %d : %.2f\n",
                   routes[i].source,
                   routes[i].destination,
                   routes[i].distance);
}
