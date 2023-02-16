#ifndef MEMORY_H
#define MEMORY_H

#include <stdlib.h>
#include <stdbool.h>

#include "constants.h"

typedef struct {
    int words[WORDS_SIZE];
} MemoryBlock;

typedef struct {
    MemoryBlock* blocks;
    int size;
} RAM;

typedef struct {
    MemoryBlock block;
    int tag; /* Address of the block in memory RAM */
    bool updated;
    int cost;
    int cacheHit;
} Line;

typedef struct {
    Line* lines;
    int size;
} Cache;

void startCache(Cache*, int);
void stopCache(Cache*);

void startRAM(RAM*, int);
void stopRAM(RAM*);

#endif // !MEMORY_H