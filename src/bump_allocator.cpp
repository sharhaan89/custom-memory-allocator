#include "bump_allocator.h"
#include "block_utils.h"

word_t *alloc(size_t size) {
    size = align(size);

    auto block = requestFromOS(size);
    if(!block) return nullptr;

    block->size = size;
    block->used = true;
    block->next = nullptr;

    if(heapStart == nullptr) {
        heapStart = block;
    }

    if(top != nullptr) {
        top->next = block;
    }

    top = block;

    return block->data;
}

void free(word_t* data) {
    auto start = getHeader(data); //points to the starting of the block now
    start->used = false;
}