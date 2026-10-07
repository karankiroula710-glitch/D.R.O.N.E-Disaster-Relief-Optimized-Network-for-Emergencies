#include "drone_structures.h"

#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int priority_before(DronePriorityItem left, DronePriorityItem right) {
    if (left.urgency != right.urgency) return left.urgency > right.urgency;
    if (left.received_order != right.received_order) return left.received_order < right.received_order;
    return left.request_id < right.request_id;
}

void drone_priority_queue_init(DronePriorityQueue *queue) {
    if (queue != NULL) queue->size = 0;
}

int drone_priority_queue_push(DronePriorityQueue *queue, DronePriorityItem item) {
    size_t child;
    if (queue == NULL || queue->size >= DRONE_PRIORITY_CAPACITY) return 0;
    child = queue->size++;
    while (child > 0) {
        size_t parent = (child - 1) / 2;
        if (!priority_before(item, queue->items[parent])) break;
        queue->items[child] = queue->items[parent];
        child = parent;
    }
    queue->items[child] = item;
    return 1;
}

int drone_priority_queue_pop(DronePriorityQueue *queue, DronePriorityItem *item) {
    DronePriorityItem last;
    size_t parent = 0;
    if (queue == NULL || item == NULL || queue->size == 0) return 0;
    *item = queue->items[0];
    last = queue->items[--queue->size];
    if (queue->size == 0) return 1;

    while (1) {
        size_t left = parent * 2 + 1;
        size_t right = left + 1;
        size_t best = parent;
        if (left < queue->size && priority_before(queue->items[left],
                                                   best == parent ? last : queue->items[best])) {
            best = left;
        }
        if (right < queue->size && priority_before(queue->items[right],
                                                    best == parent ? last : queue->items[best])) {
            best = right;
        }
        if (best == parent) break;
        queue->items[parent] = queue->items[best];
        parent = best;
    }
    queue->items[parent] = last;
    return 1;
}

int drone_priority_queue_is_empty(const DronePriorityQueue *queue) {
    return queue == NULL || queue->size == 0;
}

void drone_graph_init(DroneGraph *graph, int vertex_count) {
    int row, column;
    if (graph == NULL) return;
    if (vertex_count < 0) vertex_count = 0;
    if (vertex_count > DRONE_MAX_VERTICES) vertex_count = DRONE_MAX_VERTICES;
    graph->vertex_count = vertex_count;
    for (row = 0; row < DRONE_MAX_VERTICES; ++row) {
        for (column = 0; column < DRONE_MAX_VERTICES; ++column) {
            graph->weight[row][column] = 0.0;
        }
    }
}

int drone_graph_add_edge(DroneGraph *graph, int from, int to, double distance) {
    if (graph == NULL || from < 0 || to < 0 || from >= graph->vertex_count ||
        to >= graph->vertex_count || from == to || distance <= 0.0) return 0;
    graph->weight[from][to] = distance;
    graph->weight[to][from] = distance;
    return 1;
}

int drone_graph_shortest_path(const DroneGraph *graph, int start, int target,
                              int *path, size_t path_capacity, double *distance) {
    double best[DRONE_MAX_VERTICES];
    int previous[DRONE_MAX_VERTICES];
    unsigned char visited[DRONE_MAX_VERTICES] = {0};
    int reverse_path[DRONE_MAX_VERTICES];
    int i, current, count = 0;

    if (distance != NULL) *distance = DBL_MAX;
    if (graph == NULL || path == NULL || graph->vertex_count <= 0 ||
        start < 0 || target < 0 || start >= graph->vertex_count ||
        target >= graph->vertex_count) return -1;

    for (i = 0; i < graph->vertex_count; ++i) {
        best[i] = DBL_MAX;
        previous[i] = -1;
    }
    best[start] = 0.0;

    for (i = 0; i < graph->vertex_count; ++i) {
        int vertex;
        current = -1;
        for (vertex = 0; vertex < graph->vertex_count; ++vertex) {
            if (!visited[vertex] && (current < 0 || best[vertex] < best[current])) current = vertex;
        }
        if (current < 0 || best[current] == DBL_MAX) break;
        visited[current] = 1;
        if (current == target) break;

        for (vertex = 0; vertex < graph->vertex_count; ++vertex) {
            double edge = graph->weight[current][vertex];
            double candidate;
            if (edge <= 0.0 || visited[vertex]) continue;
            candidate = best[current] + edge;
            if (candidate < best[vertex]) {
                best[vertex] = candidate;
                previous[vertex] = current;
            }
        }
    }

    if (best[target] == DBL_MAX) return 0;
    current = target;
    while (current >= 0 && count < DRONE_MAX_VERTICES) {
        reverse_path[count++] = current;
        if (current == start) break;
        current = previous[current];
    }
    if (count == 0 || reverse_path[count - 1] != start) return 0;
    if ((size_t)count > path_capacity) return -1;

    for (i = 0; i < count; ++i) path[i] = reverse_path[count - i - 1];
    if (distance != NULL) *distance = best[target];
    return count;
}

void drone_request_queue_init(DroneRequestQueue *queue) {
    if (queue == NULL) return;
    queue->head = 0;
    queue->size = 0;
}

int drone_request_queue_push(DroneRequestQueue *queue, int request_id) {
    size_t tail;
    if (queue == NULL || queue->size >= DRONE_PENDING_CAPACITY) return 0;
    tail = (queue->head + queue->size) % DRONE_PENDING_CAPACITY;
    queue->request_ids[tail] = request_id;
    ++queue->size;
    return 1;
}

int drone_request_queue_pop(DroneRequestQueue *queue, int *request_id) {
    if (queue == NULL || request_id == NULL || queue->size == 0) return 0;
    *request_id = queue->request_ids[queue->head];
    queue->head = (queue->head + 1) % DRONE_PENDING_CAPACITY;
    --queue->size;
    return 1;
}

int drone_request_queue_is_empty(const DroneRequestQueue *queue) {
    return queue == NULL || queue->size == 0;
}

void drone_request_index_init(DroneRequestIndex *index) {
    if (index == NULL) return;
    memset(index->occupied, 0, sizeof(index->occupied));
    memset(index->keys, 0, sizeof(index->keys));
    memset(index->values, 0, sizeof(index->values));
}

int drone_request_index_insert(DroneRequestIndex *index, int request_id, size_t value) {
    size_t start, offset;
    if (index == NULL) return 0;
    start = (size_t)((unsigned int)request_id % DRONE_INDEX_CAPACITY);
    for (offset = 0; offset < DRONE_INDEX_CAPACITY; ++offset) {
        size_t slot = (start + offset) % DRONE_INDEX_CAPACITY;
        if (index->occupied[slot]) {
            if (index->keys[slot] == request_id) return 0;
            continue;
        }
        index->occupied[slot] = 1;
        index->keys[slot] = request_id;
        index->values[slot] = value;
        return 1;
    }
    return 0;
}

int drone_request_index_find(const DroneRequestIndex *index, int request_id, size_t *value) {
    size_t start, offset;
    if (index == NULL || value == NULL) return 0;
    start = (size_t)((unsigned int)request_id % DRONE_INDEX_CAPACITY);
    for (offset = 0; offset < DRONE_INDEX_CAPACITY; ++offset) {
        size_t slot = (start + offset) % DRONE_INDEX_CAPACITY;
        if (!index->occupied[slot]) return 0;
        if (index->keys[slot] == request_id) {
            *value = index->values[slot];
            return 1;
        }
    }
    return 0;
}

int drone_history_append(DroneHistoryNode **head, DroneHistoryNode **tail, const char *message) {
    DroneHistoryNode *node;
    if (head == NULL || tail == NULL || message == NULL) return 0;
    node = (DroneHistoryNode *)malloc(sizeof(*node));
    if (node == NULL) return 0;
    snprintf(node->message, sizeof(node->message), "%s", message);
    node->next = NULL;
    if (*tail == NULL) *head = node;
    else (*tail)->next = node;
    *tail = node;
    return 1;
}

void drone_history_clear(DroneHistoryNode **head, DroneHistoryNode **tail) {
    DroneHistoryNode *node;
    if (head == NULL || tail == NULL) return;
    node = *head;
    while (node != NULL) {
        DroneHistoryNode *next = node->next;
        free(node);
        node = next;
    }
    *head = NULL;
    *tail = NULL;
}
