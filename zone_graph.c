#include "drone_structures.h"

#include <stdio.h>
#include <string.h>

void drone_graph_init_demo(DroneZoneGraph *graph) {
    size_t i, j;
    static const char *names[] = {
        "Clock Tower (Ghanta Ghar)", "Doon Hospital", "Gandhi Park",
        "Parade Ground", "Dehradun Railway Station", "ISBT Dehradun",
        "Patel Nagar", "Nehru Colony", "Vasant Vihar", "Ballupur Chowk",
        "Garhi Cantt", "FRI Main Gate", "Prem Nagar", "Clement Town",
        "Rajpur Road", "Jakhan", "Sahastradhara Road", "Raipur",
        "Dalanwala", "Rispana Bridge", "Kargi Chowk", "Jogiwala",
        "Bindal Bridge", "Chakrata Road", "Kaulagarh", "Subhash Nagar",
        "Ladpur", "Maldevta"
    };
    const size_t name_count = sizeof(names) / sizeof(names[0]);
    if (graph == NULL) return;
    graph->zone_count = name_count;
    for (i = 0; i < DRONE_MAX_ZONES; ++i) {
        for (j = 0; j < DRONE_MAX_ZONES; ++j)
            graph->weights[i][j] = (i == j) ? 0 : -1;
        graph->names[i][0] = '\0';
    }
    for (i = 0; i < graph->zone_count; ++i)
        snprintf(graph->names[i], DRONE_ZONE_NAME, "%s", names[i]);

    /*
     * These links and weights are a connected demonstration graph over
     * recognizable Dehradun places. They are not verified streets or road
     * distances and must not be used for real navigation.
     */
    (void)drone_graph_add_road(graph, 0, 1, 2);
    (void)drone_graph_add_road(graph, 0, 2, 1);
    (void)drone_graph_add_road(graph, 0, 3, 2);
    (void)drone_graph_add_road(graph, 0, 4, 3);
    (void)drone_graph_add_road(graph, 0, 18, 3);
    (void)drone_graph_add_road(graph, 4, 5, 5);
    (void)drone_graph_add_road(graph, 5, 6, 2);
    (void)drone_graph_add_road(graph, 5, 7, 4);
    (void)drone_graph_add_road(graph, 5, 13, 4);
    (void)drone_graph_add_road(graph, 5, 20, 3);
    (void)drone_graph_add_road(graph, 5, 25, 4);
    (void)drone_graph_add_road(graph, 6, 8, 3);
    (void)drone_graph_add_road(graph, 6, 9, 4);
    (void)drone_graph_add_road(graph, 9, 10, 4);
    (void)drone_graph_add_road(graph, 9, 11, 3);
    (void)drone_graph_add_road(graph, 11, 12, 4);
    (void)drone_graph_add_road(graph, 12, 24, 5);
    (void)drone_graph_add_road(graph, 18, 14, 4);
    (void)drone_graph_add_road(graph, 14, 15, 3);
    (void)drone_graph_add_road(graph, 14, 16, 5);
    (void)drone_graph_add_road(graph, 16, 17, 4);
    (void)drone_graph_add_road(graph, 17, 26, 4);
    (void)drone_graph_add_road(graph, 17, 27, 7);
    (void)drone_graph_add_road(graph, 18, 19, 5);
    (void)drone_graph_add_road(graph, 19, 17, 4);
    (void)drone_graph_add_road(graph, 19, 21, 3);
    (void)drone_graph_add_road(graph, 20, 19, 3);
    (void)drone_graph_add_road(graph, 22, 0, 3);
    (void)drone_graph_add_road(graph, 23, 11, 5);
}

int drone_graph_add_zone(DroneZoneGraph *graph, const char *name) {
    size_t i;
    size_t name_length;
    size_t new_index;
    if (graph == NULL || name == NULL || graph->zone_count >= DRONE_MAX_ZONES) return -1;
    name_length = strlen(name);
    if (name_length == 0 || name_length >= DRONE_ZONE_NAME) return -1;
    for (i = 0; i < graph->zone_count; ++i)
        if (strcmp(graph->names[i], name) == 0) return -1;
    new_index = graph->zone_count;
    for (i = 0; i < DRONE_MAX_ZONES; ++i) {
        graph->weights[new_index][i] = (new_index == i) ? 0 : -1;
        graph->weights[i][new_index] = (new_index == i) ? 0 : -1;
    }
    memcpy(graph->names[new_index], name, name_length + 1);
    ++graph->zone_count;
    return (int)new_index;
}

int drone_graph_add_road(DroneZoneGraph *graph, int from, int to, int distance) {
    if (graph == NULL || from < 0 || to < 0 || (size_t)from >= graph->zone_count ||
        (size_t)to >= graph->zone_count || from == to || distance <= 0)
        return 0;
    graph->weights[from][to] = distance;
    graph->weights[to][from] = distance;
    return 1;
}

int drone_graph_shortest_path(const DroneZoneGraph *graph, int start, int goal,
                              int *out_path, size_t *in_out_length,
                              int *out_distance) {
    int distances[DRONE_MAX_ZONES];
    int previous[DRONE_MAX_ZONES];
    unsigned char visited[DRONE_MAX_ZONES] = {0};
    int reverse_path[DRONE_MAX_ZONES];
    size_t i, path_length = 0, capacity;
    if (graph == NULL || in_out_length == NULL || out_distance == NULL || start < 0 || goal < 0 ||
        (size_t)start >= graph->zone_count || (size_t)goal >= graph->zone_count)
        return 0;
    capacity = *in_out_length;
    for (i = 0; i < graph->zone_count; ++i) {
        distances[i] = 2147483647;
        previous[i] = -1;
    }
    distances[start] = 0;
    for (i = 0; i < graph->zone_count; ++i) {
        size_t node, neighbor;
        int current = -1;
        int best = 2147483647;
        for (node = 0; node < graph->zone_count; ++node)
            if (!visited[node] && distances[node] < best) {
                best = distances[node];
                current = (int)node;
            }
        if (current < 0) break;
        visited[current] = 1;
        if (current == goal) break;
        for (neighbor = 0; neighbor < graph->zone_count; ++neighbor) {
            int edge = graph->weights[current][neighbor];
            if (edge > 0 && !visited[neighbor] && distances[current] <= 2147483647 - edge &&
                distances[current] + edge < distances[neighbor]) {
                distances[neighbor] = distances[current] + edge;
                previous[neighbor] = current;
            }
        }
    }
    if (distances[goal] == 2147483647) return 0;
    {
        int cursor = goal;
        while (cursor >= 0 && path_length < DRONE_MAX_ZONES) {
            reverse_path[path_length++] = cursor;
            if (cursor == start) break;
            cursor = previous[cursor];
        }
        if (path_length == 0 || reverse_path[path_length - 1] != start) return 0;
    }
    if (path_length == 0 || path_length > capacity || out_path == NULL) {
        *in_out_length = path_length;
        return 0;
    }
    for (i = 0; i < path_length; ++i) out_path[i] = reverse_path[path_length - 1 - i];
    *in_out_length = path_length;
    *out_distance = distances[goal];
    return 1;
}
