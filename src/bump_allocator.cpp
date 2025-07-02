#include "bump_allocator.h"
#include "block_utils.h"

word_t* BumpAllocator::alloc(size_t size) {
    size = align(size);

    auto block = requestFromOS(size);
    if(!block) return nullptr;

    block->size = size;
    block->used = true;
    block->next = nullptr;

    if(this->heapStart == nullptr) {
        this->heapStart = block;
    }

    if(this->top != nullptr) {
        this->top->next = block;
    }

    this->top = block;

    return block->data;
}

void BumpAllocator::free(word_t* data) {
    auto start = getHeader(data); //points to the starting of the block now
    start->used = false;
}