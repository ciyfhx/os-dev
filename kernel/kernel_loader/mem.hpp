#ifndef INCLUDE_MEM_H
#define INCLUDE_MEM_H

#include <cstdint>

extern "C" void memcpy(void* dest, void* src, uint32_t size);
extern "C" uint32_t memcmp(void* dest, void* src, uint32_t size);

#endif /* INCLUDE_MEM_H */
    