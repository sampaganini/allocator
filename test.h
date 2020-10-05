//
// Created by root on 8/27/20.
//

#ifndef ALLOCATOR_TEST_H
#define ALLOCATOR_TEST_H
#include "custom_unistd.h"

void test_malloc();
void test_too_large();
void test_calloc();
void test_free();
void test_debug();
void test_realloc();
void is_valid();
void setup_check();
void too_big_wanted();


void test_calloc_aligned();
void test_malloc_aligned();

#endif //ALLOCATOR_TEST_H
