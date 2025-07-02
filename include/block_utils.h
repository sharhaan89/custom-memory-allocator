#pragma once

#include <cstddef>
#include <cstdint>

using word_t = intptr_t;

struct Block {
    size_t size;
    bool used;
    Block *prev;
    Block *next;
    word_t data[1];
};

//extern Block* heapStart;
//extern Block* top;

size_t align(size_t n);
size_t allocSize(size_t size);
Block* requestFromOS(size_t size);
Block* getHeader(word_t *data); 