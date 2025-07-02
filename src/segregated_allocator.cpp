#include "segregated_allocator.h"

//8 16 32 64 128 >128
//0  1  2  3   4    5
int SegregatedListAllocator::getBucket(size_t size) {
    if(size <= 8) return 0;
    if(size <= 16) return 1;
    if(size <= 32) return 2;
    if(size <= 64) return 3;
    if(size <= 128) return 4;
    return 5;
}

word_t* SegregatedListAllocator::alloc(size_t size) {
    int bucket = getBucket(size);
    return segregatedList[bucket].alloc(size);
}

void SegregatedListAllocator::free(word_t* data) {
    Block* block = getHeader(data);
    int bucket = getBucket(block->size);
    segregatedList[bucket].free(data);
}