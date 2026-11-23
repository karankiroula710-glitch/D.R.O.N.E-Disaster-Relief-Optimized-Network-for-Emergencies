#include "drone_structures.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t map_slot(int key) {
    unsigned int value = (unsigned int)key;
    return (size_t)((value * 2654435761u) % DRONE_MAX_ITEMS);
}

void drone_id_map_init(DroneIdMap *map) {
    if (map != NULL) memset(map, 0, sizeof(*map));
}

int drone_id_map_put(DroneIdMap *map, int key, int value) {
    size_t start, step;
    if (map == NULL) return 0;
    start = map_slot(key);
    for (step = 0; step < DRONE_MAX_ITEMS; ++step) {
        size_t index = (start + step) % DRONE_MAX_ITEMS;
        if (!map->occupied[index] || map->keys[index] == key) {
            map->occupied[index] = 1;
            map->keys[index] = key;
            map->values[index] = value;
            return 1;
        }
    }
    return 0;
}

int drone_id_map_get(const DroneIdMap *map, int key, int *out_value) {
    size_t start, step;
    if (map == NULL || out_value == NULL) return 0;
    start = map_slot(key);
    for (step = 0; step < DRONE_MAX_ITEMS; ++step) {
        size_t index = (start + step) % DRONE_MAX_ITEMS;
        if (!map->occupied[index]) return 0;
        if (map->keys[index] == key) { *out_value = map->values[index]; return 1; }
    }
    return 0;
}

void drone_history_init(DroneEventHistory *history) {
    if (history != NULL) { history->head = NULL; history->tail = NULL; history->count = 0; }
}

int drone_history_append(DroneEventHistory *history, const char *text) {
    DroneEventNode *node;
    if (history == NULL || text == NULL) return 0;
    node = (DroneEventNode *)malloc(sizeof(*node));
    if (node == NULL) return 0;
    snprintf(node->text, sizeof(node->text), "%s", text);
    node->next = NULL;
    if (history->tail != NULL) history->tail->next = node;
    else history->head = node;
    history->tail = node;
    ++history->count;
    return 1;
}

void drone_history_print(const DroneEventHistory *history) {
    const DroneEventNode *node;
    if (history == NULL || history->head == NULL) { puts("No events recorded this session."); return; }
    for (node = history->head; node != NULL; node = node->next) puts(node->text);
}

void drone_history_clear(DroneEventHistory *history) {
    DroneEventNode *node;
    if (history == NULL) return;
    node = history->head;
    while (node != NULL) {
        DroneEventNode *next = node->next;
        free(node);
        node = next;
    }
    history->head = history->tail = NULL;
    history->count = 0;
}
