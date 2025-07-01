#include "block_utils.h"
#include "explicit_allocator.h"
#include <iostream>

namespace explicit_allocator {

Block* freeListHead = nullptr;
Block* lastAllocated = nullptr;

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

    Block* block = lastAllocated;

    do {
        if(!block->used && block->size >= size) {
            lastAllocated = block;
            return block;
        }
        block = block->next ? block->next : freeListHead;
    } while(block != lastAllocated);

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

    if(block == top) top = newBlock;

    block->size = size;

    return block;
}

/*
bool canCoalesce(Block *block) {    
    std::cout << "Checking coalesce for block " << block << ", used=" << block->used << std::endl;
    if(block->used) return false;
    if(block == top) {
        std::cout << "Block is top, cannot coalesce" << std::endl;
        return false;
    }
    Block* nextBlock = getPhysicalNextBlock(block);
    std::cout << "Next block: " << nextBlock << ", used=" << (nextBlock ? nextBlock->used : -1) << std::endl;
    
    return nextBlock != nullptr && !nextBlock->used;
}
*/

bool canCoalesce(Block *block) {    
    if(block->used) return false;
    if(block == top) return false;

    Block* nextBlock = getPhysicalNextBlock(block);

    return nextBlock != nullptr && !nextBlock->used;
}

//TODO: COALESCE WITH PREVIOUS BLOCKS AS WELL
//ACTUALLY JUST COALESCE BY SCANNING THE WHOLE FREE LIST
//WILL DO THIS LATER
Block* coalesce(Block* block) {
    if(!canCoalesce(block)) return block;

    Block* nextBlock = getPhysicalNextBlock(block);
    removeFromFreeList(nextBlock);

    size_t HEADER_SIZE = sizeof(Block) - sizeof(word_t);
    block->size += nextBlock->size + HEADER_SIZE;

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
            removeFromFreeList(block);
            block = split(block, size);
            Block* newBlock = getPhysicalNextBlock(block);
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
    block->prev = top;

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

}