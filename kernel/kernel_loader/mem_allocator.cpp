#include "mem_allocator.hpp"

//Hard-coded heap memory location at 0xC00000-0xD00000
linkedlist_allocator allocator;

void init_allocator(){
    allocator.init((std::size_t*)0xC00000, 0x100000);
}

void* kmalloc(std::size_t n){
    return allocator.malloc(n);
}
void kfree(void* p){
    allocator.free((std::size_t*)p);
}

void* operator new[](std::size_t n) noexcept{
    return kmalloc(n);
}
void operator delete[](void* p) noexcept{
    kfree(p);
}

void* operator new(std::size_t n) noexcept{
    return kmalloc(n);
}
void operator delete(void* p) noexcept{
    kfree(p);
}