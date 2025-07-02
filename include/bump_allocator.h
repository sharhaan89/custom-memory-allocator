#pragma once

#include <cstddef>
#include "block_utils.h"

class BumpAllocator {
public:
    Block* top = nullptr;
    Block* heapStart = nullptr;
    word_t* alloc(size_t size);
    void free(word_t* data);
};
