#pragma once

#include <cstddef>
#include "block_utils.h"
#include "explicit_allocator.h"

class SegregatedListAllocator {
private:
    static const int NUM_BUCKETS = 6;
    ExplicitAllocator segregatedList[NUM_BUCKETS];
    
    int getBucket(size_t size);
    
public:
    word_t* alloc(size_t size);
    void free(word_t* data);
};