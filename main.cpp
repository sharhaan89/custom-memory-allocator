#include <iostream>
#include <cassert>
#include "block_utils.h"
#include "bump_allocator.h"
#include "implicit_allocator.h"

using namespace std;
using implicit_allocator::alloc;
using implicit_allocator::free;
using implicit_allocator::findBlock;
using implicit_allocator::bestFit;
using implicit_allocator::nextFit;

void test_bump_allocator() {
    auto p1 = bump_allocator::alloc(3);
    auto p1b = getHeader(p1);
    assert(p1b->size == align(3));

    auto p2 = bump_allocator::alloc(8);
    auto p2b = getHeader(p2);
    assert(p2b->size == align(8));

    cout << "BUMP ALLOCATOR: All test cases passed!\n";
}

void test_first_fit() {
    word_t* p1 = alloc(100);
    word_t* p2 = alloc(200);

    free(p1);

    word_t* p3 = alloc(80);

    assert(getHeader(p3) == getHeader(p1)); // ✅ reused freed block
}

void test_best_fit() {
    word_t* a = alloc(120);
    word_t* b = alloc(240);
    word_t* c = alloc(160);

    free(a);
    free(b);
    free(c);

    Block* expected = getHeader(a);
    Block* actual   = findBlock(100, bestFit);

    assert(actual == expected); // ✅ best fit should choose 120-byte block
}

void test_next_fit_wraparound() {
    word_t* a = alloc(100);
    word_t* b = alloc(200);
    word_t* c = alloc(150);

    free(a);
    free(b);
    free(c);

    // simulate last allocation as 'c'
    Block* expected = getHeader(a);
    Block* actual   = findBlock(100, nextFit);

    assert(actual == expected); // ✅ wrapped to beginning
}

int main() {
    //test_first_fit();
    test_next_fit_wraparound();
    //test_best_fit();
    cout << "all passed";

    return 0;
}
