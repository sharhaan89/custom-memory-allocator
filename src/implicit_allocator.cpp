#include "block_utils.h"
#include "implicit_allocator.h"

//uses the strategy function as passed
Block* ImplicitAllocator::findBlock(size_t size, FitFunction strategy) {
    return (this->*strategy)(size);
}

//return the first empty block that can fit the requirement
Block* ImplicitAllocator::firstFit(size_t size) {
    Block* block = this->heapStart;
    
    while(block != nullptr) {
        if(!block->used && block->size >= size) {
            return block;
        }
        
        block = block->next;
    }

    return nullptr;
}

//returns the first empty block starting from the last allocated block that can fit the requirement
Block* ImplicitAllocator::nextFit(size_t size) {
    if(!this->lastAllocated) return nullptr; // no blocks yet

    Block* block = this->lastAllocated->next ? this->lastAllocated->next : this->heapStart;
    Block* start = block;

    do {
        if(!block->used && block->size >= size) {
            return block;
        }

        block = block->next ? block->next : this->heapStart;
    } while(block != start);

    return nullptr;
}

//returns the block that can fit the requirement and of the smallest possible size
Block* ImplicitAllocator::bestFit(size_t size) {
    Block* block = this->heapStart;
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

Block* ImplicitAllocator::worstFit(size_t size) {
    Block* block = this->heapStart;
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

/*
Block = header + data + extra data

block1 = header + data + size - data -> to allocate
block2 = header + data

therefore, Block - block1 >= block2
Block >= block1 + block2
header + extra data >= 2header + size
extra data >= header + data + size - data

block->size >= sizeof(Block) + size 
Block->size = data + extra data */
bool ImplicitAllocator::canSplit(Block* block, size_t size) {
    return (block->size >= sizeof(Block) + size);
} 

/* 
block1 block2
i will allocate to block1

block1 -> sizeof(Block) + size - sizeof(word_t)
block2 starts from offset + size of above block

sizeof(Block) + block->size - sizeof(word_t)
block1 = sizeof(Block) + size - sizeof(word_t)
block2 = sizeof(Block) + x - sizeof(word_t)

sizeof(Block) + originalBlockSize - sizeof(word_t) = 
2sizeof(Block) + size + x - 2sizeof(word_t)

x = sizeof(word_t) + block->size - size - sizeof(Block) */
Block* ImplicitAllocator::split(Block* block, size_t size) {
    Block* originalNextBlock = nullptr;
    size_t originalBlockSize = block->size;

    if(block->next != nullptr) {
        originalNextBlock = block->next;
    }

    char *newBlockPtr = reinterpret_cast<char*>(block->data) + size;
    Block *newBlock = reinterpret_cast<Block*>(newBlockPtr);
    
    newBlock->used = false;
    newBlock->size = sizeof(word_t) + originalBlockSize - size - sizeof(Block);
    newBlock->next = originalNextBlock;

    block->size = size;
    block->next = newBlock;

    return block;
}

word_t* ImplicitAllocator::alloc(size_t size) {
    size = align(size);

    if(auto block = this->findBlock(size, &ImplicitAllocator::firstFit)) {
        if(this->canSplit(block, size)) block = this->split(block, size);
        this->lastAllocated = block;
        block->used = true;
        return block->data;
    }   

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

    this->lastAllocated = block;
    this->top = block;

    return block->data;
}

bool ImplicitAllocator::canCoalesce(Block *block) {
    if(block->next == nullptr) return false;
    if(block->used || block->next->used) return false; 
    
    //physical check for more security
    char *blockEnd = reinterpret_cast<char*>(block) + sizeof(Block) + block->size - sizeof(word_t); 
    return (reinterpret_cast<char*>(block->next) == blockEnd);
}

/*
block1 -> header1 + payload1
block2 -> header2 + payload2

block -> header + payload
header = header1
payload = payload1 + header2 + payload2

we don't need the header of the second block
so we can utilise it for the user payload memory*/
Block* ImplicitAllocator::coalesce(Block* block) {
    Block* nextBlock = block->next;

    size_t HEADER_SIZE = sizeof(Block) - sizeof(word_t);
    block->size += nextBlock->size + HEADER_SIZE;
    block->next = nextBlock->next;

    return block;
}

void ImplicitAllocator::free(word_t* data) {
    Block* block = getHeader(data); //points to the starting of the block now
    block->used = false;

    if(this->canCoalesce(block)) {
        this->coalesce(block);
    }
}