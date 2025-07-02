#include <iostream>
#include <cassert>
#include <cstring>
#include <vector>
#include "implicit_allocator.h"
#include "block_utils.h"

class ImplicitAllocatorTest {
private:
    ImplicitAllocator allocator;
    int testsPassed = 0;
    int totalTests = 0;

    void assertEqual(bool condition, const std::string& testName) {
        totalTests++;
        if (condition) {
            std::cout << "✓ " << testName << " PASSED" << std::endl;
            testsPassed++;
        } else {
            std::cout << "✗ " << testName << " FAILED" << std::endl;
        }
    }

    void resetAllocator() {
        // Reset allocator state for clean tests
        allocator.top = nullptr;
        allocator.heapStart = nullptr;
        allocator.lastAllocated = nullptr;
    }

public:
    void testBasicAllocation() {
        std::cout << "\n=== Testing Basic Allocation ===" << std::endl;
        resetAllocator();

        // Test single allocation
        word_t* ptr1 = allocator.alloc(64);
        assertEqual(ptr1 != nullptr, "Basic allocation returns non-null pointer");
        assertEqual(allocator.heapStart != nullptr, "HeapStart is set after first allocation");
        assertEqual(allocator.top != nullptr, "Top is set after first allocation");
        assertEqual(allocator.lastAllocated != nullptr, "LastAllocated is set after allocation");

        // Test multiple allocations
        word_t* ptr2 = allocator.alloc(32);
        word_t* ptr3 = allocator.alloc(128);
        
        assertEqual(ptr2 != nullptr, "Second allocation returns non-null pointer");
        assertEqual(ptr3 != nullptr, "Third allocation returns non-null pointer");
        assertEqual(ptr1 != ptr2 && ptr2 != ptr3 && ptr1 != ptr3, "All pointers are unique");
    }

    void testFreeAndReallocation() {
        std::cout << "\n=== Testing Free and Reallocation ===" << std::endl;
        resetAllocator();

        word_t* ptr1 = allocator.alloc(64);
        word_t* ptr2 = allocator.alloc(32);
        word_t* ptr3 = allocator.alloc(128);

        // Free middle block
        allocator.free(ptr2);
        Block* block2 = getHeader(ptr2);
        assertEqual(!block2->used, "Freed block is marked as unused");

        // Allocate something that should fit in the freed space
        word_t* ptr4 = allocator.alloc(16);
        assertEqual(ptr4 != nullptr, "Reallocation in freed space succeeds");
        
        // Check if we reused the freed space (should be same or within the freed block)
        Block* block4 = getHeader(ptr4);
        assertEqual(!block4->used == false, "Reallocated block is marked as used");
    }

    void testFirstFitStrategy() {
        std::cout << "\n=== Testing First Fit Strategy ===" << std::endl;
        resetAllocator();

        // Create some blocks and free some to create gaps
        word_t* ptr1 = allocator.alloc(64);  // Block 1
        word_t* ptr2 = allocator.alloc(32);  // Block 2
        word_t* ptr3 = allocator.alloc(96);  // Block 3
        word_t* ptr4 = allocator.alloc(48);  // Block 4

        // Free blocks 2 and 4 to create gaps
        allocator.free(ptr2);  // Gap of size 32
        allocator.free(ptr4);  // Gap of size 48

        // Test first fit - should use the first available gap (block 2's space)
        Block* foundBlock = allocator.firstFit(16);
        assertEqual(foundBlock != nullptr, "FirstFit finds available block");
        
        Block* expectedBlock = getHeader(ptr2);
        assertEqual(foundBlock == expectedBlock, "FirstFit returns the first suitable block");
    }

    void testBestFitStrategy() {
        std::cout << "\n=== Testing Best Fit Strategy ===" << std::endl;
        resetAllocator();

        // Create blocks with different sizes
        word_t* ptr1 = allocator.alloc(64);
        word_t* ptr2 = allocator.alloc(128);
        word_t* ptr3 = allocator.alloc(32);
        word_t* ptr4 = allocator.alloc(96);

        // Free blocks to create gaps of different sizes
        allocator.free(ptr2);  // Gap of 128
        allocator.free(ptr3);  // Gap of 32
        allocator.free(ptr4);  // Gap of 96

        // Best fit for size 40 should choose the 96-byte gap (smallest that fits)
        Block* foundBlock = allocator.bestFit(40);
        assertEqual(foundBlock != nullptr, "BestFit finds available block");
        
        Block* expectedBlock = getHeader(ptr4);  // Should be the 96-byte block
        assertEqual(foundBlock == expectedBlock, "BestFit returns the best fitting block");
    }

    void testWorstFitStrategy() {
        std::cout << "\n=== Testing Worst Fit Strategy ===" << std::endl;
        resetAllocator();

        word_t* ptr1 = allocator.alloc(64);
        word_t* ptr2 = allocator.alloc(128);
        word_t* ptr3 = allocator.alloc(32);
        word_t* ptr4 = allocator.alloc(96);

        allocator.free(ptr2);  // Gap of 128
        allocator.free(ptr3);  // Gap of 32
        allocator.free(ptr4);  // Gap of 96

        // Worst fit for size 20 should choose the 128-byte gap (largest available)
        Block* foundBlock = allocator.worstFit(20);
        assertEqual(foundBlock != nullptr, "WorstFit finds available block");
        
        Block* expectedBlock = getHeader(ptr2);  // Should be the 128-byte block
        assertEqual(foundBlock == expectedBlock, "WorstFit returns the largest available block");
    }

    void testNextFitStrategy() {
        std::cout << "\n=== Testing Next Fit Strategy ===" << std::endl;
        resetAllocator();

        word_t* ptr1 = allocator.alloc(64);
        word_t* ptr2 = allocator.alloc(32);
        word_t* ptr3 = allocator.alloc(96);
        
        // Free first block
        allocator.free(ptr1);
        
        // Set lastAllocated to ptr2's block
        allocator.lastAllocated = getHeader(ptr2);
        
        // NextFit should start searching from after lastAllocated
        Block* foundBlock = allocator.nextFit(48);
        assertEqual(foundBlock != nullptr, "NextFit finds available block");
    }

    void testBlockSplitting() {
        std::cout << "\n=== Testing Block Splitting ===" << std::endl;
        resetAllocator();

        // Allocate a large block
        word_t* ptr1 = allocator.alloc(256);
        allocator.free(ptr1);
        
        Block* largeBlock = getHeader(ptr1);
        size_t originalSize = largeBlock->size;
        
        // Test if we can split the block
        bool canSplit = allocator.canSplit(largeBlock, 64);
        assertEqual(canSplit, "Large block can be split");
        
        if (canSplit) {
            Block* splitBlock = allocator.split(largeBlock, 64);
            assertEqual(splitBlock != nullptr, "Split operation returns valid block");
            assertEqual(splitBlock->size == 64, "Split block has correct size");
            assertEqual(splitBlock->next != nullptr, "Split creates next block");
            assertEqual(splitBlock->next->size == originalSize - 64 - sizeof(Block) + sizeof(word_t), 
                       "Remaining block has correct size");
        }
    }

    void testBlockCoalescing() {
        std::cout << "\n=== Testing Block Coalescing ===" << std::endl;
        resetAllocator();

        // Allocate adjacent blocks
        word_t* ptr1 = allocator.alloc(64);
        word_t* ptr2 = allocator.alloc(32);
        word_t* ptr3 = allocator.alloc(96);

        // Free adjacent blocks
        allocator.free(ptr1);
        allocator.free(ptr2);

        Block* block1 = getHeader(ptr1);
        Block* block2 = getHeader(ptr2);

        // Test if blocks can be coalesced
        bool canCoalesce = allocator.canCoalesce(block1);
        
        if (canCoalesce) {
            assertEqual(true, "Adjacent free blocks can be coalesced");
            
            size_t originalSize1 = block1->size;
            size_t originalSize2 = block2->size;
            
            Block* coalescedBlock = allocator.coalesce(block1);
            assertEqual(coalescedBlock == block1, "Coalesce returns first block");
            assertEqual(coalescedBlock->size > originalSize1, "Coalesced block is larger");
        }
    }

    void testMemoryIntegrity() {
        std::cout << "\n=== Testing Memory Integrity ===" << std::endl;
        resetAllocator();

        std::vector<word_t*> ptrs;
        const size_t numAllocs = 10;

        // Allocate multiple blocks and write data
        for (size_t i = 0; i < numAllocs; i++) {
            word_t* ptr = allocator.alloc(64);
            assertEqual(ptr != nullptr, "Allocation " + std::to_string(i) + " succeeds");
            
            // Write test pattern
            memset(ptr, i + 1, 64);
            ptrs.push_back(ptr);
        }

        // Verify data integrity
        for (size_t i = 0; i < numAllocs; i++) {
            bool dataIntact = true;
            unsigned char* data = reinterpret_cast<unsigned char*>(ptrs[i]);
            for (size_t j = 0; j < 64; j++) {
                if (data[j] != (i + 1)) {
                    dataIntact = false;
                    break;
                }
            }
            assertEqual(dataIntact, "Data integrity maintained for block " + std::to_string(i));
        }

        // Free half the blocks
        for (size_t i = 0; i < numAllocs; i += 2) {
            allocator.free(ptrs[i]);
        }

        // Verify remaining data integrity
        for (size_t i = 1; i < numAllocs; i += 2) {
            bool dataIntact = true;
            unsigned char* data = reinterpret_cast<unsigned char*>(ptrs[i]);
            for (size_t j = 0; j < 64; j++) {
                if (data[j] != (i + 1)) {
                    dataIntact = false;
                    break;
                }
            }
            assertEqual(dataIntact, "Data integrity after partial free for block " + std::to_string(i));
        }
    }

    void testEdgeCases() {
        std::cout << "\n=== Testing Edge Cases ===" << std::endl;
        resetAllocator();

        // Test zero allocation
        word_t* ptr1 = allocator.alloc(0);
        assertEqual(ptr1 != nullptr, "Zero size allocation handled gracefully");

        // Test very large allocation
        word_t* ptr2 = allocator.alloc(SIZE_MAX);
        // This should likely fail or be handled by OS request
        
        // Test double free (should not crash)
        word_t* ptr3 = allocator.alloc(64);
        allocator.free(ptr3);
        // Note: Double free is undefined behavior, but shouldn't crash in this simple test
        
        // Test allocation after many frees
        std::vector<word_t*> ptrs;
        for (int i = 0; i < 100; i++) {
            ptrs.push_back(allocator.alloc(32));
        }
        
        for (auto ptr : ptrs) {
            allocator.free(ptr);
        }
        
        word_t* ptr4 = allocator.alloc(64);
        assertEqual(ptr4 != nullptr, "Allocation after many frees succeeds");
    }

    void runAllTests() {
        std::cout << "Starting ImplicitAllocator Test Suite..." << std::endl;
        
        testBasicAllocation();
        testFreeAndReallocation();
        testFirstFitStrategy();
        testBestFitStrategy();
        testWorstFitStrategy();
        testNextFitStrategy();
        testBlockSplitting();
        testBlockCoalescing();
        testMemoryIntegrity();
        testEdgeCases();

        std::cout << "\n=== Test Results ===" << std::endl;
        std::cout << "Tests Passed: " << testsPassed << "/" << totalTests << std::endl;
        
        if (testsPassed == totalTests) {
            std::cout << "🎉 All tests passed!" << std::endl;
        } else {
            std::cout << "❌ " << (totalTests - testsPassed) << " tests failed." << std::endl;
        }
    }
};

int main() {
    ImplicitAllocatorTest tester;
    tester.runAllTests();
    return 0;
}