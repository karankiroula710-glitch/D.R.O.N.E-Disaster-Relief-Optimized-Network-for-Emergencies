#ifndef DRONE_STRUCTURES_H
#define DRONE_STRUCTURES_H

#define MAX_ZONES 100

typedef struct {
    int id;
    char name[50];
    int is_accessible;
} Zone;

typedef struct {
    int source;
    int destination;
    float distance;
    int is_available;
} Route;

typedef struct {
    int id;
    float battery;
    float capacity;
} Drone;

#endif
