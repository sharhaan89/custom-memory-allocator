# 🧠 Custom Memory Allocator in C++

A modular, class-based custom memory allocator written in C++, supporting multiple allocation strategies including:

🔹 Bump Allocator  
🔹 Implicit Free List (First Fit, Best Fit, etc.)  
🔹 Explicit Free List  
🔹 Segregated Free List Allocator

This project mimics the internals of `malloc`, `free`, and `realloc`, with a clear focus on modular design, allocator architecture, performance, and extensibility.

---

## 🚀 Features

- ✅ Clean and extensible object-oriented architecture
- ✅ Multiple allocator strategies with shared utilities
- ✅ Custom `Block` header with metadata and alignment
- ✅ Block splitting and (forward) coalescing support
- ✅ Flexible fit policies: first-fit, best-fit, worst-fit, next-fit
- ✅ Segregated free list buckets for performance optimization
- ✅ Safe handling of edge cases and memory boundaries

---

## 🗂️ Allocator Modules

### 1. **Bump Allocator**
- Very fast, linear allocation
- No `free`, no reuse of memory
- Useful for arena-style memory models

### 2. **Implicit Free List**
- Single linked list of all blocks
- Headers used to track `size` and `used` status
- Linear search with configurable fit strategy

### 3. **Explicit Free List**
- Free blocks managed via a doubly linked free list
- Better performance and less fragmentation
- Modular class (`ExplicitFreeList`) supports reuse in other allocators

### 4. **Segregated Free List**
- Multiple size-classed buckets, each with its own explicit free list
- Each bucket is an instance of `ExplicitFreeList`
- Offers faster allocation and better fit locality

---

## 🧱 Architecture

```
/src
├── block_utils.*                  # Block header/footer structure, alignment utils
├── bump_allocator.*               # Simple linear allocator
├── implicit_allocator.*           # Implicit free list allocator
├── explicit_allocator.*           # Explicit free list allocator (class-based)
├── segregated_allocator.*         # Segregated free list using multiple explicit allocators
├── main_implicit_allocator.cpp    # Test for the implicit allocator
├── main_explicit_allocator.cpp    # Test for the explicit allocator
└── main_segregated_allocator.cpp  # Test for the segregated allocator
```

- The allocator design follows **low-level memory layout semantics**.
- All blocks contain metadata like:
  - `size`: payload size
  - `used`: allocation flag
  - `next`, `prev`: pointers (in explicit/segregated allocators)
- **Splitting** and **forward coalescing** are implemented to optimize block reuse.
- A shared `Block` structure is used across all strategies to simplify interoperability.

---

# Custom Memory Allocator

## 🧪 Testing & Validation

### Test Suites

The project includes comprehensive test suites for each allocator implementation:

#### **Explicit Allocator Tests** (`main_explicit_allocator.cpp`)
- **Basic Allocation**: Single and multiple allocations with memory write/read validation
- **Coalescing**: Tests adjacent free block merging to reduce fragmentation
- **Fit Strategies**: Validates first-fit, best-fit, and worst-fit algorithms
- **Block Splitting**: Ensures large blocks are properly split when partially allocated
- **Next Fit**: Tests next-fit strategy with fragmented memory patterns
- **Edge Cases**: Zero allocation, alignment verification, and boundary conditions
- **Performance**: Stress testing with 100+ allocations and random deallocation patterns

#### **Implicit Allocator Tests** (`main_implicit_allocator.cpp`)
- **Memory Integrity**: Data persistence across allocation/deallocation cycles
- **Free List Management**: Proper marking of used/unused blocks
- **Coalescing Logic**: Adjacent block merging validation
- **Fit Strategy Comparison**: Side-by-side testing of different placement algorithms
- **Block Splitting**: Large block subdivision with size verification
- **Edge Cases**: Zero-size allocation, double-free protection, large allocation handling

#### **Segregated List Tests** (`main_segregated_allocator.cpp`)
- **Bucket Distribution**: Validates size-class routing (8, 16, 32, 64, 128, >128 bytes)
- **Multiple Allocations**: Tests multiple blocks per size class
- **Fragmentation Reduction**: Measures segregation effectiveness
- **Mixed Workloads**: Random allocation patterns across different size classes
- **Boundary Testing**: Edge cases at bucket boundaries

### Running Tests

```bash
# Compile and run implicit allocator tests  
g++ -I include -Wall -Wextra -g -o test_implicit main_implicit_allocator.cpp src/implicit_allocator.cpp src/block_utils.cpp
./test_implicit

# Compile and run explicit allocator tests
g++ -I include -Wall -Wextra -g -o test_explicit main_explicit_allocator.cpp src/explicit_allocator.cpp src/block_utils.cpp
./test_explicit

# Compile and run segregated allocator tests
g++ -I include -Wall -Wextra -g -o test_seg main_segregated_allocator.cpp src/segregated_allocator.cpp src/block_utils.cpp src/explicit_allocator.cpp
./test_seg
```

### Test Output Examples

**Successful Test Runs:**

**Implicit Allocator**
```
# ./test_implicit 
Starting ImplicitAllocator Test Suite...

=== Testing Basic Allocation ===
✓ Basic allocation returns non-null pointer PASSED
✓ HeapStart is set after first allocation PASSED
✓ Top is set after first allocation PASSED
✓ LastAllocated is set after allocation PASSED
✓ Second allocation returns non-null pointer PASSED
✓ Third allocation returns non-null pointer PASSED
✓ All pointers are unique PASSED

=== Testing Free and Reallocation ===
✓ Freed block is marked as unused PASSED
✓ Reallocation in freed space succeeds PASSED
✓ Reallocated block is marked as used PASSED

=== Testing First Fit Strategy ===
✓ FirstFit finds available block PASSED
✓ FirstFit returns the first suitable block PASSED

=== Testing Best Fit Strategy ===
✓ BestFit finds available block PASSED
✓ BestFit returns the best fitting block PASSED

=== Testing Worst Fit Strategy ===
✓ WorstFit finds available block PASSED
✓ WorstFit returns the largest available block PASSED

=== Testing Next Fit Strategy ===
✓ NextFit finds available block PASSED

=== Testing Block Splitting ===
✓ Large block can be split PASSED
✓ Split operation returns valid block PASSED
✓ Split block has correct size PASSED
✓ Split creates next block PASSED
✓ Remaining block has correct size PASSED

=== Testing Block Coalescing ===
✓ Adjacent free blocks can be coalesced PASSED
✓ Coalesce returns first block PASSED
✓ Coalesced block is larger PASSED

=== Testing Memory Integrity ===
✓ Allocation 0 succeeds PASSED
✓ Allocation 1 succeeds PASSED
✓ Allocation 2 succeeds PASSED
✓ Allocation 3 succeeds PASSED
✓ Allocation 4 succeeds PASSED
✓ Allocation 5 succeeds PASSED
✓ Allocation 6 succeeds PASSED
✓ Allocation 7 succeeds PASSED
✓ Allocation 8 succeeds PASSED
✓ Allocation 9 succeeds PASSED
✓ Data integrity maintained for block 0 PASSED
✓ Data integrity maintained for block 1 PASSED
✓ Data integrity maintained for block 2 PASSED
✓ Data integrity maintained for block 3 PASSED
✓ Data integrity maintained for block 4 PASSED
✓ Data integrity maintained for block 5 PASSED
✓ Data integrity maintained for block 6 PASSED
✓ Data integrity maintained for block 7 PASSED
✓ Data integrity maintained for block 8 PASSED
✓ Data integrity maintained for block 9 PASSED
✓ Data integrity after partial free for block 1 PASSED
✓ Data integrity after partial free for block 3 PASSED
✓ Data integrity after partial free for block 5 PASSED
✓ Data integrity after partial free for block 7 PASSED
✓ Data integrity after partial free for block 9 PASSED

=== Testing Edge Cases ===
✓ Zero size allocation handled gracefully PASSED
✓ Allocation after many frees succeeds PASSED

=== Test Results ===
Tests Passed: 52/52
All tests passed!
```

**Explicit Allocator**
```
# ./test_explicit
Starting Explicit Allocator Tests
===================================

=== Testing Basic Allocation ===
✓ Basic allocation successful
✓ Memory write/read successful
✓ Multiple allocations successful

--- Heap State ---
Free List: NULL
✓ Deallocation successful

--- Heap State ---
Free List: [0x5568522e8060: size=128] -> NULL

=== Testing Coalescing ===
Allocated 4 blocks of 100 bytes each

--- Heap State ---
Free List: NULL
Freed middle two blocks

--- Heap State ---
Free List: [0x5568522e8330: size=104] -> [0x5568522e82a8: size=104] -> NULL
Freed last block - should coalesce with previous free blocks

--- Heap State ---
Free List: [0x5568522e83b8: size=104] -> [0x5568522e8330: size=104] -> [0x5568522e82a8: size=104] -> NULL
✓ Successfully allocated large block after coalescing

--- Heap State ---
Free List: [0x5568522e83b8: size=104] -> [0x5568522e8440: size=256] -> NULL

=== Testing Fit Strategies ===
Created fragmented heap with free blocks of sizes 70, 110, 150

--- Heap State ---
Free List: [0x5568522e87d8: size=152] -> [0x5568522e86a0: size=112] -> [0x5568522e85b8: size=72] -> NULL
First fit for 80 bytes: found block of size 152
Best fit for 80 bytes: found block of size 112
Worst fit for 80 bytes: found block of size 152

=== Testing Block Splitting ===
Created large free block of 1000 bytes

--- Heap State ---
Free List: [0x5568522e8890: size=1000] -> NULL
Allocated 200 bytes - should split the large block

--- Heap State ---
Free List: [0x5568522e8978: size=768] -> NULL
✓ Successfully allocated from remaining split block

--- Heap State ---
Free List: [0x5568522e8ac8: size=432] -> NULL

=== Testing Edge Cases ===
Zero allocation: succeeded
✓ Tiny allocation (1 byte) successful
✓ Large allocation (10KB) successful
✓ All allocations properly aligned

--- Heap State ---
Free List: NULL

=== Testing Next Fit Strategy ===
Created fragmented memory with alternating free blocks

--- Heap State ---
Free List: [0x5568522eba50: size=64] -> [0x5568522eb990: size=64] -> [0x5568522eb8d0: size=64] -> [0x5568522eb810: size=64] -> NULL
Allocated 3 blocks using next fit
Pointers: 0x5568522eba70, 0x5568522eb9b0, 0x5568522eb8f0

--- Heap State ---
Free List: [0x5568522eb810: size=64] -> NULL

=== Performance Test ===
Allocated 100 blocks
Freed half the blocks
Successfully allocated 100 additional blocks

===================================
All tests completed!
```

**Segregated Allocator**
```
# ./test_seg
Starting Segregated List Allocator Tests
=====================================

=== Testing Basic Allocation ===
✓ All size class allocations successful
✓ Memory write/read successful for all sizes
✓ All deallocations completed

=== Testing Bucket Distribution ===
✓ 1 byte -> bucket 0
✓ 8 bytes -> bucket 0
✓ 9 bytes -> bucket 1
✓ 16 bytes -> bucket 1
✓ 17 bytes -> bucket 2
✓ 32 bytes -> bucket 2
✓ 33 bytes -> bucket 3
✓ 64 bytes -> bucket 3
✓ 65 bytes -> bucket 4
✓ 128 bytes -> bucket 4
✓ 129 bytes -> bucket 5
✓ 1024 bytes -> bucket 5
✓ All test allocations freed

=== Testing Multiple Allocations Per Bucket ===
✓ Allocated 5 blocks in bucket 0
✓ Allocated 3 blocks in bucket 5
✓ Data integrity maintained across multiple allocations
✓ Successfully reallocated freed blocks

=== Testing Fragmentation Reduction ===
✓ Allocated 9 blocks of mixed sizes
✓ Freed every other block to create fragmentation
✓ Successfully reallocated all sizes after fragmentation

=== Testing Edge Cases ===
✓ Zero allocation handled
✓ Large allocation (10KB) successful
✓ Large allocation memory accessible
✓ Boundary size allocation (128 bytes) successful

=== All Tests Completed ===
```

---

## 📈 Performance Goals

### Allocation Speed Targets
- **Explicit Free List**: O(n) worst-case but optimized for common patterns
- **Implicit Free List**: O(n) traversal with coalescing optimization
- **Segregated Lists**: O(1) average case for size classes ≤128 bytes

### Stress Test Performance
- **100 mixed allocations**: All implementations handle successfully
- **Fragmentation recovery**: Segregated lists show best reuse patterns  
- **Large allocations (>10KB)**: Handled efficiently by all implementations

---

## 🧩 Future Work

### Core Enhancements
- [ ] **`realloc()` support**: Resize allocated blocks in-place when possible
- [ ] **Backward coalescing**: Full bi-directional coalesce for implicit allocator
- [ ] **Thread safety**: Lock-free segregated lists with atomic operations
- [ ] **Memory alignment**: Support for custom alignment requirements (16, 32, 64 byte)

### Debugging & Profiling Tools
- [ ] **Heap consistency checker CLI**: Validate free list integrity
- [ ] **CLI visualizer**: ASCII art memory map dumper for debugging
- [ ] **Memory leak detector**: Track unfreed allocations
- [ ] **Fragmentation analyzer**: Visual fragmentation reporting

---

## 🙋‍♂️ Author

Sharhaan — 3rd Year, Computer Science @ NITK  
Refactored using clean OOP principles for modularity and reuse.
