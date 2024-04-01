#pragma once
#include <NEMain.h>

typedef struct {
    int x;
    int y;
    int z;
} AtomVertex;

typedef struct {
    AtomVertex *arr;
    int vertex_count;
} AtomModel;