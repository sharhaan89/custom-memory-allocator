#pragma once

#include <cstddef>
#include "block_utils.h"

namespace bump_allocator {
    word_t* alloc(size_t size);
    void free(word_t* data);
}
