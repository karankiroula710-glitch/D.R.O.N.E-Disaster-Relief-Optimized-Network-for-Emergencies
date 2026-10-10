#include "drone_structures.h"

void drone_queue_init(DronePendingQueue *queue) {
    if (queue == NULL) return;
    queue->head = queue->tail = queue->count = 0;
}

int drone_queue_enqueue(DronePendingQueue *queue, int request_id) {
    if (queue == NULL || queue->count >= DRONE_MAX_ITEMS) return 0;
    queue->request_ids[queue->tail] = request_id;
    queue->tail = (queue->tail + 1) % DRONE_MAX_ITEMS;
    ++queue->count;
    return 1;
}

int drone_queue_dequeue(DronePendingQueue *queue, int *request_id) {
    if (queue == NULL || request_id == NULL || queue->count == 0) return 0;
    *request_id = queue->request_ids[queue->head];
    queue->head = (queue->head + 1) % DRONE_MAX_ITEMS;
    --queue->count;
    return 1;
}

size_t drone_queue_size(const DronePendingQueue *queue) {
    return queue == NULL ? 0 : queue->count;
}

