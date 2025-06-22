#include "block_utils.h"
#include "explicit_allocator.h"

using FitFunction = Block* (*)(size_t); 

static Block* heapStart = nullptr;
static Block* top = nullptr; 
static Block* freeListHead = nullptr;
static Block* lastAllocated = nullptr;

enum class SearchMode {
    FirstFit,
    NextFit,
    BestFit,
    WorstFit
};

Block* findBlock(size_t size, FitFunction strategy) {
    return strategy(size);
}

Block* firstFit(size_t size) {
    Block* block = freeListHead;
    
    while(block != nullptr) {
        if(!block->used && block->size >= size) {
            return block;
        }
        
        block = block->next;
    }

    return nullptr;
}

Block* nextFit(size_t size) {
    if(!lastAllocated) {
        return firstFit(size);
    }

    Block* block = lastAllocated->next ? lastAllocated->next : freeListHead;
    Block* start = block;

    do {
        if(!block->used && block->size >= size) {
            return block;
        }

        block = block->next ? block->next : freeListHead;
    } while(block != start);

    return nullptr;
}

Block *bestFit(size_t size) {
    Block* block = freeListHead;
    Block* resBlock = nullptr;

    while(block != nullptr) {
        if(!block->used && block->size >= size) {
            if(resBlock == nullptr || block->size < resBlock->size) {
                resBlock = block;
            }
        }
        
        block = block->next;
    }

    return resBlock;
}

Block *worstFit(size_t size) {
    Block* block = freeListHead;
    Block* resBlock = nullptr;

    while(block != nullptr) {
        if(!block->used && block->size >= size) {
            if(resBlock == nullptr || block->size > resBlock->size) {
                resBlock = block;
            }
        }
        
        block = block->next;
    }

    return resBlock;
}

//TODO: need to figure out a way to get this later
Block* getPhysicalPreviousBlock(Block *block) {
    Block* prevBlock = nullptr;
    return nullptr;
}

Block* getPhysicalNextBlock(Block *block) {
    if(block == top) {
        return nullptr;
    }

    Block* nextBlock = reinterpret_cast<Block*>(
        reinterpret_cast<char*>(block) + sizeof(Block) + block->size - sizeof(word_t) 
    );

    return nextBlock;
}

bool canSplit(Block* block, size_t size) {
    if(block->used) return false;
    return (block->size >= sizeof(Block) + size);
} 

Block* split(Block* block, size_t size) {
    removeFromFreeList(block);

    size_t originalBlockSize = block->size;

    char *newBlockPtr = reinterpret_cast<char*>(block->data) + size;
    Block *newBlock = reinterpret_cast<Block*>(newBlockPtr);
    
    newBlock->used = false;
    newBlock->size = sizeof(word_t) + originalBlockSize - size - sizeof(Block);
    addToFreeList(newBlock);

    if(block == top) top = newBlock;

    block->size = size;

    return block;
}

bool canCoalesce(Block *block) {    
    if(block->used) return false;
    if(block == top) return false;

    Block* nextBlock = getPhysicalNextBlock(block);

    return !nextBlock->used;
}

Block* coalesce(Block* block) {
    if(!canCoalesce(block)) return block;

    Block* nextBlock = getPhysicalNextBlock(block);

    size_t HEADER_SIZE = sizeof(Block) - sizeof(word_t);
    block->size += nextBlock->size + HEADER_SIZE;
    block->next = nextBlock->next;

    if(nextBlock == top) top = block;

    return block;
}

void removeFromFreeList(Block* block) {
    Block* prevBlock = block->prev;
    Block* nextBlock = block->next;

    if(prevBlock) {
        prevBlock->next = nextBlock;
    } else {
        freeListHead = nextBlock;
    }

    if(nextBlock) nextBlock->prev = prevBlock;
}

void addToFreeList(Block* block) {
    block->next = freeListHead;
    block->prev = nullptr;
    
    if(freeListHead) {
        freeListHead->prev = block;
    }

    freeListHead = block;
}

word_t *alloc(size_t size) {
    size = align(size);

    if(auto block = findBlock(size, firstFit)) {
        if(canSplit(block, size)) {
            block = split(block, size);
            Block* newBlock = getPhysicalNextBlock(block);
            removeFromFreeList(block);
            addToFreeList(newBlock);
        }

        lastAllocated = block;
        block->used = true;

        return block->data;
    }   

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

    lastAllocated = block;
    top = block;

    return block->data;
}

void free(word_t* data) {
    Block* block = getHeader(data);
    block->used = false;

    if(canCoalesce(block)) {
        coalesce(block);
        return;
    } 

    addToFreeList(block);
}