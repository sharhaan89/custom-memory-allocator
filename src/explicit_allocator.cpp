#include "block_utils.h"
#include "explicit_allocator.h"
#include <iostream>

Block* ExplicitAllocator::findBlock(size_t size, FitFunction strategy) {
    return (this->*strategy)(size);
}

Block* ExplicitAllocator::firstFit(size_t size) {
    Block* block = this->freeListHead;
    
    while(block != nullptr) {
        if(!block->used && block->size >= size) {
            return block;
        }
        
        block = block->next;
    }

    return nullptr;
}

Block* ExplicitAllocator::nextFit(size_t size) {
    if(!this->searchStart) {
        return this->firstFit(size);
    }

    Block* block = this->searchStart;

    do {
        if(!block->used && block->size >= size) {
            return block;
        }
        block = block->next ? block->next : this->freeListHead;
    } while(block != this->searchStart);

    return nullptr;
}

Block* ExplicitAllocator::bestFit(size_t size) {
    Block* block = this->freeListHead;
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

Block* ExplicitAllocator::worstFit(size_t size) {
    Block* block = this->freeListHead;
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
Block* ExplicitAllocator::getPhysicalPreviousBlock(Block *block) {
    Block* prevBlock = nullptr;
    return nullptr;
}

Block* ExplicitAllocator::getPhysicalNextBlock(Block *block) {
    if(block == this->top) {
        return nullptr;
    }

    Block* nextBlock = reinterpret_cast<Block*>(
        reinterpret_cast<char*>(block) + sizeof(Block) + block->size - sizeof(word_t) 
    );

    return nextBlock;
}

bool ExplicitAllocator::canSplit(Block* block, size_t size) {
    if(block->used) return false;
    return (block->size >= sizeof(Block) + size);
} 

Block* ExplicitAllocator::split(Block* block, size_t size) {
    //this->removeFromFreeList(block);

    size_t originalBlockSize = block->size;

    char *newBlockPtr = reinterpret_cast<char*>(block->data) + size;
    Block *newBlock = reinterpret_cast<Block*>(newBlockPtr);
    
    newBlock->used = false;
    newBlock->size = sizeof(word_t) + originalBlockSize - size - sizeof(Block);

    if(block == this->top) this->top = newBlock;

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

bool ExplicitAllocator::canCoalesce(Block *block) {    
    if(block->used) return false;
    if(block == this->top) return false;

    Block* nextBlock = this->getPhysicalNextBlock(block);

    return nextBlock != nullptr && !nextBlock->used;
}

//TODO: COALESCE WITH PREVIOUS BLOCKS AS WELL
//ACTUALLY JUST COALESCE BY SCANNING THE WHOLE FREE LIST
//WILL DO THIS LATER
Block* ExplicitAllocator::coalesce(Block* block) {
    if(!this->canCoalesce(block)) return block;

    Block* nextBlock = this->getPhysicalNextBlock(block);
    this->removeFromFreeList(nextBlock);

    size_t HEADER_SIZE = sizeof(Block) - sizeof(word_t);
    block->size += nextBlock->size + HEADER_SIZE;

    if(nextBlock == this->top) this->top = block;

    return block;
}

void ExplicitAllocator::removeFromFreeList(Block* block) {
    Block* prevBlock = block->prev;
    Block* nextBlock = block->next;

    if(prevBlock) {
        prevBlock->next = nextBlock;
    } else {
        this->freeListHead = nextBlock;
    }

    if(nextBlock) nextBlock->prev = prevBlock;
}

void ExplicitAllocator::addToFreeList(Block* block) {
    block->next = this->freeListHead;
    block->prev = nullptr;
    
    if(this->freeListHead) {
        this->freeListHead->prev = block;
    }

    this->freeListHead = block;
}

word_t* ExplicitAllocator::alloc(size_t size) {
    size = align(size);

    if(auto block = this->findBlock(size, &ExplicitAllocator::firstFit)) {
        if(this->canSplit(block, size)) {
            block = this->split(block, size);
            Block* newBlock = this->getPhysicalNextBlock(block);
            this->addToFreeList(newBlock);
        }   
        
        if(block->next) searchStart = block->next;
        else searchStart = freeListHead;
        this->removeFromFreeList(block);
        this->lastAllocated = block;
        block->used = true;

        return block->data;
    }   

    auto block = requestFromOS(size);
    if(!block) return nullptr;

    block->size = size;
    block->used = true;
    block->next = nullptr;
    block->prev = this->top;

    if(this->heapStart == nullptr) {
        this->heapStart = block;
    }

    if(this->top != nullptr) {
        this->top->next = block;
    }

    this->lastAllocated = block;
    this->top = block;

    return block->data;
}

void ExplicitAllocator::free(word_t* data) {
    Block* block = getHeader(data);
    block->used = false;

    if(this->canCoalesce(block)) {
        this->coalesce(block);
        return;
    } 

    this->addToFreeList(block);
}