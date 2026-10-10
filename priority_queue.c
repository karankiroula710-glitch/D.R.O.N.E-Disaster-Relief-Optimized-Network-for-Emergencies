#include "drone_structures.h"

static int item_precedes(DronePriorityItem left, DronePriorityItem right) {
    if (left.severity != right.severity) return left.severity > right.severity;
    return left.arrival_order < right.arrival_order;
}

void drone_heap_init(DronePriorityHeap *heap) {
    if (heap != NULL) heap->size = 0;
}

int drone_heap_push(DronePriorityHeap *heap, DronePriorityItem item) {
    size_t index;
    if (heap == NULL || heap->size >= DRONE_MAX_ITEMS) return 0;
    index = heap->size++;
    while (index > 0) {
        size_t parent = (index - 1) / 2;
        if (!item_precedes(item, heap->items[parent])) break;
        heap->items[index] = heap->items[parent];
        index = parent;
    }
    heap->items[index] = item;
    return 1;
}

int drone_heap_pop(DronePriorityHeap *heap, DronePriorityItem *out_item) {
    DronePriorityItem last;
    size_t index = 0;
    if (heap == NULL || out_item == NULL || heap->size == 0) return 0;
    *out_item = heap->items[0];
    last = heap->items[--heap->size];
    if (heap->size == 0) return 1;
    while (index * 2 + 1 < heap->size) {
        size_t child = index * 2 + 1;
        if (child + 1 < heap->size && item_precedes(heap->items[child + 1], heap->items[child]))
            ++child;
        if (!item_precedes(heap->items[child], last)) break;
        heap->items[index] = heap->items[child];
        index = child;
    }
    heap->items[index] = last;
    return 1;
}

size_t drone_heap_size(const DronePriorityHeap *heap) {
    return heap == NULL ? 0 : heap->size;
}

