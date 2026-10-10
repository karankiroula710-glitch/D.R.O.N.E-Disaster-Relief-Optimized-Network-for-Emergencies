#ifndef DRONE_STRUCTURES_H
#define DRONE_STRUCTURES_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DRONE_MAX_ZONES 8
#define DRONE_MAX_ITEMS 128
#define DRONE_ZONE_NAME 40
#define DRONE_EVENT_TEXT 160

typedef struct {
    int request_id;
    int severity;
    unsigned long arrival_order;
} DronePriorityItem;

typedef struct {
    DronePriorityItem items[DRONE_MAX_ITEMS];
    size_t size;
} DronePriorityHeap;

void drone_heap_init(DronePriorityHeap *heap);
int drone_heap_push(DronePriorityHeap *heap, DronePriorityItem item);
int drone_heap_pop(DronePriorityHeap *heap, DronePriorityItem *out_item);
size_t drone_heap_size(const DronePriorityHeap *heap);

typedef struct {
    int request_ids[DRONE_MAX_ITEMS];
    size_t head;
    size_t tail;
    size_t count;
} DronePendingQueue;

void drone_queue_init(DronePendingQueue *queue);
int drone_queue_enqueue(DronePendingQueue *queue, int request_id);
int drone_queue_dequeue(DronePendingQueue *queue, int *request_id);
size_t drone_queue_size(const DronePendingQueue *queue);

typedef struct {
    size_t zone_count;
    char names[DRONE_MAX_ZONES][DRONE_ZONE_NAME];
    int weights[DRONE_MAX_ZONES][DRONE_MAX_ZONES];
} DroneZoneGraph;

void drone_graph_init_demo(DroneZoneGraph *graph);
int drone_graph_add_road(DroneZoneGraph *graph, int from, int to, int distance);
int drone_graph_shortest_path(const DroneZoneGraph *graph, int start, int goal,
                              int *out_path, size_t *in_out_length,
                              int *out_distance);

typedef struct {
    int keys[DRONE_MAX_ITEMS];
    int values[DRONE_MAX_ITEMS];
    unsigned char occupied[DRONE_MAX_ITEMS];
} DroneIdMap;

void drone_id_map_init(DroneIdMap *map);
int drone_id_map_put(DroneIdMap *map, int key, int value);
int drone_id_map_get(const DroneIdMap *map, int key, int *out_value);

typedef struct DroneEventNode {
    char text[DRONE_EVENT_TEXT];
    struct DroneEventNode *next;
} DroneEventNode;

typedef struct {
    DroneEventNode *head;
    DroneEventNode *tail;
    size_t count;
} DroneEventHistory;

void drone_history_init(DroneEventHistory *history);
int drone_history_append(DroneEventHistory *history, const char *text);
void drone_history_print(const DroneEventHistory *history);
void drone_history_clear(DroneEventHistory *history);

#ifdef __cplusplus
}
#endif

#endif
