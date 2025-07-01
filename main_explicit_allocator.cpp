#include <iostream>
#include <vector>
#include <cassert>
#include <cstring>
#include "explicit_allocator.h"
#include "block_utils.h"

using namespace explicit_allocator;

// Test helper functions
void printHeapState() {
    std::cout << "\n--- Heap State ---\n";
    std::cout << "Free List: ";
    Block* current = freeListHead;
    int count = 0;
    while(current && count < 20) {  // Limit iterations!
        std::cout << "[" << current << ": size=" << current->size << "] -> ";
        current = current->next;
        count++;
    }
    if(count >= 20) std::cout << "... (INFINITE LOOP DETECTED!)";
    std::cout << "NULL\n";
}

void resetHeap() {
    // Simple reset - in practice you'd need proper cleanup
    freeListHead = nullptr;
    lastAllocated = nullptr;
    heapStart = nullptr;
    top = nullptr;
}

// Test basic allocation and deallocation
void testBasicAllocation() {
    std::cout << "\n=== Testing Basic Allocation ===\n";
    resetHeap();
    
    // Test simple allocation
    word_t* ptr1 = alloc(64);
    assert(ptr1 != nullptr);
    std::cout << "✓ Basic allocation successful\n";
    
    // Write to allocated memory
    *ptr1 = 0xDEADBEEF;
    assert(*ptr1 == 0xDEADBEEF);
    std::cout << "✓ Memory write/read successful\n";
    
    // Test multiple allocations
    word_t* ptr2 = alloc(128);
    word_t* ptr3 = alloc(256);
    assert(ptr2 != nullptr && ptr3 != nullptr);
    assert(ptr1 != ptr2 && ptr2 != ptr3 && ptr1 != ptr3);
    std::cout << "✓ Multiple allocations successful\n";
    
    printHeapState();
    
    // Test deallocation
    free(ptr2);
    std::cout << "✓ Deallocation successful\n";
    printHeapState();
}

// Test fragmentation and coalescing
void testCoalescing() {
    std::cout << "\n=== Testing Coalescing ===\n";
    resetHeap();
    
    // Allocate several blocks
    word_t* ptr1 = alloc(100);
    word_t* ptr2 = alloc(100);
    word_t* ptr3 = alloc(100);
    word_t* ptr4 = alloc(100);
    
    std::cout << "Allocated 4 blocks of 100 bytes each\n";
    printHeapState();
    
    // Free middle blocks to create fragmentation
    free(ptr2);
    free(ptr3);
    std::cout << "Freed middle two blocks\n";
    printHeapState();
    
    // Free adjacent block to test coalescing
    free(ptr4);
    std::cout << "Freed last block - should coalesce with previous free blocks\n";
    printHeapState();
    
    // Try to allocate a large block that should fit in coalesced space
    word_t* large_ptr = alloc(250);
    if(large_ptr) {
        std::cout << "✓ Successfully allocated large block after coalescing\n";
    } else {
        std::cout << "✗ Failed to allocate large block - coalescing may not be working\n";
    }
    printHeapState();
}

// Test different fit strategies
void testFitStrategies() {
    std::cout << "\n=== Testing Fit Strategies ===\n";
    resetHeap();
    
    // Create some fragmented free blocks
    word_t* ptrs[6];
    for(int i = 0; i < 6; i++) {
        ptrs[i] = alloc(50 + i * 20); // Different sizes: 50, 70, 90, 110, 130, 150
    }
    
    // Free every other block to create fragmentation
    free(ptrs[1]); // 70 bytes
    free(ptrs[3]); // 110 bytes
    free(ptrs[5]); // 150 bytes
    
    std::cout << "Created fragmented heap with free blocks of sizes 70, 110, 150\n";
    printHeapState();
    
    // Test first fit
    Block* block = firstFit(80);
    std::cout << "First fit for 80 bytes: " << (block ? "found block of size " + std::to_string(block->size) : "not found") << "\n";
    
    // Test best fit
    block = bestFit(80);
    std::cout << "Best fit for 80 bytes: " << (block ? "found block of size " + std::to_string(block->size) : "not found") << "\n";
    
    // Test worst fit
    block = worstFit(80);
    std::cout << "Worst fit for 80 bytes: " << (block ? "found block of size " + std::to_string(block->size) : "not found") << "\n";
}

// Test splitting
void testSplitting() {
    std::cout << "\n=== Testing Block Splitting ===\n";
    resetHeap();
    
    // Allocate and free a large block
    word_t* large_ptr = alloc(1000);
    free(large_ptr);
    
    std::cout << "Created large free block of 1000 bytes\n";
    printHeapState();
    
    // Allocate a smaller block - should split the large one
    word_t* small_ptr = alloc(200);
    assert(small_ptr != nullptr);
    
    std::cout << "Allocated 200 bytes - should split the large block\n";
    printHeapState();
    
    // Check if we can still allocate more from the remaining space
    word_t* another_ptr = alloc(300);
    if(another_ptr) {
        std::cout << "✓ Successfully allocated from remaining split block\n";
    } else {
        std::cout << "✗ Failed to allocate from remaining space\n";
    }
    printHeapState();
}

// Test edge cases
void testEdgeCases() {
    std::cout << "\n=== Testing Edge Cases ===\n";
    resetHeap();
    
    // Test zero allocation
    word_t* zero_ptr = alloc(0);
    std::cout << "Zero allocation: " << (zero_ptr ? "succeeded" : "failed") << "\n";
    
    // Test very small allocation
    word_t* tiny_ptr = alloc(1);
    assert(tiny_ptr != nullptr);
    std::cout << "✓ Tiny allocation (1 byte) successful\n";
    
    // Test large allocation
    word_t* huge_ptr = alloc(10000);
    assert(huge_ptr != nullptr);
    std::cout << "✓ Large allocation (10KB) successful\n";
    
    // Test alignment
    for(int i = 1; i <= 20; i++) {
        word_t* ptr = alloc(i);
        assert(ptr != nullptr);
        assert(((uintptr_t)ptr) % sizeof(word_t) == 0);
    }
    std::cout << "✓ All allocations properly aligned\n";
    
    printHeapState();
}

// Test next fit strategy
void testNextFit() {
    std::cout << "\n=== Testing Next Fit Strategy ===\n";
    resetHeap();
    
    // Create fragmented memory
    word_t* ptrs[8];
    for(int i = 0; i < 8; i++) {
        ptrs[i] = alloc(64);
    }
    
    // Free every other block
    for(int i = 1; i < 8; i += 2) {
        free(ptrs[i]);
    }
    
    std::cout << "Created fragmented memory with alternating free blocks\n";
    printHeapState();
    
    // Test next fit behavior
    word_t* next1 = alloc(32);
    word_t* next2 = alloc(32);
    word_t* next3 = alloc(32);
    
    std::cout << "Allocated 3 blocks using next fit\n";
    std::cout << "Pointers: " << next1 << ", " << next2 << ", " << next3 << "\n";
    printHeapState();
}

// Performance test
void testPerformance() {
    std::cout << "\n=== Performance Test ===\n";
    resetHeap();
    
    const int NUM_ALLOCS = 100;
    std::vector<word_t*> ptrs;
    
    // Allocate many blocks
    for(int i = 0; i < NUM_ALLOCS; i++) {
        word_t* ptr = alloc(64 + (i % 100));
        if(ptr) {
            ptrs.push_back(ptr);
            // Write some data to ensure the memory is valid
            *ptr = i;
        }
    }
    
    std::cout << "Allocated " << ptrs.size() << " blocks\n";
    
    // Free half of them randomly
    for(size_t i = 0; i < ptrs.size(); i += 2) {
        free(ptrs[i]);
    }
    
    std::cout << "Freed half the blocks\n";
    
    // Try to allocate more
    int successful_allocs = 0;
    for(int i = 0; i < 100; i++) {
        word_t* ptr = alloc(128);
        if(ptr) {
            successful_allocs++;
            *ptr = i + 10000;
        }
    }
    
    std::cout << "Successfully allocated " << successful_allocs << " additional blocks\n";
}

int main() {
    std::cout << "Starting Explicit Allocator Tests\n";
    std::cout << "===================================\n";
    
    try {
        testBasicAllocation();
        testCoalescing();
        testFitStrategies();
        testSplitting();
        testEdgeCases();
        testNextFit();
        testPerformance();
        
        std::cout << "\n===================================\n";
        std::cout << "All tests completed!\n";
        std::cout << "Review the output above for any issues.\n";
        
    } catch(const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}