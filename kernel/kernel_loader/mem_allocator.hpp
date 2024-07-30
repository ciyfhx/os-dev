#ifndef INCLUDE_ME_ALLOCATOR_H
#define INCLUDE_ME_ALLOCATOR_H

#include "linkedlist_allocator.hpp"

extern linkedlist_allocator allocator;

void init_allocator();

void* kmalloc(std::size_t n);
void kfree(void* p);

void* operator new[](std::size_t n) noexcept;
void operator delete[](void* p) noexcept;

void* operator new(std::size_t n) noexcept;
void operator delete(void* p) noexcept;

#endif /* INCLUDE_ME_ALLOCATOR_H */
    