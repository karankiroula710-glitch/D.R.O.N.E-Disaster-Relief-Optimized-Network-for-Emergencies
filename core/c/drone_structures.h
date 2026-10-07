#ifndef DRONE_STRUCTURES_H
#define DRONE_STRUCTURES_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DRONE_MAX_VERTICES 16
#define DRONE_PRIORITY_CAPACITY 64
#define DRONE_PENDING_CAPACITY 64
#define DRONE_INDEX_CAPACITY 128
#define DRONE_HISTORY_TEXT 192

typedef struct DronePriorityItem {
    int request_id;
    int urgency;
    unsigned long long received_order;
} DronePriorityItem;

typedef struct DronePriorityQueue {
    DronePriorityItem items[DRONE_PRIORITY_CAPACITY];
    size_t size;
} DronePriorityQueue;

typedef struct DroneGraph {
    int vertex_count;
    double weight[DRONE_MAX_VERTICES][DRONE_MAX_VERTICES];
} DroneGraph;

typedef struct DroneRequestQueue {
    int request_ids[DRONE_PENDING_CAPACITY];
    size_t head;
    size_t size;
} DroneRequestQueue;

typedef struct DroneRequestIndex {
    int keys[DRONE_INDEX_CAPACITY];
    size_t values[DRONE_INDEX_CAPACITY];
    unsigned char occupied[DRONE_INDEX_CAPACITY];
} DroneRequestIndex;

typedef struct DroneHistoryNode {
    char message[DRONE_HISTORY_TEXT];
    struct DroneHistoryNode *next;
} DroneHistoryNode;

void drone_priority_queue_init(DronePriorityQueue *queue);
int drone_priority_queue_push(DronePriorityQueue *queue, DronePriorityItem item);
int drone_priority_queue_pop(DronePriorityQueue *queue, DronePriorityItem *item);
int drone_priority_queue_is_empty(const DronePriorityQueue *queue);

void drone_graph_init(DroneGraph *graph, int vertex_count);
int drone_graph_add_edge(DroneGraph *graph, int from, int to, double distance);
/* Returns path vertex count, 0 if unreachable, or -1 for invalid/capacity errors. */
int drone_graph_shortest_path(const DroneGraph *graph, int start, int target,
                              int *path, size_t path_capacity, double *distance);

void drone_request_queue_init(DroneRequestQueue *queue);
int drone_request_queue_push(DroneRequestQueue *queue, int request_id);
int drone_request_queue_pop(DroneRequestQueue *queue, int *request_id);
int drone_request_queue_is_empty(const DroneRequestQueue *queue);

void drone_request_index_init(DroneRequestIndex *index);
int drone_request_index_insert(DroneRequestIndex *index, int request_id, size_t value);
int drone_request_index_find(const DroneRequestIndex *index, int request_id, size_t *value);

int drone_history_append(DroneHistoryNode **head, DroneHistoryNode **tail, const char *message);
void drone_history_clear(DroneHistoryNode **head, DroneHistoryNode **tail);

#ifdef __cplusplus
}
#endif

#endif
