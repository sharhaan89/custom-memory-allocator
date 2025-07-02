#include "block_utils.h"
#include <unistd.h> 
#include <cstddef>

size_t align(size_t n) {
    return (n + sizeof(word_t) - 1) & ~(sizeof(word_t) - 1);
}

size_t allocSize(size_t size) {
    return align(sizeof(Block) + size - sizeof(word_t));
}

Block *requestFromOS(size_t size) {
    auto block = (Block *)sbrk(0);
    if(sbrk(allocSize(size)) == (void *)-1) {
        return nullptr;
    }
    return block;
}

Block *getHeader(word_t *data) {
    return (Block *)((char *)data - offsetof(Block, data));
}
