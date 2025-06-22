#pragma once

#include <cstddef>
#include "block_utils.h"

namespace explicit_allocator {
    using FitFunction = Block* (*)(size_t);

    extern Block* lastAllocated;

    enum class SearchMode {
        FirstFit,
        NextFit,
        BestFit,
        WorstFit
    };

    word_t* alloc(size_t size);
    void free(word_t* data);

    Block* firstFit(size_t size);
    Block* nextFit(size_t size);
    Block* bestFit(size_t size);
    Block* worstFit(size_t size);

    Block* findBlock(size_t size, FitFunction strategy);
}
