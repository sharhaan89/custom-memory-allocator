#pragma once

#include <cstddef>
#include "block_utils.h"

class ExplicitAllocator {
public:
    using FitFunction = Block* (ExplicitAllocator::*)(size_t);

    Block* top = nullptr;
    Block* heapStart = nullptr;
    Block* freeListHead = nullptr;
    Block* lastAllocated = nullptr;
    Block* searchStart = nullptr;

    enum class SearchMode {
        FirstFit,
        NextFit,
        BestFit,
        WorstFit
    };

    Block* findBlock(size_t size, FitFunction strategy);
    Block* firstFit(size_t size);
    Block* nextFit(size_t size);
    Block* bestFit(size_t size);
    Block* worstFit(size_t size);

    Block* getPhysicalPreviousBlock(Block* block);
    Block* getPhysicalNextBlock(Block* block);

    bool canSplit(Block* block, size_t size);
    bool canCoalesce(Block* block);
    Block* split(Block* block, size_t size);
    Block* coalesce(Block* block);

    void removeFromFreeList(Block* block);
    void addToFreeList(Block* block);
    
    word_t* alloc(size_t size);
    void free(word_t* data);
};
