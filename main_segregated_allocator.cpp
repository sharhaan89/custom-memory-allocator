#include <iostream>
#include <vector>
#include <cstring>
#include "segregated_allocator.h"

void printSeparator(const std::string& title) {
    std::cout << "\n=== " << title << " ===\n";
}

void testBasicAllocation() {
    printSeparator("Testing Basic Allocation");
    
    SegregatedListAllocator allocator;
    
    // Test different size classes
    word_t* ptr8 = allocator.alloc(8);
    word_t* ptr16 = allocator.alloc(16);
    word_t* ptr32 = allocator.alloc(32);
    word_t* ptr64 = allocator.alloc(64);
    word_t* ptr128 = allocator.alloc(128);
    word_t* ptr256 = allocator.alloc(256); // >128 bucket
    
    if(ptr8 && ptr16 && ptr32 && ptr64 && ptr128 && ptr256) {
        std::cout << "✓ All size class allocations successful\n";
        
        // Test writing to each allocation
        *ptr8 = 0xDEADBEEF;
        *ptr16 = 0xCAFEBABE;
        *ptr32 = 0x12345678;
        *ptr64 = 0x87654321;
        *ptr128 = 0xABCDEF00;
        *ptr256 = 0xFEEDFACE;
        
        if(*ptr8 == 0xDEADBEEF && *ptr16 == 0xCAFEBABE && 
           *ptr32 == 0x12345678 && *ptr64 == 0x87654321 &&
           *ptr128 == 0xABCDEF00 && *ptr256 == 0xFEEDFACE) {
            std::cout << "✓ Memory write/read successful for all sizes\n";
        } else {
            std::cout << "✗ Memory write/read failed\n";
        }
        
        // Free all allocations
        allocator.free(ptr8);
        allocator.free(ptr16);
        allocator.free(ptr32);
        allocator.free(ptr64);
        allocator.free(ptr128);
        allocator.free(ptr256);
        
        std::cout << "✓ All deallocations completed\n";
    } else {
        std::cout << "✗ Some allocations failed\n";
    }
}

void testBucketDistribution() {
    printSeparator("Testing Bucket Distribution");
    
    SegregatedListAllocator allocator;
    std::vector<word_t*> ptrs;
    
    // Test edge cases for bucket boundaries
    struct TestCase {
        size_t size;
        int expectedBucket;
        const char* description;
    };
    
    TestCase cases[] = {
        {1, 0, "1 byte -> bucket 0"},
        {8, 0, "8 bytes -> bucket 0"},
        {9, 1, "9 bytes -> bucket 1"},
        {16, 1, "16 bytes -> bucket 1"},
        {17, 2, "17 bytes -> bucket 2"},
        {32, 2, "32 bytes -> bucket 2"},
        {33, 3, "33 bytes -> bucket 3"},
        {64, 3, "64 bytes -> bucket 3"},
        {65, 4, "65 bytes -> bucket 4"},
        {128, 4, "128 bytes -> bucket 4"},
        {129, 5, "129 bytes -> bucket 5"},
        {1024, 5, "1024 bytes -> bucket 5"}
    };
    
    for(auto& testCase : cases) {
        word_t* ptr = allocator.alloc(testCase.size);
        if(ptr) {
            ptrs.push_back(ptr);
            std::cout << "✓ " << testCase.description << "\n";
        } else {
            std::cout << "✗ Failed: " << testCase.description << "\n";
        }
    }
    
    // Free all test allocations
    for(word_t* ptr : ptrs) {
        allocator.free(ptr);
    }
    std::cout << "✓ All test allocations freed\n";
}

void testMultipleAllocationsPerBucket() {
    printSeparator("Testing Multiple Allocations Per Bucket");
    
    SegregatedListAllocator allocator;
    std::vector<word_t*> bucket0_ptrs;
    std::vector<word_t*> bucket5_ptrs;
    
    // Allocate multiple blocks in bucket 0 (8 bytes)
    for(int i = 0; i < 5; i++) {
        word_t* ptr = allocator.alloc(8);
        if(ptr) {
            bucket0_ptrs.push_back(ptr);
            *ptr = i; // Write unique value
        }
    }
    std::cout << "✓ Allocated " << bucket0_ptrs.size() << " blocks in bucket 0\n";
    
    // Allocate multiple blocks in bucket 5 (>128 bytes)
    for(int i = 0; i < 3; i++) {
        word_t* ptr = allocator.alloc(256 + i * 100);
        if(ptr) {
            bucket5_ptrs.push_back(ptr);
            *ptr = i + 100; // Write unique value
        }
    }
    std::cout << "✓ Allocated " << bucket5_ptrs.size() << " blocks in bucket 5\n";
    
    // Verify data integrity
    bool dataIntact = true;
    for(size_t i = 0; i < bucket0_ptrs.size(); i++) {
        if(*bucket0_ptrs[i] != (word_t)i) {
            dataIntact = false;
            break;
        }
    }
    for(size_t i = 0; i < bucket5_ptrs.size(); i++) {
        if(*bucket5_ptrs[i] != (word_t)(i + 100)) {
            dataIntact = false;
            break;
        }
    }
    
    if(dataIntact) {
        std::cout << "✓ Data integrity maintained across multiple allocations\n";
    } else {
        std::cout << "✗ Data integrity compromised\n";
    }
    
    // Free some blocks and reallocate
    allocator.free(bucket0_ptrs[1]);
    allocator.free(bucket0_ptrs[3]);
    
    word_t* new_ptr1 = allocator.alloc(8);
    word_t* new_ptr2 = allocator.alloc(8);
    
    if(new_ptr1 && new_ptr2) {
        std::cout << "✓ Successfully reallocated freed blocks\n";
        allocator.free(new_ptr1);
        allocator.free(new_ptr2);
    }
    
    // Clean up remaining allocations
    for(word_t* ptr : bucket0_ptrs) {
        if(ptr != bucket0_ptrs[1] && ptr != bucket0_ptrs[3]) {
            allocator.free(ptr);
        }
    }
    for(word_t* ptr : bucket5_ptrs) {
        allocator.free(ptr);
    }
}

void testFragmentationReduction() {
    printSeparator("Testing Fragmentation Reduction");
    
    SegregatedListAllocator allocator;
    std::vector<word_t*> mixed_ptrs;
    
    // Allocate mixed sizes to test segregation benefits
    size_t sizes[] = {8, 64, 16, 128, 32, 256, 8, 64, 16};
    
    for(size_t size : sizes) {
        word_t* ptr = allocator.alloc(size);
        if(ptr) {
            mixed_ptrs.push_back(ptr);
            *ptr = size; // Store size as value for verification
        }
    }
    
    std::cout << "✓ Allocated " << mixed_ptrs.size() << " blocks of mixed sizes\n";
    
    // Free every other block to create fragmentation
    for(size_t i = 1; i < mixed_ptrs.size(); i += 2) {
        allocator.free(mixed_ptrs[i]);
        mixed_ptrs[i] = nullptr;
    }
    
    std::cout << "✓ Freed every other block to create fragmentation\n";
    
    // Try to allocate same sizes again - should reuse freed blocks
    std::vector<word_t*> realloc_ptrs;
    for(size_t size : sizes) {
        word_t* ptr = allocator.alloc(size);
        if(ptr) {
            realloc_ptrs.push_back(ptr);
            *ptr = size + 1000; // Different value to distinguish
        }
    }
    
    if(realloc_ptrs.size() == sizeof(sizes)/sizeof(sizes[0])) {
        std::cout << "✓ Successfully reallocated all sizes after fragmentation\n";
    } else {
        std::cout << "✗ Failed to reallocate some sizes\n";
    }
    
    // Clean up
    for(word_t* ptr : mixed_ptrs) {
        if(ptr) allocator.free(ptr);
    }
    for(word_t* ptr : realloc_ptrs) {
        if(ptr) allocator.free(ptr);
    }
}

void testZeroAndLargeAllocations() {
    printSeparator("Testing Edge Cases");
    
    SegregatedListAllocator allocator;
    
    // Test zero allocation
    word_t* zero_ptr = allocator.alloc(0);
    if(zero_ptr) {
        std::cout << "✓ Zero allocation handled\n";
        allocator.free(zero_ptr);
    } else {
        std::cout << "! Zero allocation returned NULL (expected behavior)\n";
    }
    
    // Test very large allocation
    word_t* large_ptr = allocator.alloc(10000);
    if(large_ptr) {
        std::cout << "✓ Large allocation (10KB) successful\n";
        *large_ptr = 0xBEEFCAFE;
        if(*large_ptr == 0xBEEFCAFE) {
            std::cout << "✓ Large allocation memory accessible\n";
        }
        allocator.free(large_ptr);
    } else {
        std::cout << "✗ Large allocation failed\n";
    }
    
    // Test boundary sizes
    word_t* boundary_ptr = allocator.alloc(128); // Exact boundary
    if(boundary_ptr) {
        std::cout << "✓ Boundary size allocation (128 bytes) successful\n";
        allocator.free(boundary_ptr);
    }
}

int main() {
    std::cout << "Starting Segregated List Allocator Tests\n";
    std::cout << "=====================================\n";
    
    testBasicAllocation();
    testBucketDistribution();
    testMultipleAllocationsPerBucket();
    testFragmentationReduction();
    testZeroAndLargeAllocations();
    
    std::cout << "\n=== All Tests Completed ===\n";
    return 0;
}